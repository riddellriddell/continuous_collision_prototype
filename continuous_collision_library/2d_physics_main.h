#pragma once
#include <array>


#include "base_types_definition.h"
#include "misc_utilities/int_type_selection.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"
#include "continuous_collision_library/overlap_tracking_grid.h"
#include "array_utilities/fixed_free_list.h"
#include "array_utilities/paged_2d_array.h"



namespace ContinuousCollisionLibrary
{
	template<size_t Imax_objects, size_t Iworld_sector_x_count, size_t Iworld_sector_y_count>
	class phyisics_2d_main
	{
		static constexpr size_t page_size_for_agent_data = 1024;

		static constexpr size_t max_sectors_internal = Iworld_sector_x_count * Iworld_sector_y_count;

		using sector_count_type = MiscUtilities::uint_s<max_sectors_internal>::uint_t;

		static constexpr sector_count_type sector_count = max_sectors_internal;

		static constexpr size_t bits_needed_to_store_handle_count = MiscUtilities::bits_needed_to_represent_number(Imax_objects);

		//temp handle type definition
		using handle_type = uint32_t;


		using handle_data_lookup_system_type = fixed_free_list<Imax_objects, handle_type>;

		//the system to manage all the handles and data lookup addresses 
		handle_data_lookup_system_type handle_manager;

		//grid tracking collisions 
		overlap_tracking_grid overlap_grid;

		void update_physics();

		//get a handle from the handle system
		handle_type 

	};
	
	template<size_t Imax_objects, size_t Iworld_sector_x_count, size_t Iworld_sector_y_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count, Iworld_sector_y_count>::update_physics()
	{
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