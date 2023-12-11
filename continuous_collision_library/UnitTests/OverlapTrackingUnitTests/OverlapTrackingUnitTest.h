#pragma once
#include <limits>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <ranges>

#include "../../overlap_tracking_grid.h"
#include "vector_2d_math_utils/vector_types.h"
#include "vector_2d_math_utils/byte_vector_2d.h"

namespace ContinuousCollisionLibrary
{
	static class overlap_tracking_unit_test
	{
	public:
		static void run_test()
		{
			//create an overlap tracking grid 
			auto overlap_grid = new ContinuousCollisionLibrary::overlap_tracking_grid();
			
			//test grid helper 
			{
				math_2d_util::uivec2d target_tile(0, 0);

				auto sector_index = overlap_grid->grid_helper.from_xy(target_tile);

				auto decoded_tile = overlap_grid->grid_helper.to_xy(sector_index);

				assert(decoded_tile == target_tile);
			}

			//test grid helper 
			{
				math_2d_util::uivec2d target_tile(1, 0);

				auto sector_index = overlap_grid->grid_helper.from_xy(target_tile);

				auto decoded_tile = overlap_grid->grid_helper.to_xy(sector_index);

				assert(decoded_tile == target_tile);
			}

			//test grid helper 
			{
				math_2d_util::uivec2d target_tile(15, 0);

				auto sector_index = overlap_grid->grid_helper.from_xy(target_tile);

				auto decoded_tile = overlap_grid->grid_helper.to_xy(sector_index);

				assert(decoded_tile == target_tile);
			}

			//test grid helper 
			{
				math_2d_util::uivec2d target_tile(16, 0);

				auto sector_index = overlap_grid->grid_helper.from_xy(target_tile);

				auto decoded_tile = overlap_grid->grid_helper.to_xy(sector_index);

				assert(decoded_tile == target_tile);
			}

			//test grid helper 
			{
				math_2d_util::uivec2d target_tile(0, 1);

				auto sector_index = overlap_grid->grid_helper.from_xy(target_tile);

				auto decoded_tile = overlap_grid->grid_helper.to_xy(sector_index);

				assert(decoded_tile == target_tile);
			}


			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile(1, 1);

				//the overlap region to add flags to
				math_2d_util::irect area_to_add_flags_to{ 0u, 0u, 3u, 3u };

				math_2d_util::irect old_bounds = math_2d_util::irect::inverse_max_size_rect();

				//add overlap flags to a region
				overlap_grid->add_flag_to_tiles(target_tile, area_to_add_flags_to, old_bounds, area_to_add_flags_to);

				auto new_bounds = ContinuousCollisionLibrary::tile_local_bounds::center_rect();

				new_bounds.min -= ContinuousCollisionLibrary::tile_local_bounds::vector_type{ 1u,1u };
				new_bounds.max += ContinuousCollisionLibrary::tile_local_bounds::vector_type{ 2u,2u };

				//update the bounds
				overlap_grid->bounds.set_data(overlap_grid->grid_helper.from_xy(target_tile), new_bounds);

				//check if flags were added
				for (int32 iy = area_to_add_flags_to.min.y; iy < area_to_add_flags_to.max.y; ++iy)
				{
					for (int32 ix = area_to_add_flags_to.min.x; ix < area_to_add_flags_to.max.x; ++ix)
					{
						//get the tile to check
						math_2d_util::ivec2d tile_to_check{ ix,iy };

						//get the flag for that tile
						auto flag_for_source_tile = overlap_grid->calculate_flag_for_tile(target_tile, tile_to_check);

						//get the index of the flag data in the lookup array 
						auto index = overlap_grid->grid_helper.from_xy(tile_to_check);

						//check if that flag is set in the tracking grid 
						auto& flag_data = overlap_grid->overlaps.get_ref_to_data(index);

						//check if flag is set 
						assert(flag_data.has_flags(flag_for_source_tile));
					}
				}

			}

			

			//add flags for another area
			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile(2, 1);

				//the overlap region to add flags to
				math_2d_util::irect area_to_add_flags_to{ 1u, 0u, 4u, 3u };

				math_2d_util::irect old_bounds = math_2d_util::irect::inverse_max_size_rect();

				//add overlap flags to a region
				overlap_grid->add_flag_to_tiles(target_tile, area_to_add_flags_to, old_bounds, area_to_add_flags_to);

				auto new_bounds = ContinuousCollisionLibrary::tile_local_bounds::center_rect();

				new_bounds.min -= ContinuousCollisionLibrary::tile_local_bounds::vector_type{ 1u,1u };
				new_bounds.max += ContinuousCollisionLibrary::tile_local_bounds::vector_type{ 2u,2u };

				//update the bounds
				overlap_grid->bounds.set_data(overlap_grid->grid_helper.from_xy(target_tile), new_bounds);

				//check if flags were added
				for (int32 iy = area_to_add_flags_to.min.y; iy < area_to_add_flags_to.max.y; ++iy)
				{
					for (int32 ix = area_to_add_flags_to.min.x; ix < area_to_add_flags_to.max.x; ++ix)
					{
						//get the tile to check
						math_2d_util::ivec2d tile_to_check{ ix,iy };

						//get the flag for that tile
						auto flag_for_source_tile = overlap_grid->calculate_flag_for_tile(target_tile, tile_to_check);

						//get the index of the flag data in the lookup array 
						auto index = overlap_grid->grid_helper.from_xy(tile_to_check);

						//check if that flag is set in the tracking grid 
						auto& flag_data = overlap_grid->overlaps.get_ref_to_data(index);

						//check if flag is set 
						assert(flag_data.has_flags(flag_for_source_tile));
					}
				}

			}

			

			//check if both tiles have a overlaps tracked
			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile_1(1, 1);
				math_2d_util::ivec2d target_tile_2(2, 1);

				//convert to lookup 
				auto index_01 = overlap_grid->grid_helper.from_xy<math_2d_util::ivec2d>(target_tile_1);
				auto index_02 = overlap_grid->grid_helper.from_xy<math_2d_util::ivec2d>(target_tile_2);

				auto target_01_overlap_start = overlap_grid->overlap_pairs[index_01.components.sector_index].get_root_node_start(index_01.components.sector_sub_tile_index);
				auto target_01_overlap_end = overlap_grid->overlap_pairs[index_01.components.sector_index].end();
				
				std::for_each(target_01_overlap_start, target_01_overlap_end, [&](auto& target_offset) 
					{
						//coordinate is a byte vec offset, convert to a true value
						math_2d_util::ivec2d overlap_offset = static_cast<math_2d_util::ivec2d>(target_offset) -math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();
						math_2d_util::ivec2d overlap_tile = overlap_offset + target_tile_1;
				
						//check that the tile is what we expect
						assert(overlap_tile == target_tile_2);
					
					});
				
				auto target_02_overlap_start = overlap_grid->overlap_pairs[index_02.components.sector_index].get_root_node_start(index_02.components.sector_sub_tile_index);
				auto target_02_overlap_end = overlap_grid->overlap_pairs[index_02.components.sector_index].end();
				
				std::for_each(target_02_overlap_start, target_02_overlap_end, [&](auto& target_offset)
					{
						//coordinate is a byte vec offset, convert to a true value
						math_2d_util::ivec2d overlap_offset = static_cast<math_2d_util::ivec2d>(target_offset) - math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();
						math_2d_util::ivec2d overlap_tile = overlap_offset + target_tile_2;
				
						//check that the tile is what we expect
						assert(overlap_tile == target_tile_1);
				
					});
			}
		
			
			//remove flags for first bounds 
			{
				//the sector to remove flags for
				math_2d_util::ivec2d target_tile(1, 1);

				//the overlap region to remove flags 
				math_2d_util::irect area_to_remove_flags_from{ 0, 0, 3, 3};

				math_2d_util::irect new_bounds = math_2d_util::irect::inverse_max_size_rect();

				//add overlap flags to a region
				overlap_grid->remove_flag_from_tiles(target_tile, area_to_remove_flags_from, area_to_remove_flags_from, new_bounds);

				//update the bounds
				overlap_grid->bounds.set_data(overlap_grid->grid_helper.from_xy(target_tile), ContinuousCollisionLibrary::tile_local_bounds::inverse_max_size_rect());

				//check if flags were added
				for (int32 iy = area_to_remove_flags_from.min.y; iy < area_to_remove_flags_from.max.y; ++iy)
				{
					for (int32 ix = area_to_remove_flags_from.min.x; ix < area_to_remove_flags_from.max.x; ++ix)
					{
						//get the tile to check
						math_2d_util::ivec2d tile_to_check{ ix,iy };

						//get the flag for that tile
						auto flag_for_source_tile = overlap_grid->calculate_flag_for_tile(target_tile, tile_to_check);

						//get the index of the flag data in the lookup array 
						auto index = overlap_grid->grid_helper.from_xy(tile_to_check);

						//check if that flag is set in the tracking grid 
						auto& flag_data = overlap_grid->overlaps.get_ref_to_data(index);

						//check if flag is set 
						assert(!flag_data.has_flags(flag_for_source_tile));
					}
				}

			}

			
			//remove flags for second bounds 
			{
				//the sector to remove flags for
				math_2d_util::ivec2d target_tile(2, 1);

				//the overlap region to remove flags 
				math_2d_util::irect area_to_remove_flags_from{ 1u, 0u, 4u, 3u };

				math_2d_util::irect new_bounds = math_2d_util::irect::inverse_max_size_rect();

				//add overlap flags to a region
				overlap_grid->remove_flag_from_tiles(target_tile, area_to_remove_flags_from, area_to_remove_flags_from, new_bounds);

				//update the bounds
				overlap_grid->bounds.set_data(overlap_grid->grid_helper.from_xy(target_tile), ContinuousCollisionLibrary::tile_local_bounds::inverse_max_size_rect());

				//check if flags were added
				for (int32 iy = area_to_remove_flags_from.min.y; iy < area_to_remove_flags_from.max.y; ++iy)
				{
					for (int32 ix = area_to_remove_flags_from.min.x; ix < area_to_remove_flags_from.max.x; ++ix)
					{
						//get the tile to check
						math_2d_util::ivec2d tile_to_check{ ix,iy };

						//get the flag for that tile
						auto flag_for_source_tile = overlap_grid->calculate_flag_for_tile(target_tile, tile_to_check);

						//get the index of the flag data in the lookup array 
						auto index = overlap_grid->grid_helper.from_xy(tile_to_check);

						//check if that flag is set in the tracking grid 
						auto& flag_data = overlap_grid->overlaps.get_ref_to_data(index);

						//check if flag is set 
						assert(!flag_data.has_flags(flag_for_source_tile));
					}
				}
			}
		

			//reset the grid
			overlap_grid->initialize();

			
			//add bounds again 
			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile(1, 1);

				//the overlap region to add flags to
				math_2d_util::irect area_to_add_flags_to{ 0u, 0u, 3u, 3u };
				
				//get the index of the flag data in the lookup array 
				auto index = overlap_grid->grid_helper.from_xy(target_tile);

				//update the bounds 
				overlap_grid->update_bounds(target_tile, index, area_to_add_flags_to);
			}

			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile(2, 1);

				//the overlap region to add flags to
				math_2d_util::irect area_to_add_flags_to{ 1u, 0u, 4u, 3u };

				//get the index of the flag data in the lookup array 
				auto index = overlap_grid->grid_helper.from_xy(target_tile);

				//update the bounds 
				overlap_grid->update_bounds(target_tile, index, area_to_add_flags_to);
			}

			//check if both tiles have a overlaps tracked
			{
				//the sector to add flags for
				math_2d_util::ivec2d target_tile_1(1, 1);
				math_2d_util::ivec2d target_tile_2(2, 1);

				//convert to lookup 
				auto index_01 = overlap_grid->grid_helper.from_xy(target_tile_1);
				auto index_02 = overlap_grid->grid_helper.from_xy(target_tile_2);

				auto target_01_overlap_start = overlap_grid->overlap_pairs[index_01.components.sector_index].get_root_node_start(index_01.components.sector_sub_tile_index);
				auto target_01_overlap_end = overlap_grid->overlap_pairs[index_01.components.sector_index].end();

				std::for_each(target_01_overlap_start, target_01_overlap_end, [&](auto& target_offset)
					{
						//coordinate is a byte vec offset, convert to a true value
						math_2d_util::ivec2d overlap_offset = static_cast<math_2d_util::ivec2d>(target_offset) - math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();
						math_2d_util::ivec2d overlap_tile = overlap_offset + target_tile_1;

						//check that the tile is what we expect
						assert(overlap_tile == target_tile_2);

					});

				auto target_02_overlap_start = overlap_grid->overlap_pairs[index_02.components.sector_index].get_root_node_start(index_02.components.sector_sub_tile_index);
				auto target_02_overlap_end = overlap_grid->overlap_pairs[index_02.components.sector_index].end();

				std::for_each(target_02_overlap_start, target_02_overlap_end, [&](auto& target_offset)
					{
						//coordinate is a byte vec offset, convert to a true value
						math_2d_util::ivec2d overlap_offset = static_cast<math_2d_util::ivec2d>(target_offset) - math_2d_util::byte_vector_2d::center_as<math_2d_util::ivec2d>();
						math_2d_util::ivec2d overlap_tile = overlap_offset + target_tile_2;

						//check that the tile is what we expect
						assert(overlap_tile == target_tile_1);

					});
			}

			//remove flags for first bounds 
			{
				//the sector to remove flags for
				math_2d_util::ivec2d target_tile(1, 1);

				//the overlap region to remove flags 
				math_2d_util::irect area_to_remove_flags_from{ 0u, 0u, 3u, 3u };

				math_2d_util::irect new_bounds = math_2d_util::irect::inverse_max_size_rect();

				//add overlap flags to a region
				overlap_grid->remove_flag_from_tiles(target_tile, area_to_remove_flags_from, area_to_remove_flags_from, new_bounds);

				//update the bounds
				overlap_grid->bounds.set_data(overlap_grid->grid_helper.from_xy(target_tile), ContinuousCollisionLibrary::tile_local_bounds::inverse_max_size_rect());

				//check if flags were added
				for (int32 iy = area_to_remove_flags_from.min.y; iy < area_to_remove_flags_from.max.y; ++iy)
				{
					for (int32 ix = area_to_remove_flags_from.min.x; ix < area_to_remove_flags_from.max.x; ++ix)
					{
						//get the tile to check
						math_2d_util::ivec2d tile_to_check{ ix,iy };

						//get the flag for that tile
						auto flag_for_source_tile = overlap_grid->calculate_flag_for_tile(target_tile, tile_to_check);

						//get the index of the flag data in the lookup array 
						auto index = overlap_grid->grid_helper.from_xy(tile_to_check);

						//check if that flag is set in the tracking grid 
						auto& flag_data = overlap_grid->overlaps.get_ref_to_data(index);

						//check if flag is set 
						assert(!flag_data.has_flags(flag_for_source_tile));
					}
				}
			}
		}
	};
}
