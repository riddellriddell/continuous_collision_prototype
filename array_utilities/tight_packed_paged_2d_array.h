#pragma once

#include <tuple>
#include <iterator>

#include "paged_2d_array.h"

//this class serves as an array of arrays where the first axis is meant to represent "groups" and the 2nd axis the data for members of those groups with an upper limit on the
// total number of members in any group. the 2nd axis of elements act as if they are in contigious areas of memeory but are actually split across memory pages 
//this is to allow the overall data structure to scale to large numbers of groups while not taking up exponential amounts of space eg a map split up into 1024 sectors
// will not take up 1024 * max number of entries in any group

namespace ArrayUtilities
{
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename  ... Tmanaged_array >
	struct tight_packed_paged_2d_array_manager
	{
		//a tightly packed array
		using paged_array_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;
	
		//the type used to select a "group" to add to 
		using x_axis_type = paged_array_type::x_axis_count_type;
	
		//the address to look up items in the array 
		using real_address_type = paged_array_type::real_node_address_type;
	
		using combined_virtual_address_type = paged_array_type::combined_address_virtual_memory_map_type;

		//address details type 
		using address_return_type = paged_array_type::address_return_type;
	
		//the data structure used to house the different arrays 
		using managed_data_type = std::tuple<Tmanaged_array ...>;

	private:

		//paged array manager
		paged_array_type paged_array_header;
	
		//the data to keep tightly packed
		managed_data_type packed_data;


		//take a data type that supports itterators and use them to move data from one address to another
		//this is used to keep the array tightly packed 
		void replace_remove_internal(auto& datatype_to_modifiy, real_address_type replace_from, real_address_type replace_to)const;

	public:

		//get a const version of the paged array for external access 
		const paged_array_type& get_array_header() const {return paged_array_header};

		//add an item to the paged array
		address_return_type add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to);
	
		//removes element and returns address of data that was replaced
		address_return_type remove_item_from_paged_array(x_axis_type x_index_to_remove_from, combined_virtual_address_type address_to_remove);

		//find the data at the given address 
		real_address_type resolve_address(auto... args) const;

	};
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename ...Tmanaged_array>
	inline void tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::replace_remove_internal(auto& datatype_to_modifiy, real_address_type replace_from, real_address_type replace_to) const
	{
		//get iterator pointing to the read location
		auto read_it = datatype_to_modifiy.begin() + replace_from;
		
		//itterator opinting to write address
		auto write_it = datatype_to_modifiy.begin() + replace_to;

		//copy the values across
		*write_it = *read_it;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename ...Tmanaged_array>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::address_return_type 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to)
	{
		//we are doing nothing to the data so we can safely return it
		return paged_array_header.push_back_and_return_address(x_index_to_add_to);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename ...Tmanaged_array>
	inline  tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::address_return_type 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::remove_item_from_paged_array(x_axis_type x_index_to_remove_from, combined_virtual_address_type address_type)
	{
		//reduce the number of elelments in the list and get the address of the last entry
		auto replacment_element_address = paged_array_header.pop_back_and_return_address(x_index_to_remove_from);

		//convert replacement address 
		auto remove_real_address = paged_array_header.find_address(address_type);

		//loop through all the arrays and move the data 
		std::apply(packed_datastd::apply([](auto& ...data) {(..., replace_remove_internal(data, replacment_element_address.address, remove_real_address.address)); }, the_tuple););

		//return the address of the items moved 
		//this is so other systems can update the index of tracked data
		return remove_real_address;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename ...Tmanaged_array>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::real_address_type 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::resolve_address(auto... args) const
	{
		//forward args to paged array header 
		return paged_array_header.find_address(args);
	}

	
}