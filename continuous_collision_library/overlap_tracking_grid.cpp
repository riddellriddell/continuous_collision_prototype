//#include <algorithm>
//#include "vector_2d_math_utils/vector_types.h"
//#include "vector_2d_math_utils/byte_vector_2d.h"
#include "pch.h"
#include "overlap_tracking_grid.h"
#include <algorithm>
#include <type_traits>

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

void ContinuousCollisionLibrary::overlap_tracking_grid::update_bounds(const math_2d_util::ivec2d& tile_coordinate_to_update, overlap_grid_index tile_sector_packed_index_to_update, const math_2d_util::irect& new_world_bounds_for_tile_items)
{
	//check that the index and coordinate match
	assert(grid_helper.to_xy<math_2d_util::ivec2d>(tile_sector_packed_index_to_update) == tile_coordinate_to_update, "Passed in tile xy corrdinate did not match tile index");

	auto signed_tile_coordinate_to_update = static_cast<math_2d_util::ivec2d>(tile_coordinate_to_update);

	//get existing bounds 
	auto& old_local_bounds = bounds.get_ref_to_data(tile_sector_packed_index_to_update);

	//get the top left corner the tile is projected from
	auto bounds_window_top_left = signed_tile_coordinate_to_update - tile_local_bounds::vector_type::center().convert_to<math_2d_util::ivec2d>();

	//convert to world bounds
	auto old_world_bounds = math_2d_util::rect_2d_math::get_offset_rect_as<math_2d_util::irect>(old_local_bounds, bounds_window_top_left);

	//array to hold the remove pass
	std::array<math_2d_util::irect, 4> remove_rects;
	uint32 remove_count = 0;
	
	//array to hold the add pass
	std::array<math_2d_util::irect, 4> add_rects;
	uint32 add_count = 0;

	auto new_min_y = std::max(new_world_bounds_for_tile_items.min.y, old_world_bounds.min.y);
	auto new_max_y = std::min(new_world_bounds_for_tile_items.max.y, old_world_bounds.max.y);

	//catch if this is the first time the tile is getting something added and does not need a multi pass add  or remove pass
	{
		add_rects[add_count] = new_world_bounds_for_tile_items;
		add_count = old_local_bounds == tile_local_bounds::inverse_max_size_rect();
	}

	//catch if this tile is being completely cleared 
	//no need to do a mulit part add or remove remove pass
	{
		remove_rects[remove_count] = old_world_bounds;
		remove_count = new_world_bounds_for_tile_items == math_2d_util::irect::inverse_max_size_rect();
	}

	//check we are not both adding nothing from nothing
	assert((add_count != remove_count) || (add_count ) != 1);
	
	if(!(add_count | remove_count))
	{

		//------------- remove top and bottom before sides as removes are faster with long x and short y axis -----------------

		{
			//trim the top
			{
				remove_rects[remove_count].min = old_world_bounds.min;
				remove_rects[remove_count].max = math_2d_util::ivec2d{ old_world_bounds.max.x, new_world_bounds_for_tile_items.min.y };
				remove_count += old_world_bounds.min.y < new_world_bounds_for_tile_items.min.y;
			}

			//trim the left
			{
				remove_rects[remove_count].min = math_2d_util::ivec2d{ old_world_bounds.min.x,new_min_y };
				remove_rects[remove_count].max = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.min.x,new_max_y };
				remove_count += old_world_bounds.min.x < new_world_bounds_for_tile_items.min.x;
			}

			//trim the right
			{
				remove_rects[remove_count].min = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.max.x, new_min_y };
				remove_rects[remove_count].max = math_2d_util::ivec2d{ old_world_bounds.max.x, new_max_y };
				remove_count += old_world_bounds.max.x > new_world_bounds_for_tile_items.max.x;
			}

			//trim the bottom 
			{
				remove_rects[remove_count].max = old_world_bounds.max;
				remove_rects[remove_count].min = math_2d_util::ivec2d{ old_world_bounds.min.x, new_world_bounds_for_tile_items.max.y };
				remove_count += old_world_bounds.max.y > new_world_bounds_for_tile_items.max.y;
			}
		}



		//------------- add top and bottom before sides as adds are faster with long x and short y axis -----------------

		{
			//add to the top
			{
				add_rects[add_count].min = new_world_bounds_for_tile_items.min;
				add_rects[add_count].max = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.max.x, old_local_bounds.min.y };
				add_count += old_world_bounds.min.y > new_world_bounds_for_tile_items.min.y;
			}

			//add to the left
			{
				add_rects[add_count].min = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.min.x,new_min_y };
				add_rects[add_count].max = math_2d_util::ivec2d{ old_local_bounds.min.x,new_max_y };
				add_count += old_world_bounds.min.x > new_world_bounds_for_tile_items.min.x;
			}

			//add to the right
			{
				add_rects[add_count].min = math_2d_util::ivec2d{ old_local_bounds.max.x, new_min_y };
				add_rects[add_count].max = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.max.x, new_max_y };
				add_count += old_world_bounds.max.x < new_world_bounds_for_tile_items.max.x;
			}

			//add to the bottom 
			{
				add_rects[add_count].max = new_world_bounds_for_tile_items.max;
				add_rects[add_count].min = math_2d_util::ivec2d{ new_world_bounds_for_tile_items.min.x,old_local_bounds.max.y };
				add_count += old_world_bounds.max.y < new_world_bounds_for_tile_items.max.y;
			}
		}
	}

	//loop through all the remove rects and change the flags 
	for (uint32 i = 0; i < remove_count; i++)
	{
		remove_flag_from_tiles(tile_coordinate_to_update, tile_sector_packed_index_to_update, remove_rects[i], old_world_bounds, new_world_bounds_for_tile_items);
	}

	//loop through all the add rects and change the flags 
	for (uint32 i = 0; i < add_count; i++)
	{
		add_flag_to_tiles(tile_coordinate_to_update, tile_sector_packed_index_to_update, (add_rects[i]), old_world_bounds, new_world_bounds_for_tile_items);
	}

	//convert new world bounds back to local bounds
	auto new_local_min = new_world_bounds_for_tile_items.min - bounds_window_top_left;
	auto new_local_max = new_world_bounds_for_tile_items.max - bounds_window_top_left;

	//apply the update to the bounds of the rect 
	old_local_bounds.min = static_cast<tile_local_bounds::vector_type>(new_local_min);
	old_local_bounds.max = static_cast<tile_local_bounds::vector_type>(new_local_max);

}

ContinuousCollisionLibrary::overlap_flags ContinuousCollisionLibrary::overlap_tracking_grid::calculate_flag_for_tile(
	const math_2d_util::ivec2d & tile_to_create_flag_for, 
	const math_2d_util::ivec2d & target_tile) const
{
	//get top left corner for overlap range
	auto top_left_corner = target_tile - ContinuousCollisionLibrary::overlap_flags::center<std::remove_reference<decltype(target_tile)>::type>();
	
	//offset from top left corner
	auto offset = tile_to_create_flag_for - top_left_corner;
	
	//convert from uint to int vectors
	return ContinuousCollisionLibrary::overlap_flags(offset);
}

void ContinuousCollisionLibrary::overlap_tracking_grid::add_flag_to_tiles(
	const math_2d_util::ivec2d& source_tile_cord,
	const math_2d_util::irect& add_to_area,
	const math_2d_util::irect& old_bounds,
	const math_2d_util::irect& new_bounds)
{
	//the source tile index
	auto source_world_tile = grid_helper.from_xy(source_tile_cord);

	add_flag_to_tiles(source_tile_cord, source_world_tile, add_to_area, old_bounds, new_bounds);
}

void ContinuousCollisionLibrary::overlap_tracking_grid::add_flag_to_tiles(
	const math_2d_util::ivec2d& source_tile_cord,
	overlap_grid_index source_world_tile,
	const math_2d_util::irect& add_to_area,
	const math_2d_util::irect& old_bounds,
	const math_2d_util::irect& new_bounds)
{
	//sanity check to make sure boudns falls within grid 
	assert(is_rect_in_grid(add_to_area), "rect exits map bounds");

	assert(grid_helper.to_xy<math_2d_util::ivec2d>(source_world_tile) == source_tile_cord, "source cord and source index must be for the same tile");

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
	for (int32 iy = add_to_area.min.y; iy < add_to_area.max.y; ++iy)
	{
		for (int32 ix = add_to_area.min.x; ix < add_to_area.max.x; ++ix)
		{
			//reset the number of overlaps found at this tile
			num_of_overlaps_in_tile = 0;

			math_2d_util::ivec2d target_xy(ix,iy);

			//convert from xy to lookup
			SectorGrid::sector_tile_index<grid_dimensions> target_index = grid_helper.from_xy(target_xy);

			//get the flag data for the sector we are accessing 
			overlap_flags& overlaps_in_tile = overlaps.get_ref_to_data(target_index);

			//calculate the corner offset for this tile 
			//this calculates a point 4 tiles up/left from the target tile
			//the target tile has a uint64 offset flags where each bit represents a value that can represent 64 points in a 8 by 8 tile window
			//the center tile is considered at 4,4 biased to the bottom right
			math_2d_util::ivec2d overlap_corner = target_xy - overlap_flags::center<math_2d_util::ivec2d>();


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
					math_2d_util::ivec2d offset = overlap_flags::calcualte_offset_for_flag_index(flag_index);

					//calculate the coordinate of this overlap flag
					math_2d_util::ivec2d overlap_tile_coordinate = offset + overlap_corner;

					//calculate the top left corner of the overlap region for that tile
					math_2d_util::ivec2d overlap_tile_overlap_region_top_left = overlap_tile_coordinate - static_cast<math_2d_util::ivec2d>(tile_local_bounds::vector_type::center());

					//convert from coordinate to sector grid index 
					auto overlap_index = grid_helper.from_xy(overlap_tile_coordinate);

					//get the bounds of the overlapped tile 
					const auto bounds_of_overlapped_tile = bounds.get_ref_to_data(overlap_index);

					//convert bounds from local space to world space using the top left corner offset
					auto world_bounds_of_overlapped_tile = math_2d_util::rect_2d_math::get_offset_rect_as<math_2d_util::irect>(bounds_of_overlapped_tile, overlap_tile_overlap_region_top_left);

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
				math_2d_util::ivec2d offset_in_target_window = source_tile_cord - overlap_corner;

				auto flag = overlap_flags(offset_in_target_window);

				//add overlap flag for tile
				overlaps_in_tile |= flag;
			}
		}
	}

	auto source_sector = grid_helper.to_sector_index(source_world_tile);

	auto source_sub_tile = grid_helper.to_sub_sector_index(source_world_tile);

	auto& source_overlap_sector = overlap_pairs[source_sector];

	//the offset to get to the top left corner of the source tile pair window
	auto window_offset = math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();;
	 	
	//the top left corner for the window that all overlaping tiles must fall inside
	const auto source_overlap_pair_top_left_corner = source_tile_cord - window_offset;
	
	//loop through all the new overlapping tiles and add this tile to the pairs list for those tiles
	for (uint32 i = 0; i < num_of_new_overlaps; ++i)
	{
		auto overlap_index = new_overlaps[i];
	
		//get the sector list
		uint32 target_overlap_sector = grid_helper.to_sector_index(overlap_index);
		uint32 target_overlap_sub_tile = grid_helper.to_sub_sector_index(overlap_index);
	
		//get the overlap tracking data structure for the sector
		auto& target_sector_overlap_list = overlap_pairs[target_overlap_sector];
	
		//compute the difference from this tile to the other tile
		auto other_world_tile = grid_helper.to_xy<math_2d_util::ivec2d>(overlap_index);
	
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


void ContinuousCollisionLibrary::overlap_tracking_grid::remove_flag_from_tiles(const math_2d_util::ivec2d& source_tile_cord, const math_2d_util::irect& remove_area, const math_2d_util::irect& old_bounds, const math_2d_util::irect& new_bounds)
{
	//the source tile index
	auto source_world_tile = grid_helper.from_xy(source_tile_cord);

	remove_flag_from_tiles(source_tile_cord, source_world_tile, remove_area, old_bounds, new_bounds);
}

void ContinuousCollisionLibrary::overlap_tracking_grid::remove_flag_from_tiles(const math_2d_util::ivec2d& source_tile_cord, overlap_grid_index source_world_tile,  const math_2d_util::irect & remove_area, const math_2d_util::irect & old_bounds, const math_2d_util::irect & new_bounds)
{
	//sanity check to make sure boudns falls within grid 
	assert(is_rect_in_grid(remove_area), "rect exits map bounds");
	assert(grid_helper.to_xy<math_2d_util::ivec2d>(source_world_tile) ==  static_cast<math_2d_util::ivec2d>(source_tile_cord), "source cord and source index must be for the same tile");

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
	for (int32 iy = remove_area.min.y; iy < remove_area.max.y; ++iy)
	{
		for (int32 ix = remove_area.min.x; ix < remove_area.max.x; ++ix)
		{
			//reset the number of overlaps found at this tile
			num_of_overlaps_in_tile = 0;

			math_2d_util::ivec2d target_xy(ix,iy);

			//convert from xy to lookup
			SectorGrid::sector_tile_index<grid_dimensions> target_index = grid_helper.from_xy(target_xy);

			//get the flag data for the sector we are accessing 
			overlap_flags& overlaps_in_tile = overlaps.get_ref_to_data(target_index);

			//calculate the corner offset for this tile 
			//this calculates a point 4 tiles up/left from the target tile
			//the target tile has a uint64 offset flags where each bit represents a value that can represent 64 points in a 8 by 8 tile window
			//the center tile is considered at 4,4 biased to the bottom right
			math_2d_util::ivec2d overlap_corner = target_xy - overlap_flags::center<math_2d_util::ivec2d>();

			//remove the flag from the tile. we do this first so we dont self detect overlaps
			{
				//get the offset 
				math_2d_util::ivec2d offset_in_target_window = source_tile_cord - overlap_corner;

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
					math_2d_util::ivec2d offset = overlap_flags::calcualte_offset_for_flag_index(flag_index);

					//calculate the coordinate of this overlap flag
					math_2d_util::ivec2d overlap_tile_coordinate = offset + overlap_corner;

					//calculate the top left corner of the overlap region for that tile
					math_2d_util::ivec2d overlap_tile_overlap_region_top_left = overlap_tile_coordinate - static_cast<math_2d_util::ivec2d>(tile_local_bounds::vector_type::center());

					//convert from coordinate to sector grid index 
					auto overlap_index = grid_helper.from_xy(overlap_tile_coordinate);

					//get the bounds of the overlapped tile 
					const auto bounds_of_overlapped_tile = bounds.get_ref_to_data(overlap_index);

					//convert bounds from local space to world space using the top left corner offset
					auto world_bounds_of_overlapped_tile = math_2d_util::rect_2d_math::get_offset_rect_as<math_2d_util::irect>(bounds_of_overlapped_tile, overlap_tile_overlap_region_top_left);

					//check if these tiles will be overlappling after the bounds have changed 
					bool will_continue_to_overlap = math_2d_util::rect_2d_math::is_overlapping(world_bounds_of_overlapped_tile, static_cast<math_2d_util::irect>(new_bounds));

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

	auto source_sector = grid_helper.to_sector_index(source_world_tile);

	auto source_sub_tile = grid_helper.to_sub_sector_index(source_world_tile);

	auto& source_overlap_sector = overlap_pairs[source_sector];

	//the offset to get to the top left corner of the source tile pair window
	auto window_offset = math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();;

	//the top left corner for the window that all overlaping tiles must fall inside
	const auto source_overlap_pair_top_left_corner = source_tile_cord - window_offset;

	//loop through all the new overlapping tiles and add this tile to the pairs list for those tiles
	for (uint32 i = 0; i < num_of_new_overlaps; ++i)
	{
		auto overlap_index = new_overlaps[i];

		//get the sector list
		uint32 target_overlap_sector = grid_helper.to_sector_index(overlap_index);
		uint32 target_overlap_sub_tile = grid_helper.to_sub_sector_index(overlap_index);

		//get the overlap tracking data structure for the sector
		auto& target_sector_overlap_list = overlap_pairs[target_overlap_sector];

		//compute the difference from this tile to the other tile
		auto other_world_tile = grid_helper.to_xy<math_2d_util::ivec2d>(overlap_index);

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

bool ContinuousCollisionLibrary::overlap_tracking_grid::is_point_in_grid(const math_2d_util::ivec2d& point)
{
	bool x_valid = (point.x < grid_dimensions::tile_w) && point.x >= 0;
	bool y_valid = (point.y < grid_dimensions::tile_w) && point.y >= 0;
	return x_valid & y_valid;
}

bool ContinuousCollisionLibrary::overlap_tracking_grid::is_rect_in_grid(const math_2d_util::uirect& rect)
{
	return is_point_in_grid(rect.min) & is_point_in_grid(rect.max);
}

bool ContinuousCollisionLibrary::overlap_tracking_grid::is_rect_in_grid(const math_2d_util::irect& rect)
{
	return is_point_in_grid(rect.min) & is_point_in_grid(rect.max);
}

ContinuousCollisionLibrary::uint32 ContinuousCollisionLibrary::overlap_tracking_grid::get_affinity_for_offset(const math_2d_util::uivec2d& offset) const
{
	
	return ContinuousCollisionLibrary::uint32();
}
