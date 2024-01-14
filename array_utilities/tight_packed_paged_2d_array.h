#pragma once

#include <tuple>

#include "paged_2d_array.h"

//this class serves as an array of arrays where the firs axis is meant to represent "groups" and the 2nd axis the data for members of those groups with an upper limit on the
// total number of members in any group. the 2nd axis of elements act as if they are in contigious areas of memeory but are actually split across memory pages 
//this is to allow the overall data structure to scale to large numbers of groups while not taking up exponential amounts of space eg a map split up into 1024 sectors
// will not take up 1024 * max number of entries in any group

namespace ArrayUtilities
{
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Ttuple_of_managed_arrays>
	struct tight_packed_paged_2d_array_manager
	{
		//a tightly packed array
		using paged_array_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;

		//the type used to select a "group" to add to 
		using x_axis_type = paged_array_type::x_axis_count_type;

		//the address to look up items in the array 
		using real_address_type = paged_array_type::real_node_address_type;

		using combined_virtual_address_type = paged_array_type::combined_address_virtual_memory_map_type;

		//paged array manager
		paged_array_type paged_array_header;

		//add an item to the paged array
		std::tuple< combined_virtual_address_type, real_address_type>  add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to);

		void remove_item_from_paged_array(combined_virtual_address_type address_type);
	};
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Ttuple_of_managed_arrays>
	inline std::tuple<
		typename tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Ttuple_of_managed_arrays>::combined_virtual_address_type, 
		typename tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Ttuple_of_managed_arrays>::real_address_type> 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Ttuple_of_managed_arrays>::add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to)
	{
		//add to the target array 
		paged_array_header.push_back(x_index_to_add_to);

		return std::tuple<combined_virtual_address_type, real_address_type>();
	}
}