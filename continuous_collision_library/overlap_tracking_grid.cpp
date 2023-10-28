#include <algorithm>
#include "vector_types.h"
#include "byte_vector_2d.h"
#include "pch.h"
#include "overlap_tracking_grid.h"


ContinuousCollisionLibrary::overlap_flags ContinuousCollisionLibrary::overlap_tracking_grid::calculate_flag_for_tile(
	const math_2d_util::uivec2d & tile_to_create_flag_for, 
	const math_2d_util::uivec2d & target_tile) const
{
	//get top left corner for overlap range
	math_2d_util::uivec2d top_left_corner = { target_tile.x - tile_overlap_max_width, target_tile.y - tile_overlap_max_width };

	//offset from top left corner
	math_2d_util::uivec2d offset = tile_to_create_flag_for - top_left_corner;

	//convert from uint to int vectors
	return calculate_flag_for_offset(offset);
}

constexpr ContinuousCollisionLibrary::overlap_flags ContinuousCollisionLibrary::overlap_tracking_grid::calculate_flag_for_offset(const math_2d_util::uivec2d& offset) const
{
	//all offsets are from the top left corner of the target tile and are all in positive directions

	//sanity check that the offsets are not larger than the largest possible flag width
	assert(offset.x < tile_overlap_half_rep_max && offset.y < tile_overlap_half_rep_max, "Offset larget than allowed values");

	uint32 flag_offset = offset.x + tile_overlap_half_rep_max * offset.y;

	assert(flag_offset < 64, "the uint 64 represeing all the tiles overlapping this tile can only store 64 values staring at offset 0");

	uint64 flag = 1ul << flag_offset;

	return ContinuousCollisionLibrary::overlap_flags{ flag };
}

math_2d_util::uivec2d ContinuousCollisionLibrary::overlap_tracking_grid::calcualte_offset_for_flag_index(uint32 flag_index) const
{
	//extract the offset 
	uint32 y_offset = flag_index / tile_overlap_half_rep_max;
	uint32 x_offset = flag_index - (y_offset * tile_overlap_half_rep_max);

	return math_2d_util::uivec2d{ x_offset , y_offset };
}


void ContinuousCollisionLibrary::overlap_tracking_grid::add_flag_to_tiles(
	math_2d_util::uivec2d& source_tile_cord, 
	const math_2d_util::uirect& add_to_area, 
	const math_2d_util::uirect& old_bounds, 
	const math_2d_util::uirect& new_bounds)
{
	//sanity check to make sure boudns falls within grid 
	assert(is_rect_in_grid(add_to_area), "rect exits map bounds");

	//maximum estimated number of overlap pairs. at max an overlap only occurs at the max size of this tile plus the 
	//max size of the other tile so 2 times max tile overlap squared 
	static constexpr uint32 max_overlap_pairs = (tile_overlap_max_width + tile_overlap_max_width) * (tile_overlap_max_width + tile_overlap_max_width) ;

	//the per tile overlap list is a lot more likely to trigger a cache miss so I am 
	//delaying it and doing it in a sepparate pass to hopefully not trash the cache as much
	std::array<SectorGrid::sector_tile_index<grid_dimensions>, max_overlap_pairs> new_overlaps;
	uint32 num_of_new_overlaps = 0;

	//loop through all the tiles 
	for (uint32 iy = add_to_area.min.y; iy < add_to_area.max.y; ++iy)
	{
		for (uint32 ix = add_to_area.min.x; ix < add_to_area.max.x; ++ix)
		{
			math_2d_util::uivec2d target_xy{ ix,iy };

			//convert from xy to lookup
			SectorGrid::sector_tile_index<grid_dimensions> target_index = grid_helper.from_xy(target_xy);

			//get the flag data for the sector we are accessing 
			overlap_flags& overlaps_in_tile = overlaps.get_ref_to_data(target_index);

			//calculate the corner offset for this tile 
			//this calculates a point 4 tiles up/left from the target tile
			//the target tile has a uint64 offset flags where each bit represents a value that can represent 64 points in a 8 by 8 tile window
			//the center tile is considered at 4,4 biased to the bottom right
			math_2d_util::uivec2d overlap_corner = target_xy - offset_for_0_to_7_vec;

			//alocate an array for all the possible overlaps 
			//I am doing this calclulation in passes in the hopes that by doing it this way the 
			//cpu / compiler will be better at parelelizing it 
			std::array<uint32, overlap_flags::max_overlaps> overlap_indexes;
			uint32 num_of_overlaps = 0;

			//loop through all tight tile bounds that overlap the target tile
			//and pull out the overlap indexes 
			for (uint32 overlap_flag_index : overlaps_in_tile)
			{
				overlap_indexes[num_of_overlaps++] = overlap_flag_index;
			}

			//loop over all items in the index
			std::for_each(overlap_indexes.begin(), overlap_indexes.begin() + num_of_overlaps, [&](uint32 flag_index)
				{
					//calculate the offset for the flag 
					//this value is 0-7 in both x and y axis
					math_2d_util::uivec2d offset = calcualte_offset_for_flag_index(flag_index);

					//calculate the coordinate of this overlap flag
					math_2d_util::uivec2d overlap_tile_coordinate = offset + overlap_corner;

					//calculate the top left corner of the overlap region for that tile
					math_2d_util::uivec2d overlap_tile_overlap_region_top_left = overlap_tile_coordinate - tile_overlap_offset;

					//convert from coordinate to sector grid index 
					auto overlap_index = grid_helper.from_xy(overlap_tile_coordinate);

					//get the bounds of the overlapped tile 
					auto bounds_of_overlapped_tile = bounds.get_ref_to_data(overlap_index);

					//convert bounds from local space to world space using the top left corner offset
					auto world_bounds_of_overlapped_tile = math_2d_util::rect_2d_math::get_offset_rect_as<
						math_2d_util::uirect,
						decltype(bounds_of_overlapped_tile),
						decltype(overlap_tile_overlap_region_top_left)>(
							bounds_of_overlapped_tile, 
							overlap_tile_overlap_region_top_left);

					//check if these tiles were already overlapping
					bool was_overlapping = math_2d_util::rect_2d_math::is_overlapping(world_bounds_of_overlapped_tile, old_bounds);

					//get the min tile overlap between the new bounds 
					auto min_overlap_address = math_2d_util::rect_2d_math::get_top_left_corner_of_overlap(new_bounds, world_bounds_of_overlapped_tile);

					//check if this tile is the first tile that these two rects overlap
					bool is_first_overlap_tile = target_xy == min_overlap_address;
					
					//if this is the first time these two tiles have overlapped
					if (is_first_overlap_tile && !was_overlapping)
					{
						//add it to list of all overlap pairs for this tile
						new_overlaps[num_of_new_overlaps++] = overlap_index;
						//get the top left corner for all posible overlaps 
					}
				});

			//calculate the offset for the flag 
			auto flag = ContinuousCollisionLibrary::overlap_flags();

			flag = calculate_flag_for_offset(target_xy);

			//add overlap flag for tile
			overlaps_in_tile |= flag;
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
	
		//compute the differenec from this tile to the other tile
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
		source_overlap_sector.add(target_overlap_sub_tile, dif_source_byte_vec);
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