#pragma once
#include <array>


#include "base_types_definition.h"
#include "misc_utilities/int_type_selection.h"
#include "continuous_collision_library/overlap_tracking_grid.h"
#include "continuous_collision_library/HandleSystem/handle_system.h"
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

		//the type of data structure used to track the data for all the agents in a sector 
		using paged_data_header_type = ArrayUtilities::paged_2d_array_header< sector_count, Imax_objects, Imax_objects, page_size_for_agent_data>;

		//the address type used to find an address in the per sector data lookup array
		using combined_virtual_memory_address_type = paged_data_header_type::combined_address_virtual_memory_map_type;

		//the real address type 
		using real_address_type = paged_data_header_type::real_node_address_type;

		//header to track what index a given piece of data for an object in a sector should be located 
		paged_data_header_type object_data_header;

		//list of all available handles 
		HandleSystem::handle_system<Imax_objects> handles;

		//grid tracking collisions 
		overlap_tracking_grid overlap_grid;

		//lookup table for the address in memory for the given handle 
		std::array<combined_virtual_memory_address_type, Imax_objects> handle_address_lookup;

		void update_physics();

	};
	
	template<size_t Imax_objects, size_t Iworld_sector_x_count, size_t Iworld_sector_y_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count, Iworld_sector_y_count>::update_physics()
	{
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

		//apply force to agent 
		
	}
}