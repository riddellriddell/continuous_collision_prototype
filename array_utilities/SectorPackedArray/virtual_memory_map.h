#pragma once 
#include <array>
#include <bit>
#include "misc_utilities/int_type_selection.h"
#include "array_utilities/SectorPackedArray/shared_virtual_memory_types.h"

namespace ArrayUtilities
{

	template<typename Taddress_type>
	struct virtual_address
	{
		Taddress_type address;

		//prefix 
		virtual_address<Taddress_type>& operator++()
		{
			++address;

			return *this;
		}

		virtual_address<Taddress_type>& operator--()
		{
			--address;

			return *this;
		}

		//postfix plus
		virtual_address<Taddress_type>& operator++(Taddress_type)
		{
			Taddress_type temp{ address };

			++address;

			return temp;
		}

		virtual_address<Taddress_type>& operator--(Taddress_type)
		{
			Taddress_type temp{ address };

			--address;

			return temp;
		}
	};

	template<typename Taddress_type,size_t Ipage_bits>
	struct real_address
	{
	private:
		static constexpr Taddress_type  page_bits_mask = (1 << Ipage_bits) - 1;
	public:


		Taddress_type address;

		constexpr bool is_valid() const;
	};

	//this structure is for mapping a contigious address space across discontigious memory pages 
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	struct virtual_memory_map
	{
	private:

		//the address size we will use to track pages 
		//one is added so we can safely assume max value == invalid address 
		using page_address_value_type = MiscUtilities::uint_s<total_number_of_pages + 1>::int_type_t;

		//the virtual address type 
		using virtual_address_value_type = MiscUtilities::uint_s<max_number_of_pages_in_virtual_address_space * page_size >::int_type_t;

		//the non virtual address type needed to actually access the memory 
		using real_address_value_type = MiscUtilities::uint_s<total_number_of_pages * page_size >::int_type_t;

		//the bits that make up the local address
		static constexpr uint32_t local_address_bits = std::bit_width(page_size - 1);
		static constexpr virtual_address_value_type local_address_mask = (1 << local_address_bits) - 1;
		static constexpr virtual_address_value_type page_address_mask = ~local_address_mask;
		static constexpr page_address_value_type invalid_page_address = std::numeric_limits<page_address_value_type>::max();


	public:

		using virtual_address_type = virtual_address<virtual_address_value_type>;
		using real_address_type = real_address<real_address_value_type, local_address_bits>;
		using page_handle_type = page_handle<real_address_value_type>;

	private:

		

		//pages need to be a power of 2 to work efficiently
		static_assert((1 << local_address_bits) == page_size);

		//the all the pages assigned to this address space 
		std::array<page_handle_type, max_number_of_pages_in_virtual_address_space> pages_in_space;

		real_address_value_type convert_to_real_using_page_internal(virtual_address_value_type virtual_address, auto page_number) const;

	public:


		

		real_address_type resolve_address(virtual_address_type address) const;

		real_address_type resolve_address_using_virtual_page_offset(virtual_address_type address, auto virtual_page_number);

		//check that the address given has an alocated page file 
		bool does_address_have_page(virtual_address_type address) const;

		//extracts virtual page number from virtual address
		static virtual_address_value_type extract_page_number_from_virtual_address(virtual_address_type address);
		
		//check if address is first item in a memory page
		static bool is_first_item_in_real_page(virtual_address_type address);

		//check if a virtual page has a real page value
		bool does_virtual_page_have_real_page(auto virtual_page_number) const;

		//sets the page for the given virtual address but only if the bool is set to true 
		//this is done in a non branching optimal way
		void non_branching_add_page(auto virtual_page_number, page_handle_type page_to_add, bool apply_page);

		//gets a refference to a physical page that this virtual page map is using
		//this is to pass to the page tracker to return to the page pool
		//need to find a better way to structue this 
		page_handle_type& get_page_handle_to_return(auto virtual_page_number);

	};
	
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::real_address_value_type virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::convert_to_real_using_page_internal(virtual_address_value_type virtual_address, auto page_number) const
	{
		assert((virtual_address >> local_address_bits) == page_number, "the virtual address has a different virtual page number to the virtual page we are using, the virtual page should match that of the virtual address, the only reason it is not calculated is for optimization reasons");

		auto page_val = pages_in_space[page_number];

		real_address_value_type out_address = virtual_address & local_address_mask;

		out_address |= page_val.get_page() << local_address_bits;

		return out_address;
	}

	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::real_address_type virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::resolve_address(virtual_address_type address) const
	{
		auto page_number = extract_page_number_from_virtual_address(address);

		return real_address_type(convert_to_real_using_page_internal(address.address, page_number));
	}

	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::real_address_type virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::resolve_address_using_virtual_page_offset(virtual_address_type address, auto virtual_page_number)
	{
		assert(extract_page_number_from_virtual_address(address) == virtual_page_number, "this function is an optimization over just passing in the address and calculating the page number on the fly, the passed in page number should match that of the virtual address");

		return real_address_type(convert_to_real_using_page_internal(address.address, virtual_page_number));
	}
	
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline bool virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::does_address_have_page(virtual_address_type address) const
	{
		//remove the lower bits we dont care about leaving only the virtual page number
		auto virtual_page_number = address.address >> local_address_bits;

		return does_virtual_page_have_real_page(virtual_page_number);

		
	}
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::virtual_address_value_type virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::extract_page_number_from_virtual_address(virtual_address_type address)
	{
		return address.address >> local_address_bits;
	}
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline bool virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::is_first_item_in_real_page(virtual_address_type address)
	{
		//get the page offset component of the virtual address
		auto page_offset = address.address & local_address_mask;

		//first item in a page should have an offset of 0, doing a boolean invert should return the correct val
		return !page_offset;
	}
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline bool virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::does_virtual_page_have_real_page(auto virtual_page_number) const
	{
		//get the page address for virtual page number
		auto page_number = pages_in_space[virtual_page_number];

		//check if this page number is "invalid"
		bool page_alocated = page_number.get_page() != invalid_page_address;

		return page_alocated;
	}

	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline void virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::non_branching_add_page(auto virtual_page_number, page_handle_type page_to_add, bool apply_page)
	{
		//this assumes the existing page is a invalid value which == max value
		//by adding 1 max value wraps arround to 0,
		//by adding the rest of the page number it should set the virtual page to the new page value
		//if apply page == 0 then we are just adding 0 and should make no change
		pages_in_space[virtual_page_number].branchless_set_handle(page_to_add, apply_page);
	}
	template<size_t page_size, size_t max_number_of_pages_in_virtual_address_space, size_t total_number_of_pages>
	inline  virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::page_handle_type& virtual_memory_map<page_size, max_number_of_pages_in_virtual_address_space, total_number_of_pages>::get_page_handle_to_return(auto virtual_page_number)
	{
		page_handle_type& page = pages_in_space[virtual_page_number];

		//check that the page handle is valid and has not already been destroyed
		assert(page.is_valid(), "should not be handing out reffs to non allocated pages, this function is intended for getting pages to later destroy / return to the page pool");

		return page;
	}
}