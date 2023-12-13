#pragma once 
#include <array>
#include <bit>
#include "misc_utilities/int_type_selection.h"

namespace ArrayUtilities
{
	//this structure is for mapping a contigious address space across discontigious memory pages 
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	struct virtual_memory_map
	{
		//the address size we will use to track pages 
		using page_address_type = MiscUtilities::uint_s<total_number_of_pages>::int_type_t;

		//the virtual address type 
		using virtual_address_type = MiscUtilities::uint_s<max_number_of_pages_in_virtual_address_space * page_size >::int_type_t;

		//the non virtual address type needed to actually access the memory 
		using physical_address_type = MiscUtilities::uint_s<total_number_of_pages * page_size >::int_type_t;

		//the bits that make up the local address
		static constexpr uint32_t local_address_bits = std::bit_width(page_size - 1);

		static constexpr virtual_address_type local_address_mask = (1 << local_address_bits) - 1;
		static constexpr virtual_address_type page_address_mask = ~local_address_mask;

		//pages need to be a power of 2 to work efficiently
		static_assert((1 << local_address_bits) == page_size);

		//the all the pages assigned to this address space 
		std::array<page_address_type, max_number_of_pages_in_virtual_address_space> pages_in_space;

		physical_address_type convert_to_physical(virtual_address_type virtual_address);

	};
	
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::physical_address_type virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::convert_to_physical(virtual_address_type virtual_address)
	{
		auto page_index = virtual_address >> local_address_bits;

		auto page_val = pages_in_space[page_index];

		physical_address_type out_address = virtual_address & local_address_mask;

		out_address |= page_val << local_address_bits;

		return out_address;
	}
}