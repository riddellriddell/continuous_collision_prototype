#pragma once

#include <array>

#include "tight_packed_paged_2d_array.h"
#include "HandleSystem/handle_system.h"


//this data structure is intended to wrap a number of raw data arrays
//to add data to the structure you first must get a handle
//once you have aquired a handle you can use it to read and write to data associated with that handle 
//once you are done with the handle you can return it to the handle pool and all the associated data will be returned 
//under the hood this struct uses the memeory page and virtual address systems this allows you to add up to max data to a single 
//y axis array without needing all y axis arrays to be big enough to hold the worst case data amount 
namespace ArrayUtilities
{
	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename  ... Tmanaged_array >
	struct handle_tracked_2d_paged_array
	{
		//this index type for the handle 
		using handle_index_type = Thandle_type::handle_index_type;

		//forwad declaring the paged array type
		//this is needed to get the combined_virtual_address_type
		using paged_array_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;

		//the max number of entities needed 
		static constexpr size_t max_total_entries = paged_array_type::max_total_entries;

		//the type for mapping a real address to the handle that ownes it
		using address_to_handle_type = std::array<Thandle_type, max_total_entries>;

		//the type for handling the arrary packing 
		//we add in 
		using tight_packed_array_type = tight_packed_paged_2d_array_manager<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, address_to_handle_type, Tmanaged_array...>;

		//the address type we use
		using combined_virtual_address_type = tight_packed_array_type::combined_virtual_address_type;

		//the real address type
		using real_address_type = tight_packed_array_type::real_address_type;

		//type used to find an element on an axis
		using x_axis_type = paged_array_type::x_axis_count_type;

		//the type for finding the address of the data that belongs to a handle 
		using handle_to_address_type = std::array<combined_virtual_address_type, Imax_total_y_items>;

		//constructor 
		handle_tracked_2d_paged_array(const Tmanaged_array&& ...arrays_to_manage);

		void add_handle(Thandle_type handle, x_axis_type index_to_add_to);

		void remove_handle(Thandle_type handle);

		//real_address

		//the type of array we use to look up the 
		address_to_handle_type address_to_handle_array;

		//lookup array to use to convert from handle to index in the packed arrays 
		handle_to_address_type handle_index_to_address_array;

		//packed array manager, this class 
		tight_packed_array_type packed_array_manager;

	};


	template<typename Thandle_type, size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size, typename ...Tmanaged_array>
	inline handle_tracked_2d_paged_array<Thandle_type, Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size, Tmanaged_array...>::handle_tracked_2d_paged_array(const Tmanaged_array&& ...arrays_to_manage) : packed_array_manager( std::forward<arrays_to_manage> arrays_to_manage...)
	{


	}
}