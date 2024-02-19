#pragma once
#include <array>
#include <algorithm>

#include "base_types_definition.h"
#include "misc_utilities/int_type_selection.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"
#include "continuous_collision_library/overlap_tracking_grid.h"
#include "array_utilities/fixed_free_list.h"
#include "array_utilities/paged_2d_array.h"
#include "array_utilities/handle_tracked_2d_paged_array.h"
#include "array_utilities/HandleSystem/handle_system.h"
#include "array_utilities/fixed_size_vector.h"

#include "sector_grid_data_structure/sector_grid_dimensions.h"
#include "sector_grid_data_structure/sector_grid_index.h"


namespace ContinuousCollisionLibrary
{
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	class phyisics_2d_main
	{
		//definition of the target dimensions of the grid system
		using grid_dimension_type = SectorGrid::sector_grid_dimensions<Iworld_sector_x_count, 16>;

		using tile_coordinate_type = SectorGrid::sector_tile_index<grid_dimension_type>;

		//use the sector tile helper to convert to sector
		using sector_grid_helper_type = SectorGrid::sector_grid_helper<grid_dimension_type>;

		//sector grid instance this currently holds nothing but am thinking about making sizing at load not compile 
		sector_grid_helper_type grid_helper;

		static constexpr size_t page_size_for_collider_data = 1024;

		static constexpr size_t max_sectors_internal = grid_dimension_type::sector_grid_count_max;

		using sector_count_type = MiscUtilities::uint_s<max_sectors_internal>::uint_t;

		static constexpr sector_count_type sector_count = max_sectors_internal;

		static constexpr size_t bits_needed_to_store_handle_count = MiscUtilities::bits_needed_to_represent_number(Imax_objects);

		//temp handle type definition
		using handle_type = HandleSystem::default_handle_type<Imax_objects>;

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
		};

		//data for all the colliders that is tied to / tracked with unique handle id's from the handle manager
		using collision_data_container_type = ArrayUtilities::handle_tracked_2d_paged_array<handle_type, sector_count, Imax_objects, Imax_objects, page_size_for_collider_data, collision_data_ref>;

		//data for all the items in the grid
		collision_data_container_type collision_data_container;

		//buffer for new data to add to the grid
		struct new_collider_data
		{
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


		//buffer to hold all data being added to the grid 
		using new_collider_header = ArrayUtilities::paged_2d_array_header < sector_count, Imax_objects, Imax_objects, page_size_for_collider_data>;

		using new_collider_data = std::array<new_collider_data, new_collider_header::max_total_entries>;

		new_collider_header collider_to_add_header;
		new_collider_data colliders_to_add_data;
		
		//which sectors have items queued to be added into the game
		ArrayUtilities::fixed_size_vector_array<sector_count_type, max_sectors_internal> sectors_with_queued_items;

		sector_count_type number_of_active_sectors;
		std::array<sector_count_type, max_sectors_internal> active_sectors;

		//grid tracking collisions 
		overlap_tracking_grid overlap_grid;

		//try add a handle to the simulation
		handle_type try_queue_item_to_add( new_collider_data&& data_for_new_collider);

		//add all the queued items for a sector into the physics system 
		void add_items_from_sector(sector_count_type sector_index);

		//add queued items 
		void update_physics();
	};


	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::handle_type 
		phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::try_queue_item_to_add(new_collider_data&& data_for_new_collider)
	{
		//try and get a free handle 
		handle_data_lookup_system_type::index_type index = handle_manager.get_free_element();

		//check that index is valid 
		if (!handle_data_lookup_system_type::is_valid_index(index))
		{
			//if the handle is invalid dont queue data and just return an invalid handle
			return handle_type::invalid_handle_value;
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
		colliders_to_add_data[address_to_add_item_at] = std::move(data_for_new_collider);

		//update the handle 
		colliders_to_add_data[address_to_add_item_at].handle = handle;

		return handle;
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_items_from_sector(sector_count_type sector_index)
	{
		//get index iterator from the item add header 
		std::for_each(collider_to_add_header.begin(sector_index), collider_to_add_header.end(sector_index), [](auto real_address_to_add)
			{
				//get a ref struct to the existing data
				new_collider_data& existing_data = &colliders_to_add_data[real_address_to_add.address];

				//add to the target sector 
				auto [destination__real_address, _, _]  = collision_data_container.insert(existing_data.owner, sector_index);

				//get the ref struct to the data we are writing to
				collision_data_ref data_ref = collision_data_container.get_ref_tuple(destination__real_address);

				//copy across the values (could maybe make a meta function for this)
				data_ref.x = existing_data.position.x;
				data_ref.y = existing_data.position.y;

				data_ref.velocity_x = existing_data.velocity.x;
				data_ref.velocity_y = existing_data.velocity.y;

				data_ref.radius = existing_data.radius;
			};
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_physics()
	{
		//add any new items to the simulation 

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


}