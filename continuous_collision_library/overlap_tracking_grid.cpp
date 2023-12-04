//#include <algorithm>
//#include "vector_2d_math_utils/vector_types.h"
//#include "vector_2d_math_utils/byte_vector_2d.h"
#include "pch.h"
#include "overlap_tracking_grid.h"
#include <algorithm>

ContinuousCollisionLibrary::overlap_tracking_grid::overlap_tracking_grid()
{
	//call the setup code
	initialize();
}

void ContinuousCollisionLibrary::overlap_tracking_grid::initialize()
{
	//set all the overlap flags to 0
	overlaps.clear_data();

	// set the bounds of all tiles to be inside out / impossible to collide with 
	std::transform(bounds.data.tile_data.begin(), bounds.data.tile_data.end(), bounds.data.tile_data.begin(), [](const auto x)
		{
			return tile_local_bounds::inverse_max_size_rect(); 
		});

	// initialize the per sector overlap trackers 
	std::for_each(overlap_pairs.begin(), overlap_pairs.end(), [](auto sector_overlap_pairs) {sector_overlap_pairs.reset();});

}

ContinuousCollisionLibrary::overlap_flags ContinuousCollisionLibrary::overlap_tracking_grid::calculate_flag_for_tile(
	const math_2d_util::uivec2d & tile_to_create_flag_for, 
	const math_2d_util::uivec2d & target_tile) const
{
	//get top left corner for overlap range
	math_2d_util::uivec2d top_left_corner = target_tile - ContinuousCollisionLibrary::overlap_flags::center();

	//offset from top left corner
	math_2d_util::uivec2d offset = tile_to_create_flag_for - top_left_corner;

	//convert from uint to int vectors
	return ContinuousCollisionLibrary::overlap_flags(offset);
}

void ContinuousCollisionLibrary::overlap_tracking_grid::add_flag_to_tiles(
	math_2d_util::uivec2d& source_tile_cord, 
	const math_2d_util::uirect& add_to_area, 
	const math_2d_util::uirect& old_bounds, 
	const math_2d_util::uirect& new_bounds)
{
	
	//sanity check to make sure boudns falls within grid 
	assert(is_rect_in_grid(add_to_area), "rect exits map bounds");


	//the per tile overlap list is a lot more likely to trigger a cache miss so I am 
	//delaying it and doing it in a sepparate pass to hopefully not trash the cache as much
	std::array<SectorGrid::sector_tile_index<grid_dimensions>, max_overlap_pairs> new_overlaps;
	uint32 num_of_new_overlaps = 0;

	//alocate an array for all the possible overlaps 
	//I am doing this calclulation in passes in the hopes that by doing it this way the 
	//cpu / compiler will be better at parelelizing it 
	std::array<uint32, overlap_flags::max_overlaps> overlap_indexes_in_tile;
	uint32 num_of_overlaps_in_tile = 0;

	//loop through all the tiles 
	for (uint32 iy = add_to_area.min.y; iy < add_to_area.max.y; ++iy)
	{
		for (uint32 ix = add_to_area.min.x; ix < add_to_area.max.x; ++ix)
		{
			//reset the number of overlaps found at this tile
			num_of_overlaps_in_tile = 0;

			math_2d_util::uivec2d target_xy{ ix,iy };

			//convert from xy to lookup
			SectorGrid::sector_tile_index<grid_dimensions> target_index = grid_helper.from_xy(target_xy);

			//get the flag data for the sector we are accessing 
			overlap_flags& overlaps_in_tile = overlaps.get_ref_to_data(target_index);

			//calculate the corner offset for this tile 
			//this calculates a point 4 tiles up/left from the target tile
			//the target tile has a uint64 offset flags where each bit represents a value that can represent 64 points in a 8 by 8 tile window
			//the center tile is considered at 4,4 biased to the bottom right
			math_2d_util::uivec2d overlap_corner = target_xy - overlap_flags::center();


			//loop through all tight tile bounds that overlap the target tile
			//and pull out the overlap indexes 
			for (uint32 overlap_flag_index : overlaps_in_tile)
			{
				overlap_indexes_in_tile[num_of_overlaps_in_tile++] = overlap_flag_index;
			}

			
			//loop over all items in the index
			std::for_each(overlap_indexes_in_tile.begin(), overlap_indexes_in_tile.begin() + num_of_overlaps_in_tile, [&](uint32 flag_index)
				{
					//calculate the offset for the flag 
					//this value is 0-7 in both x and y axis
					math_2d_util::uivec2d offset = overlap_flags::calcualte_offset_for_flag_index(flag_index);

					//calculate the coordinate of this overlap flag
					math_2d_util::uivec2d overlap_tile_coordinate = offset + overlap_corner;

					//offset per axis
					constexpr uint32 offset_per_axis = tile_local_bounds::vector_type::center_extent;

					//calculate the top left corner of the overlap region for that tile
					math_2d_util::uivec2d overlap_tile_overlap_region_top_left = overlap_tile_coordinate - math_2d_util::uivec2d{ offset_per_axis,offset_per_axis };

					//convert from coordinate to sector grid index 
					auto overlap_index = grid_helper.from_xy(overlap_tile_coordinate);

					//get the bounds of the overlapped tile 
					auto bounds_of_overlapped_tile = bounds.get_ref_to_data(overlap_index);

					//convert bounds from local space to world space using the top left corner offset
					auto world_bounds_of_overlapped_tile = math_2d_util::rect_2d_math::get_offset_rect_as<math_2d_util::uirect>(bounds_of_overlapped_tile, overlap_tile_overlap_region_top_left);

					//check if these tiles were already overlapping
					bool was_overlapping = math_2d_util::rect_2d_math::is_overlapping(world_bounds_of_overlapped_tile, old_bounds);

					//get the min tile overlap between the new bounds 
					auto min_overlap_address = math_2d_util::rect_2d_math::get_top_left_corner_of_overlap(new_bounds, world_bounds_of_overlapped_tile);

					//check if this tile is the first tile that these two rects overlap
					bool is_first_overlap_tile = target_xy == min_overlap_address;
					
					//if this is the first time these two tiles have overlapped
					{
						//add it to list of all overlap pairs for this tile
						new_overlaps[num_of_new_overlaps] = overlap_index;
						
						num_of_new_overlaps += is_first_overlap_tile && !was_overlapping;
					}
				});
			
			//flag for source tile 
			{
				//get the offset 
				math_2d_util::uivec2d offset_in_target_window = source_tile_cord - overlap_corner;

				auto flag = overlap_flags(offset_in_target_window);

				//add overlap flag for tile
				overlaps_in_tile |= flag;
			}
		}
	}

	//the source tile index
	auto source_world_tile = grid_helper.from_xy(source_tile_cord);

	auto source_sector = source_world_tile.components.sector_index;

	auto source_sub_tile = source_world_tile.components.sector_sub_tile_index;

	auto& source_overlap_sector = overlap_pairs[source_sector];

	//the offset to get to the top left corner of the source tile pair window
	auto window_offset = math_2d_util::byte_vector_2d::center_as<math_2d_util::uivec2d>();;
	 	
	//the top left corner for the window that all overlaping tiles must fall inside
	const auto source_overlap_pair_top_left_corner = source_tile_cord - window_offset;
	
	//loop through all the new overlapping tiles and add this tile to the pairs list for those tiles
	for (uint32 i = 0; i < num_of_new_overlaps; ++i)
	{
		auto overlap_index = new_overlaps[i];
	
		//get the sector list
		uint32 target_overlap_sector = overlap_index.components.sector_index;
		uint32 target_overlap_sub_tile = overlap_index.components.sector_sub_tile_index;
	
		//get the overlap tracking data structure for the sector
		auto& target_sector_overlap_list = overlap_pairs[target_overlap_sector];
	
		//compute the difference from this tile to the other tile
		auto other_world_tile = grid_helper.to_xy(overlap_index);
	
		//get the overlap window for the target tile 
		const auto target_overlap_pair_top_left_corner = other_world_tile - window_offset;
	
		//compute things 
		auto dif_from_source_window = other_world_tile - source_overlap_pair_top_left_corner;
	
		auto dif_from_target_window = source_tile_cord - target_overlap_pair_top_left_corner;
	
		//convert from dif to byte vec to compress size to reduce cache misses
		auto dif_source_byte_vec = math_2d_util::byte_vector_2d::from_vector(dif_from_source_window);
		auto dif_target_byte_vec = math_2d_util::byte_vector_2d::from_vector(dif_from_target_window);

		//add this tile to the target tile 
		target_sector_overlap_list.add(target_overlap_sub_tile, dif_target_byte_vec);
		source_overlap_sector.add(source_sub_tile, dif_source_byte_vec);
	}
}

void ContinuousCollisionLibrary::overlap_tracking_grid::remove_flag_from_tiles(math_2d_util::uivec2d& source_tile_cord, const math_2d_util::uirect& remove_area, const math_2d_util::uirect& old_bounds, const math_2d_util::uirect& new_bounds)
{
	//sanity check to make sure boudns falls within grid 
	assert(is_rect_in_grid(remove_area), "rect exits map bounds");


	//the per tile overlap list is a lot more likely to trigger a cache miss so I am 
	//delaying it and doing it in a sepparate pass to hopefully not trash the cache as much
	std::array<SectorGrid::sector_tile_index<grid_dimensions>, max_overlap_pairs> new_overlaps;
	uint32 num_of_new_overlaps = 0;

	//alocate an array for all the possible overlaps 
	//I am doing this calclulation in passes in the hopes that by doing it this way the 
	//cpu / compiler will be better at parelelizing it 
	std::array<uint32, overlap_flags::max_overlaps> overlap_indexes_in_tile;
	uint32 num_of_overlaps_in_tile = 0;

	//loop through all the tiles 
	for (uint32 iy = remove_area.min.y; iy < remove_area.max.y; ++iy)
	{
		for (uint32 ix = remove_area.min.x; ix < remove_area.max.x; ++ix)
		{
			//reset the number of overlaps found at this tile
			num_of_overlaps_in_tile = 0;

			math_2d_util::uivec2d target_xy{ ix,iy };

			//convert from xy to lookup
			SectorGrid::sector_tile_index<grid_dimensions> target_index = grid_helper.from_xy(target_xy);

			//get the flag data for the sector we are accessing 
			overlap_flags& overlaps_in_tile = overlaps.get_ref_to_data(target_index);

			//calculate the corner offset for this tile 
			//this calculates a point 4 tiles up/left from the target tile
			//the target tile has a uint64 offset flags where each bit represents a value that can represent 64 points in a 8 by 8 tile window
			//the center tile is considered at 4,4 biased to the bottom right
			math_2d_util::uivec2d overlap_corner = target_xy - overlap_flags::center();

			//remove the flag from the tile. we do this first so we dont self detect overlaps
			{
				//get the offset 
				math_2d_util::uivec2d offset_in_target_window = source_tile_cord - overlap_corner;

				auto flag = overlap_flags(offset_in_target_window);

				//add overlap flag for tile
				overlaps_in_tile &= ~flag;
			}



			//loop through all tight tile bounds that overlap the target tile
			//and pull out the overlap indexes 
			for (uint32 overlap_flag_index : overlaps_in_tile)
			{
				overlap_indexes_in_tile[num_of_overlaps_in_tile++] = overlap_flag_index;
			}


			//loop over all items in the index
			std::for_each(overlap_indexes_in_tile.begin(), overlap_indexes_in_tile.begin() + num_of_overlaps_in_tile, [&](uint32 flag_index)
				{
					//calculate the offset for the flag 
					//this value is 0-7 in both x and y axis
					math_2d_util::uivec2d offset = overlap_flags::calcualte_offset_for_flag_index(flag_index);

					//calculate the coordinate of this overlap flag
					math_2d_util::uivec2d overlap_tile_coordinate = offset + overlap_corner;

					//offset per axis
					constexpr uint32 offset_per_axis = tile_local_bounds::vector_type::center_extent;

					//calculate the top left corner of the overlap region for that tile
					math_2d_util::uivec2d overlap_tile_overlap_region_top_left = overlap_tile_coordinate - math_2d_util::uivec2d{ offset_per_axis,offset_per_axis };

					//convert from coordinate to sector grid index 
					auto overlap_index = grid_helper.from_xy(overlap_tile_coordinate);

					//get the bounds of the overlapped tile 
					auto bounds_of_overlapped_tile = bounds.get_ref_to_data(overlap_index);

					//convert bounds from local space to world space using the top left corner offset
					auto world_bounds_of_overlapped_tile = math_2d_util::rect_2d_math::get_offset_rect_as<math_2d_util::uirect>(bounds_of_overlapped_tile, overlap_tile_overlap_region_top_left);

					//check if these tiles will be overlappling after the bounds have changed 
					bool will_continue_to_overlap = math_2d_util::rect_2d_math::is_overlapping(world_bounds_of_overlapped_tile, new_bounds);

					//get the min tile overlap between the new bounds 
					auto min_overlap_address = math_2d_util::rect_2d_math::get_top_left_corner_of_overlap(old_bounds, world_bounds_of_overlapped_tile);

					//check if this tile is the first tile that these two rects overlap
					bool is_first_overlap_tile = target_xy == min_overlap_address;

					//if this is the first time these two tiles have overlapped
					{
						//add it to list of all overlap pairs for this tile
						new_overlaps[num_of_new_overlaps] = overlap_index;

						num_of_new_overlaps += is_first_overlap_tile && !will_continue_to_overlap;
					}
				});

			
		}
	}

	//the source tile index
	auto source_world_tile = grid_helper.from_xy(source_tile_cord);

	auto source_sector = source_world_tile.components.sector_index;

	auto source_sub_tile = source_world_tile.components.sector_sub_tile_index;

	auto& source_overlap_sector = overlap_pairs[source_sector];

	//the offset to get to the top left corner of the source tile pair window
	auto window_offset = math_2d_util::byte_vector_2d::center_as<math_2d_util::uivec2d>();;

	//the top left corner for the window that all overlaping tiles must fall inside
	const auto source_overlap_pair_top_left_corner = source_tile_cord - window_offset;

	//loop through all the new overlapping tiles and add this tile to the pairs list for those tiles
	for (uint32 i = 0; i < num_of_new_overlaps; ++i)
	{
		auto overlap_index = new_overlaps[i];

		//get the sector list
		uint32 target_overlap_sector = overlap_index.components.sector_index;
		uint32 target_overlap_sub_tile = overlap_index.components.sector_sub_tile_index;

		//get the overlap tracking data structure for the sector
		auto& target_sector_overlap_list = overlap_pairs[target_overlap_sector];

		//compute the difference from this tile to the other tile
		auto other_world_tile = grid_helper.to_xy(overlap_index);

		//get the overlap window for the target tile 
		const auto target_overlap_pair_top_left_corner = other_world_tile - window_offset;

		//compute things 
		auto dif_from_source_window = other_world_tile - source_overlap_pair_top_left_corner;

		auto dif_from_target_window = source_tile_cord - target_overlap_pair_top_left_corner;

		//convert from dif to byte vec to compress size to reduce cache misses
		auto dif_source_byte_vec = math_2d_util::byte_vector_2d::from_vector(dif_from_source_window);
		auto dif_target_byte_vec = math_2d_util::byte_vector_2d::from_vector(dif_from_target_window);

		//remove this tile from the overlap list 
		target_sector_overlap_list.remove(target_overlap_sub_tile, dif_target_byte_vec);
		source_overlap_sector.remove(source_sub_tile, dif_source_byte_vec);
	}
}

bool ContinuousCollisionLibrary::overlap_tracking_grid::is_point_in_grid(const math_2d_util::uivec2d& point)
{
	bool x_valid = point.x < grid_dimensions::tile_w;
	bool y_valid = point.y < grid_dimensions::tile_w;
	return x_valid & y_valid;
}

bool ContinuousCollisionLibrary::overlap_tracking_grid::is_rect_in_grid(const math_2d_util::uirect& rect)
{
	return is_point_in_grid(rect.min) & is_point_in_grid(rect.max);
}

ContinuousCollisionLibrary::uint32 ContinuousCollisionLibrary::overlap_tracking_grid::get_affinity_for_offset(const math_2d_util::uivec2d& offset) const
{
	
	return ContinuousCollisionLibrary::uint32();
}
