#pragma once
#include <array>
#include <algorithm>

#include "vector_2d_math_utils/rect_types.h"
#include "vector_2d_math_utils/vector_types.h"

#include "base_types_definition.h"

#include "misc_utilities/int_type_selection.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"

#include "continuous_collision_library/overlap_tracking_grid.h"
#include "array_utilities/fixed_free_list.h"
#include "array_utilities/paged_2d_array.h"
#include "array_utilities/handle_tracked_2d_paged_array.h"
#include "array_utilities/HandleSystem/handle_system.h"
#include "array_utilities/fixed_size_vector.h"
#include "array_utilities/paged_wide_node_linked_list.h"

#include "sector_grid_data_structure/sector_grid.h"
#include "sector_grid_data_structure/sector_grid_dimensions.h"
#include "sector_grid_data_structure/sector_grid_index.h"


namespace ContinuousCollisionLibrary
{
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	class phyisics_2d_main
	{
		using phyisics_2d_main_type = phyisics_2d_main<Imax_objects, Iworld_sector_x_count>;
		
		//definition of the target dimensions of the grid system
		using grid_dimension_type = SectorGrid::sector_grid_dimensions<Iworld_sector_x_count, 16>;

		using tile_coordinate_type = SectorGrid::sector_tile_index<grid_dimension_type>;

		//use the sector tile helper to convert to sector
		using sector_grid_helper_type = SectorGrid::sector_grid_helper<grid_dimension_type>;

		//sector grid instance this currently holds nothing but am thinking about making sizing at load not compile 
		sector_grid_helper_type grid_helper;

		static constexpr size_t page_size_for_collider_data = 1024;

		static constexpr size_t max_sectors_internal = grid_dimension_type::sector_grid_count_max;

		using sector_count_type = MiscUtilities::uint_s<max_sectors_internal>::int_type_t;

		static constexpr sector_count_type sector_count = max_sectors_internal;

		static constexpr size_t bits_needed_to_store_handle_count = MiscUtilities::bits_needed_to_represent_number(Imax_objects);

	public:
		//temp handle type definition
		using handle_type = HandleSystem::default_handle_type<Imax_objects>;

	private:

		using handle_data_lookup_system_type = ArrayUtilities::fixed_free_list<Imax_objects, handle_type>;

		//the system to manage all the handles and data lookup addresses 
		handle_data_lookup_system_type handle_manager;

		struct collision_data_ref
		{
			float& x;
			float& y;
			
			float& velocity_x;
			float& velocity_y;
						
			float& radius;


			auto get_as_tuple()
			{
				return std::tie(x, y, velocity_x, velocity_y, radius);
			}

			//auto get_as_tuple()
			//{
			//	return std::tie(x);
			//}
		};

		//static_assert(sector_count == 256);
		//static_assert(Imax_objects == 254);
		//static_assert(page_size_for_collider_data == 1024);

		//data for all the colliders that is tied to / tracked with unique handle id's from the handle manager
		using collision_data_container_type = ArrayUtilities::handle_tracked_2d_paged_array<handle_type, sector_count, Imax_objects, Imax_objects, page_size_for_collider_data, collision_data_ref>;

		//data for all the items in the grid
		collision_data_container_type collision_data_container;

	public:

		//buffer for new data to add to the grid
		struct new_collider_data
		{
			friend class phyisics_2d_main_type;

		private:

			//this value is set on add to the add buffer 
			handle_type owner;
		public:
			math_2d_util::fvec2d position;

			math_2d_util::fvec2d velocity;

			float radius;

			auto get_as_tuple()
			{
				return std::tie(position.x, position.y, velocity.x, velocity.y, radius);
			}
		};

	private:


		//buffer to hold all data being added to the grid 
		using new_collider_header_type = ArrayUtilities::paged_2d_array_header < sector_count, Imax_objects, Imax_objects, page_size_for_collider_data>;

		using new_collider_data_containter_type = std::array<new_collider_data, new_collider_header_type::max_total_entries>;

		new_collider_header_type collider_to_add_header;
		new_collider_data_containter_type colliders_to_add_data;
		
		//which sectors have items queued to be added into the game
		ArrayUtilities::fixed_size_vector_array<sector_count_type, max_sectors_internal> sectors_with_queued_items;

		//sector_count_type number_of_active_sectors;
		//std::array<sector_count_type, max_sectors_internal> active_sectors;

		//grid tracking collisions 
		overlap_tracking_grid overlap_grid;

		//assuming average tile will have at max 6? 
		static constexpr size_t node_width = 8;

		//assuming half of the tiles in each sector are full on average
		static constexpr size_t page_size = (grid_dimension_type::sector_tile_count / 2) ;

		//list of all the agents in each tile in each sector
		using per_tile_collider_list_type = ArrayUtilities::paged_wide_node_linked_list<handle_type, grid_dimension_type::tile_count, node_width, Imax_objects, Imax_objects, Imax_objects, page_size, grid_dimension_type::sector_tile_count>;

		//agent lookup
		per_tile_collider_list_type colliders_in_tile_tracker;

		public:

		//try add a handle to the simulation
		handle_type try_queue_item_to_add( new_collider_data&& data_for_new_collider);

		private:

		//add all the queued items for a sector into the physics system 
		void add_items_from_sector(sector_count_type sector_index);

		//add all the new items 
		void add_items_from_all_sectors();

		//update the grid overlap system
		void update_bounds_in_sector(sector_count_type sector_index);

		//update the bounds in all sectors 
		void update_bounds_in_all_sectors();

		//move all objects in sector 
		void update_positions_in_sector(sector_count_type sector_index);

		//move all objects in the world
		void update_all_positions();

		public:
		//add queued items 
		void update_physics();

	};


	
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::handle_type 
		phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::try_queue_item_to_add(new_collider_data&& data_for_new_collider)
	{
		//try and get a free handle 
		typename handle_data_lookup_system_type::index_type index = handle_manager.get_free_element();

		//check that index is valid 
		if (!handle_data_lookup_system_type::is_valid_index(index))
		{
			//if the handle is invalid dont queue data and just return an invalid handle
			return handle_type::get_invalid_index();
		}

		//create the handle 
		handle_type handle = handle_type(index);

		//convert from position to sector and tile 

		//convert to tile
		math_2d_util::ivec2d tile_xy = static_cast<math_2d_util::ivec2d>(data_for_new_collider.position);

		//use grid helper to get sector
		uint32 sector_index = grid_helper.to_sector_index(tile_xy);

		//check if this is the first item in this sector
		bool is_first_in_sector = !collider_to_add_header.y_axis_count[sector_index];

		//if this is the first in the sector add to the active sector list
		sectors_with_queued_items.push_back(sector_index, is_first_in_sector);

		//alocate space in the in buffer for the data 
		auto address_to_add_item_at = collider_to_add_header.push_back(sector_index);

		//store the data about the new item to add 
		colliders_to_add_data[address_to_add_item_at.address] = std::move(data_for_new_collider);

		//update the handle 
		colliders_to_add_data[address_to_add_item_at.address].owner = handle;

		return handle;
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_items_from_sector(sector_count_type sector_index)
	{
		//get index iterator from the item add header 
		std::for_each(collider_to_add_header.begin(sector_index), collider_to_add_header.end(sector_index), [&](auto real_address_to_add)
			{
				//get a ref struct to the existing data
				new_collider_data& existing_data = colliders_to_add_data[real_address_to_add.address];

				//add to the target sector 
				auto [destination__real_address, _, __]  = collision_data_container.insert(existing_data.owner, sector_index);

				//get the ref struct to the data we are writing to
				auto data_ref = collision_data_container.get(destination__real_address);

				//copy across the values (could maybe make a meta function for this)
				data_ref.x = existing_data.position.x;
				data_ref.y = existing_data.position.y;

				data_ref.velocity_x = existing_data.velocity.x;
				data_ref.velocity_y = existing_data.velocity.y;

				data_ref.radius = existing_data.radius;
			});

		//inject the handle into the per tile trakcer
		std::for_each(collider_to_add_header.begin(sector_index), collider_to_add_header.end(sector_index), [&](auto real_address_to_add)
			{
				//get a ref struct to the existing data
				new_collider_data& existing_data = colliders_to_add_data[real_address_to_add.address];

				//get the handle 
				auto handle = existing_data.owner;

				//convert to tile
				math_2d_util::ivec2d tile_xy = static_cast<math_2d_util::ivec2d>(existing_data.position);

				//use grid helper to get sector
				auto sector_index = grid_helper.from_xy(tile_xy);

				//inject the handle into the grid structure 
				colliders_in_tile_tracker.add(sector_index.index, handle);
			});
	}



	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_items_from_all_sectors()
	{
		//single threaded version.
		//loop through all sectors and add their items into the simulation
		std::for_each(sectors_with_queued_items.begin(), sectors_with_queued_items.end(), [&](auto sector_index)
			{
				add_items_from_sector(sector_index);
			});
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_bounds_in_sector(sector_count_type sector_index)
	{
		//get the iterators for the sector
		auto begin_itr = colliders_in_tile_tracker.get_active_nodes_in_group_start(sector_index);
		auto end_itr = colliders_in_tile_tracker.get_active_nodes_in_group_end(sector_index);

		//sector offset 
		auto tile_offset = sector_index * grid_dimension_type::sector_tile_count;

		//loop through all active nodes in the sector
		std::for_each(begin_itr, end_itr, [&](auto root_index)
			{
				//convert from subtile to global tile 
				auto world_tile = root_index + tile_offset;

				//get the iterator for the tile
				auto tile_start = colliders_in_tile_tracker.get_root_node_start(world_tile);
				auto tile_end = colliders_in_tile_tracker.end();

				//the rect for the tile
				math_2d_util::irect new_bounds = math_2d_util::irect::inverse_max_size_rect();

				using sector_type_type = typename sector_grid_helper_type::sector_tile_index_type;

				//convert to a tile type
				sector_type_type tile = sector_type_type{ world_tile };

				//get the tils coordinate
				math_2d_util::ivec2d tile_coordinate = grid_helper.to_xy<math_2d_util::ivec2d>(tile);

				//calculate the bounds for each agent
				std::for_each(tile_start, tile_end,
					[&](auto handle)
					{
						//get ref struct 
						typename collision_data_container_type::handle_reference_wrapper ref_struct = collision_data_container.get(handle);

						//calcualte x min max
						float x_min = ref_struct.x - ref_struct.radius;
						float x_max = ref_struct.x + ref_struct.radius;

						//calculate y min max 
						float y_min = ref_struct.y - ref_struct.radius;
						float y_max = ref_struct.y + ref_struct.radius;

						new_bounds.min.x = std::min(new_bounds.min.x, static_cast<int32>(x_min));
						new_bounds.max.x = std::max(new_bounds.max.x, static_cast<int32>(x_max));

						new_bounds.min.y = std::min(new_bounds.min.y, static_cast<int32>(y_min));
						new_bounds.max.y = std::max(new_bounds.max.y, static_cast<int32>(y_max));

					});

				//update the bounds of the tile
				overlap_grid.update_bounds(tile_coordinate, tile, new_bounds);

			});
	}


	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_bounds_in_all_sectors()
	{
		//single threaded version.
		//loop through all sectors and add their items into the simulation
		std::for_each(sectors_with_queued_items.begin(), sectors_with_queued_items.end(), [&](auto sector_index)
			{
				update_bounds_in_sector(sector_index);
			});
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_all_positions()
	{
		//single threaded version.
		//loop through all sectors and add their items into the simulation
		std::for_each(sectors_with_queued_items.begin(), sectors_with_queued_items.end(), [&](auto sector_index)
			{
				update_positions_in_sector(sector_index);
			});
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_physics()
	{
		//add any new items to the simulation 
		add_items_from_all_sectors();

		//move all objects 
		update_all_positions();

		//update the tile boundry overlaps 
		update_bounds_in_all_sectors();


		//object ask the physics system for a handle to a phys object
		
		//there is a map from the handle to the address in per sector data 

		//each object in the world registers movement commands and uses their handle to pass movement forces into the system

		//update the agent positions inside a sector 

		//loop through all the active sectors and update the posititons of all the active agents 

		//for each sector, for each active tile
		
		//get the list of all the other active tiles they have potential overlaps with
		
		//for each agent in the tile

		//check if agent overlaps with the bounds of the other overlap
		
		//gather all the agent id's inside the tile
		
		//use the agent id's to gather the real addresses for each of the potential collider list   
		
		//loop through the real addresses and compare flags to see what kind of collisions we are talking about 
		
		//for first pass do a discrete sphere check and if collision detected add to list of collisions 

		//loop through list of collisions and calculate repulsion forces 

		//sum up all the repulsion forces 

		//apply force to agent 
		
	}
	
	//move all objects 
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_positions_in_sector(sector_count_type sector_index)
	{
		//get the page data iterators
		auto page_begin_itr = collision_data_container.get_tight_packed_data().get_array_header().page_begin(sector_index);
		auto page_end_itr = collision_data_container.get_tight_packed_data().get_array_header().page_end(sector_index);

		//loop through all the pages that a sectors data is in
		std::for_each(page_begin_itr, page_end_itr, [&](auto& page_address_and_count)
			{

				//flip velocity if it will take the agent off the map
				for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
				{
					//get ref struct 
					collision_data_ref ref_struct = collision_data_container.get(real_address);

					float new_max_edge = (ref_struct.x + ref_struct.radius) + ref_struct.velocity_x;

					float new_min_edge = (ref_struct.x - ref_struct.radius) + ref_struct.velocity_x;

					//check if this will take the object off the bottom of the map
					bool will_take_off_map = new_max_edge > grid_dimension_type::tile_w || new_min_edge < 0;

					//flip the x velocity 
					ref_struct.velocity_x = will_take_off_map ? -ref_struct.velocity_x : ref_struct.velocity_x;
				}

				for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
				{
					//get ref struct 
					collision_data_ref ref_struct = collision_data_container.get(real_address);

					float new_max_edge = (ref_struct.y + ref_struct.radius) + ref_struct.velocity_y;

					float new_min_edge = (ref_struct.y - ref_struct.radius) + ref_struct.velocity_y;

					//check if this will take the object off the bottom of the map
					bool will_take_off_map = new_max_edge > grid_dimension_type::tile_w || new_min_edge < 0;

					//flip the x velocity 
					ref_struct.velocity_y = will_take_off_map ? -ref_struct.velocity_y : ref_struct.velocity_y;
				}


				//cant be bothered writing anothre iterator 
				//apply x movement
				for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
				{
					//get ref struct 
					collision_data_ref ref_struct = collision_data_container.get(real_address);

					//apply the x velocity to the x position 
					ref_struct.x += ref_struct.velocity_x;
				}

				//apply y movement 
				for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
				{
					//get ref struct 
					collision_data_ref ref_struct = collision_data_container.get(real_address);

					//apply the x velocity to the x position 
					ref_struct.y += ref_struct.velocity_y;
				}

			});
	}
}