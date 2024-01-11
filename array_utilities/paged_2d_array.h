#pragma once
#include <stdint.h>
//#include <math.h>
#include <algorithm>
#include <array>
#include <limits>
#include <assert.h>
#include <bit>

#include "array_utilities/SectorPackedArray/virtual_memory_map.h"
#include "array_utilities/SectorPackedArray/paged_memory_header.h"
#include "misc_utilities/int_type_selection.h"


namespace ArrayUtilities
{

	static constexpr size_t calculate_number_of_pages_needed(size_t number_of_x_axis_items, size_t max_y_items, size_t max_total_y_items, size_t page_size)
	{
		size_t max_y_axis_pages = ((max_y_items - 1) / page_size) + 1;

		size_t max_items_needed_for_max_y_pages = ((max_y_axis_pages - 1) * page_size) + 1;

		//how many pages can be filled if we just put one item in each x axis entry 
		size_t wasted_on_partial_fill = max_total_y_items - (std::min(max_total_y_items, (number_of_x_axis_items)));

		size_t entries_remaining = max_total_y_items - wasted_on_partial_fill;

		//number of y axises we can fill to max pages per axis 
		//one is subtraceted as we already alocate one on the wasted on spread calc
		size_t full_filled = entries_remaining / (max_items_needed_for_max_y_pages - 1);

		entries_remaining -= (full_filled * (max_items_needed_for_max_y_pages - 1));

		//take the remaining entries and fill one y axis as much as possible 
		size_t last_axis_page_count = (((entries_remaining + 1) - 1) / page_size) + 1;

		//the final number of pages is as follows
		//the total number of pages we can fill by adding only one item 
		//plus the extra pages we can create by filling each y axis to the max 
		//plus the pages we can create with the remaining itmes 
		size_t total_pages_needed = wasted_on_partial_fill + (full_filled * (max_y_axis_pages - 1)) + (last_axis_page_count - 1);

		return total_pages_needed;
	}

	template<size_t Inumber_of_x_axis_items,size_t Imax_y_items,size_t Imax_total_y_items, size_t Ipage_size>
	struct paged_2d_array_header
	{
		//the type we use to track how full the y axises are full
		using y_axis_count_type = MiscUtilities::uint_s<Imax_total_y_items + 1>::int_type_t;
		using x_axis_count_type = MiscUtilities::uint_s<Inumber_of_x_axis_items + 1>::int_type_t;

		//this array header supports accessing arrays using an x,y address or a combined index  
		//to support this there are 2 sets of virtual memory maps mapped to the same memory 
		//the correct one is accessed using type punning 
		//these 2 usings are used to define the 2 different map types 
		using y_axis_virtual_memory_map_type = virtual_memory_map<Ipage_size, Imax_y_items, Imax_total_y_items>;
		
		using combined_address_virtual_memory_map_type = virtual_memory_map<Ipage_size, Imax_y_items * Inumber_of_x_axis_items, Imax_total_y_items>;

		//the address type returned when pulling from the mem map
		using real_node_address_type = y_axis_virtual_memory_map_type::real_node_address_type;
	
		static constexpr y_axis_count_type invalid_y_axis_value = std::numeric_limits<y_axis_count_type>::max();
		static constexpr x_axis_count_type invalid_x_axis_value = std::numeric_limits<x_axis_count_type>::max();

		static constexpr uint32_t page_size = Ipage_size;

		//adds an item to the back of the array and if required alocates new memory pagex 
		y_axis_virtual_memory_map_type::real_node_address_type push_back(x_axis_count_type x_axis_index);
		
		//removes the last item in a y axis array and frees pages if needed 
		void pop_back(x_axis_count_type x_axis_index);

		//find the read write address 
		y_axis_virtual_memory_map_type::real_node_address_type find_address(x_axis_count_type x_axis_index, y_axis_virtual_memory_map_type::virtual_address_type virtual_address) const;
		y_axis_virtual_memory_map_type::real_node_address_type find_address(combined_address_virtual_memory_map_type::virtual_address_type virtual_address) const;

		//total amount of data that needs to be alocated 
		//the worst case is one item in every x axis cell and then one cell with all the remaining items 
		static constexpr size_t max_pages = calculate_number_of_pages_needed(Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size);
		static constexpr size_t max_y_axis_pages = ((Imax_y_items - 1) / Ipage_size) + 1;
		
		//the maximum number of items neede for all pages 
		static constexpr size_t max_total_entries = max_pages * Ipage_size;

		//page tracker that tracks what pages are available
		paged_memory_header<max_pages> paged_memory_tracker;

		//the number of items in the y axis for each x axis 
		std::array<y_axis_count_type, Inumber_of_x_axis_items> y_axis_count = {};


		//iterator for stepping over all real addresses in the y axis 
		struct y_address_iterator
		{
			//refference to the virtual memory map to iterate over
			y_axis_virtual_memory_map_type& memory_map;

			//refferene to the y axis count to count up to 
			y_axis_count_type& y_axis_count;

			//the current virtual address 
			y_axis_virtual_memory_map_type::virtual_address_type current_address;

		};

		const std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>& get_y_axis_memory_map() const;
		const combined_address_virtual_memory_map_type& get_all_axis_memory_map() const;

	private:
		//array of all the address mappings 
		std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items> virtual_memory_lookup;

		//make sure the per y axis memory lookup array is the same size as the all axis memeory lookup
		static_assert(sizeof(std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>) == sizeof(combined_address_virtual_memory_map_type));

	};

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_virtual_memory_map_type::real_node_address_type  
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::push_back(x_axis_count_type x_axis_index)
	{
		assert(x_axis_index < Inumber_of_x_axis_items, "indexing outside x array bounds");

		//create virtual address
		auto new_y_address = typename y_axis_virtual_memory_map_type::virtual_address_type{ y_axis_count[x_axis_index]};

		//increment the axis count
		y_axis_count[x_axis_index]++;

		//check we have not exceeded the y axis count limit
		assert(new_y_address.address < Imax_y_items, "too many items added to x axis array");

		//get the page for this
		auto virtual_page_number = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(new_y_address);

		//check if adding this item needs a new memory page
		const bool is_page_alocated = virtual_memory_lookup[x_axis_index].does_virtual_page_have_real_page(virtual_page_number);

		//allocate a new page if needed
		auto new_page = paged_memory_tracker.branchless_allocate(!is_page_alocated);

		//optionally apply the new page value and convert the address over to a "physical address"
		virtual_memory_lookup[x_axis_index].non_branching_add_page(virtual_page_number, new_page,!is_page_alocated);

		//apply the virtual page to the virtual address
		auto address_to_write_to = virtual_memory_lookup[x_axis_index].resolve_address_using_virtual_page_offset(new_y_address, virtual_page_number);

		//return the address that can be safely written to
		return address_to_write_to;
	};

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline void paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::pop_back(x_axis_count_type x_axis_index)
	{
		//get the y axis virtual address as well as decrement the number of items in this y axis array
		auto address_to_pop = typename y_axis_virtual_memory_map_type::virtual_address_type { --y_axis_count[x_axis_index] };

		//check if we need to free the underlying memory page
		bool is_only_item_in_page = y_axis_virtual_memory_map_type::is_first_item_in_real_page(address_to_pop);

		auto virtual_page_index = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(address_to_pop);

		//clear the memory page but only if it is the last item in the memory pool
		paged_memory_tracker.branchless_free(virtual_memory_lookup[x_axis_index].get_page_handle_to_return(virtual_page_index), is_only_item_in_page);
		
	};

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_virtual_memory_map_type::real_node_address_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::find_address(x_axis_count_type x_axis_index, y_axis_virtual_memory_map_type::virtual_address_type virtual_address) const
	{
		return get_y_axis_memory_map()[x_axis_index].resolve_address(virtual_address);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_virtual_memory_map_type::real_node_address_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::find_address(combined_address_virtual_memory_map_type::virtual_address_type virtual_address) const
	{
		return get_all_axis_memory_map().resolve_address(virtual_address);
	}
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline const std::array<typename paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>& 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_y_axis_memory_map() const
	{
		//return a ref to virtual mem lookup array
		return virtual_memory_lookup;
	}
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline const typename paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::combined_address_virtual_memory_map_type& 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_all_axis_memory_map() const
	{
		// TODO: insert return statement here
		return std::bit_cast<const combined_address_virtual_memory_map_type>(virtual_memory_lookup);
	}
	;
}

