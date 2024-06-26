#pragma once

#include <array>
#include <type_traits>
#include <iostream>

#include "tight_packed_paged_2d_array.h"
#include "HandleSystem/handle_system.h"
#include "StructOfArraysHelper/struct_of_arrays.h"


//this data structure is intended to wrap a number of raw data arrays
//to add data to the structure you first must get a handle
//once you have aquired a handle you can use it to read and write to data associated with that handle 
//once you are done with the handle you can return it to the handle pool and all the associated data will be returned 
//under the hood this struct uses the memeory page and virtual address systems this allows you to add up to max data to a single 
//y axis array without needing all y axis arrays to be big enough to hold the worst case data amount 
namespace ArrayUtilities
{
	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	struct handle_tracked_2d_paged_array
	{

		using handle_type = Thandle_type;

		//this index type for the handle 
		using handle_index_type = Thandle_type::handle_index_type;

		//this is used to splice the parent handle onto the start of the reference struct 
		struct handle_reference_wrapper : Treference_struct
		{
			Thandle_type& handle;

			auto get_as_tuple()
			{
				return std::tuple_cat(Treference_struct::get_as_tuple(), std::tie(handle));
			}
		};

		using reference_tuple_type = decltype(std::declval<handle_reference_wrapper>().get_as_tuple());

		//forward declare the headerr so we can get the max number of elements we need
		using paged_array_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;
		
		// the address type we use to translate a handle index to a index in the packed arrays
		using virtual_combined_node_adderss_type = paged_array_type::virtual_combined_node_adderss_type;

		//max possible entries needed by the system to support the paged array setup
		static constexpr size_t max_total_entries = paged_array_type::max_total_entries;

		// the tuple of arrays type that holds all the data
		using container_type = struct_of_arrays_with_ref_struct<handle_reference_wrapper, max_total_entries>::tuple_array_transferable_type;

		//the array manager we are wrapping and addeing handle tracking to 
		using tight_packed_array_type = tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, container_type>;

		//the real address type
		using real_address_type = tight_packed_array_type::real_address_type;

		using address_return_type = paged_array_type::address_return_type;

		using reserve_space_for_move_return_type = typename tight_packed_array_type::expand_return_type;

		//type used to find an element on an axis
		using x_axis_type = tight_packed_array_type::x_axis_type;

		//the array for finding the address of the data that belongs to a handle 
		std::array<virtual_combined_node_adderss_type, Imax_total_y_items> handle_to_data_lookup;

		//data structure holding all the data including the poiter to the handle each bit of data belongs to 
		tight_packed_array_type	tight_packed_data;

		const tight_packed_array_type& get_tight_packed_data() const { return tight_packed_data; };

		address_return_type insert(Thandle_type handle, x_axis_type x_index_to_add_to);

		//change where a handle is pointing to and get a ref struct so the data can be overwritten;
		handle_reference_wrapper overwrite(Thandle_type handle, real_address_type real_address, virtual_combined_node_adderss_type address);

		//reserve_space_for_move 
		reserve_space_for_move_return_type reserve_space_for_move(x_axis_type axis_to_move_to, typename paged_array_type::y_axis_count_type number_to_add);

		address_return_type move(x_axis_type x_index_to_add_to, auto address_to_move_from);

		void remove(Thandle_type handle);

		//this is used when doing a bulk replace 
		void remove_without_updating_handle(x_axis_type x_index_to_remove_from, real_address_type real_address);

		//used when moving data arround instead of adding or removing 
		void update_handle_address(Thandle_type handle, virtual_combined_node_adderss_type address);

		reference_tuple_type get_ref_tuple(auto address);

		handle_reference_wrapper get(auto address);

#pragma region Iterators

#pragma endregion


	};


	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::address_return_type 
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::insert(Thandle_type handle, x_axis_type x_index_to_add_to)
	{
		//allocate 
		address_return_type return_address = tight_packed_data.add_item_to_paged_array_unsafe(x_index_to_add_to);

		auto ref_tuple = tight_packed_data.get_reference_to_address(std::get<0>(return_address));

		static_assert(std::is_same<decltype(std::declval<handle_reference_wrapper>().get_as_tuple()), decltype(ref_tuple)::base_type>::value);

		//use the real address to get a ref to the data 
		handle_reference_wrapper ref_to_new_data = struct_of_arrays_helper<handle_reference_wrapper>::create_ref_struct_from_tuple(ref_tuple);
		
		//set the handle this data belongs to 
		ref_to_new_data.handle = handle;
		
		//get the index of the handle 
		handle_index_type handle_index = handle.get_index();
		
		//set the address of data 
		handle_to_data_lookup[handle_index] = std::get<1>(return_address);

		return return_address;
	}


	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::handle_reference_wrapper 
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::overwrite(Thandle_type handle, real_address_type real_address, virtual_combined_node_adderss_type address)
	{
		handle_reference_wrapper overwrite_target = get(real_address);

		//point data to new handle 
		overwrite_target.handle = handle;

		//change where the source handle points to
		handle_index_type handle_index = handle.get_index();

		//set the address of data 
		handle_to_data_lookup[handle_index] = address;

		//return the ref wrapper for other code to overwrite 
		return overwrite_target;
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline typename handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::reserve_space_for_move_return_type
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::reserve_space_for_move(x_axis_type axis_to_move_to, typename paged_array_type::y_axis_count_type number_to_add)
	{
		//expand the array and return the iterators for accessing the new addresses 
		return tight_packed_data.add_item_range_to_paged_array_unsafe(axis_to_move_to, number_to_add);
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::address_return_type 
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::move(x_axis_type x_index_to_add_to, auto address_to_move_from)
	{
		return tight_packed_data.move(x_index_to_add_to, address_to_move_from);
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline void handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::remove(Thandle_type handle)
	{
		// get the handle index
		handle_index_type handle_index = handle.get_index();

		//lookup the address of the data
		virtual_combined_node_adderss_type virtual_address = handle_to_data_lookup[handle_index];

		//remove replace the data at that address
		real_address_type address_of_removed_item = tight_packed_data.remove_item_from_paged_array(virtual_address);

		//get the data that was used to replace the item removed
		Treference_struct ref_to_new_data = tight_packed_data.get_reference_to_address(std::get<0>(address_of_removed_item));

		//get the handle of the replacement item
		Thandle_type replacement_handle = ref_to_new_data.handle;

		//get the replacement handle index
		handle_index_type replacement_handle_index = replacement_handle.get_index();

		//update the replacement handle address
		handle_to_data_lookup[handle_index] = virtual_address;
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline void handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::remove_without_updating_handle(x_axis_type x_index_to_remove_from, real_address_type real_address)
	{
		//remove replace the data at that address
		tight_packed_data.remove_item_from_paged_array(x_index_to_remove_from, real_address);
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline void handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::update_handle_address(Thandle_type handle, virtual_combined_node_adderss_type address)
	{
		// get the handle index
		handle_index_type handle_index = handle.get_index();

		handle_to_data_lookup[handle_index] = address;

	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::reference_tuple_type 
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::get_ref_tuple(auto address)
	{
		if constexpr (std::is_same<decltype(address), Thandle_type>::value)
		{
			// get the handle index
			handle_index_type handle_index = address.get_index();

			//lookup the address of the data
			virtual_combined_node_adderss_type virtual_address = handle_to_data_lookup[handle_index];

			return tight_packed_data.get_reference_to_address(virtual_address);
		}
		else
		{
			return tight_packed_data.get_reference_to_address(address);
		}
	}

	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename Treference_struct>
	inline  handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::handle_reference_wrapper 
		handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Treference_struct>::get(auto address)
	{
		return  struct_of_arrays_helper<handle_reference_wrapper>::create_ref_struct_from_tuple(get_ref_tuple(address));
	}

}