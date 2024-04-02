#pragma once

#include <tuple>
#include <iterator>
#include <functional>

#include "paged_2d_array.h"
#include "misc_utilities/type_static_assert_helper.h"

//this class serves as an array of arrays where the first axis is meant to represent "groups" and the 2nd axis the data for members of those groups with an upper limit on the
// total number of members in any group. the 2nd axis of elements act as if they are in contigious areas of memeory but are actually split across memory pages 
//this is to allow the overall data structure to scale to large numbers of groups while not taking up exponential amounts of space eg a map split up into 1024 sectors
// will not take up 1024 * max number of entries in any group

namespace ArrayUtilities
{
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size,typename Tcontainer>
	struct tight_packed_paged_2d_array_manager
	{
		//a tightly packed array
		using paged_array_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;
	
		//the type used to select a "group" to add to 
		using x_axis_type = paged_array_type::x_axis_count_type;
	
		//the address to look up items in the array 
		using real_address_type = paged_array_type::real_node_address_type;
	
		using virtual_y_axis_node_adderss_type = paged_array_type::virtual_y_axis_node_adderss_type;

		using virtual_combined_node_adderss_type = paged_array_type::virtual_combined_node_adderss_type;

		//address details type 
		using address_return_type = paged_array_type::address_return_type;

		using expand_return_type = std::tuple< typename paged_array_type::y_real_address_iterator, typename paged_array_type::y_real_address_iterator>;
	
		//the data structure used to house the different arrays 
		using container_type = Tcontainer;

	private:

		//paged array manager
		paged_array_type paged_array_header;
	
		//the data to keep tightly packed
		//repacking is done using 
		container_type packed_data;
		
		//take a data type that supports itterators and use them to move data from one address to another
		//this is used to keep the array tightly packed 
		static void replace_remove_internal(auto& datatype_to_modifiy, real_address_type replace_from, real_address_type replace_to);

		//this code will be for managing dynamic size data types, something not needed in this implementation

		//get number of elements allocated 
		//static real_address_type get_allocated_size(const auto& object_to_check_size);

		//increase the size of the object by at least 1
		//static void allocate_additional_space(auto& object_to_expand);

	public:

		//iterator 
#pragma region Iterator



#pragma endregion


		//get a const version of the paged array for external access 
		const paged_array_type& get_array_header() const { return paged_array_header; };

		//add an item to the paged array
		address_return_type add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to);

		//expands the paged array by the given type
		expand_return_type add_item_range_to_paged_array_unsafe(x_axis_type x_index_to_add_to, paged_array_type::y_axis_count_type number_to_add);
	
		//remove an element only using a combined address
		real_address_type remove_item_from_paged_array(virtual_combined_node_adderss_type address_to_remove);

		//removes element and returns address of data that was replaced
		real_address_type remove_item_from_paged_array(x_axis_type x_index_to_remove_from, virtual_combined_node_adderss_type address_to_remove);

		//find the data at the given address 
		real_address_type resolve_address(auto... args) const;

		//get a iterator to the data at an address
		auto get_iterator_address(auto address);

		//get a reference to the data at an address 
		decltype(auto) get_reference_to_address(auto address);

		//move data from one x address to another 
		address_return_type move(x_axis_type x_index_move_to, auto address);

	};
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size,typename Tcontainer>
	void tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::replace_remove_internal(auto& datatype_to_modifiy, real_address_type replace_from, real_address_type replace_to)
	{
		//get iterator pointing to the read location
		auto read_it = datatype_to_modifiy.begin() + replace_from.address;
		
		//itterator opinting to write address
		auto write_it = datatype_to_modifiy.begin() + replace_to.address;

		//copy the values across
		*write_it = *read_it;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::address_return_type 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::add_item_to_paged_array_unsafe(x_axis_type x_index_to_add_to)
	{
		//check that the underlying types have enough data allocated 

		//we are doing nothing to the data so we can safely return it
		return paged_array_header.push_back_and_return_address(x_index_to_add_to);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::expand_return_type
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::add_item_range_to_paged_array_unsafe(x_axis_type x_index_to_add_to, paged_array_type::y_axis_count_type number_to_add)
	{
		//get the current number of items in the y axis
		auto current_item_count = paged_array_header.y_axis_count[x_index_to_add_to];
		auto new_item_count = current_item_count + number_to_add;

		//expand the address space in the header 
		paged_array_header.expand(x_index_to_add_to, new_item_count);

		//create the start and end address iterators
		auto begin_itr = paged_array_header.begin(x_index_to_add_to) + current_item_count;
		auto end_itr = paged_array_header.end(x_index_to_add_to);

		//return tuple with start and end iterator 
		//this will allow the caller to iterate over all the newly created addresses 
		return expand_return_type(begin_itr, end_itr);
	}


	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::real_address_type
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::remove_item_from_paged_array(virtual_combined_node_adderss_type address_to_remove)
	{
		//calculate x address using combined address
		x_axis_type x_address = paged_array_type::convert_from_combined_virtual_address_to_x(address_to_remove);

		return remove_item_from_paged_array(x_address, address_to_remove);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline  tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::real_address_type
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::remove_item_from_paged_array(x_axis_type x_index_to_remove_from, virtual_combined_node_adderss_type address_type)
	{

		//convert replacement address 
		//do this first as if this address is the same as the address to remove the 
		//next line will invalidate the page handle it is on
		auto remove_real_address = paged_array_header.find_address(address_type);


		//reduce the number of elelments in the list and get the address of the last entry
		auto replacment_element_address = paged_array_header.pop_back_and_return_address(x_index_to_remove_from);

		//use iterators to get the last and first addresses
		replace_remove_internal(packed_data, std::get<0>(replacment_element_address), remove_real_address);
		
		//return the address of the items moved 
		//this is so other systems can update the index of tracked data
		return remove_real_address;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::real_address_type 
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::resolve_address(auto... args) const
	{
		//forward args to paged array header 
		return paged_array_header.find_address(args...);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline auto tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::get_iterator_address(auto address)
	{
		static_assert(std::is_same<real_address_type, decltype(address)>::value || std::is_same<virtual_combined_node_adderss_type, decltype(address)>::value);

		real_address_type real_address;

		//convert from virtual to real
		if constexpr (std::is_same<real_address_type, decltype(address)>::value)
		{
			//get the iterator to the target and apply the offset
			return packed_data.begin() + address.address;
		}
		else if constexpr (std::is_same<virtual_combined_node_adderss_type, decltype(address)>::value)
		{
			real_address = paged_array_header.find_address(address);

			return packed_data.begin() + real_address.address;

		}
		else
		{	
			MiscUtilities::debug_error_type<real_address_type> type_error0;
			//this code is only compiled if the address is of the wrong type
			//static_assert(false);
		}

	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline decltype(auto) tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::get_reference_to_address(auto address)
	{
		// TODO: insert return statement here
		return *get_iterator_address(address);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Tcontainer>
	inline tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::address_return_type
		tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tcontainer>::move(x_axis_type x_index_move_to, auto address)
	{
		//get ref to the current data 
		auto current_data_itr = get_iterator_address(address);

		//alocate new data to move to 
		address_return_type address_of_new_data = add_item_to_paged_array_unsafe(x_index_move_to);

		//get reference to the new data 
		auto new_data_itr = get_iterator_address(std::get<0>(address_of_new_data));

		//transfer the data 
		*new_data_itr = *current_data_itr;

		return address_of_new_data;
	}

	
}