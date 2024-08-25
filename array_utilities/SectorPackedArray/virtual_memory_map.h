#pragma once 
#include <array>
#include <bit>
#include "misc_utilities/int_type_selection.h"
#include "array_utilities/SectorPackedArray/shared_virtual_memory_types.h"

namespace ArrayUtilities
{

	//the real address needs to be external to the structure because 2 different virtual memory maps can map to the same real address 
	//this is due to type punning allowing access to arrays through a combined or x,y address 
	template<typename real_address_value_type, real_address_value_type local_address_bits>
	struct real_address
	{
	private:
		static constexpr real_address_value_type  page_bits_mask = (1 << local_address_bits) - 1;
	public:

		// Increment operator ++
		real_address& operator++() 
		{
			++address;
			return *this;
		}

		// Post-increment operator i++
		real_address operator++(int) 
		{
			real_address temp(*this);
			++(*this);
			return temp;
		}

		// Decrement operator --
		real_address& operator--() 
		{
			--address;
			return *this;
		}

		// Post-increment operator i++
		real_address operator--(int) 
		{
			real_address temp(*this);
			--(*this);
			return temp;
		}

		real_address operator+(real_address_value_type offset)
		{
			return real_address(address + offset);
		}

		real_address operator-(real_address_value_type offset)
		{
			return real_address(address - offset);
		}

		//setup comparitor
		auto operator<=>(const real_address&) const = default;

		using address_value_type = real_address_value_type;

		real_address_value_type address;

		real_address_value_type get_sub_page_offset()
		{
			return address & page_bits_mask;
		}
	};


	//this structure is for mapping a contigious address space across discontigious memory pages 
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	struct virtual_memory_map
	{

	private:

		//the address size we will use to track pages 
		//one is added so we can safely assume max value == invalid address 
		using page_address_value_type = MiscUtilities::uint_s<Itotal_number_of_pages + 1>::int_type_t;

		//the virtual address number type needed to store all possible addess values in the available address space 
		using virtual_address_value_type = MiscUtilities::uint_s<Imax_number_of_pages_in_virtual_address_space* Ipage_size >::int_type_t;

		//the non virtual address type needed to actually access the memory 
		using real_address_value_type = MiscUtilities::uint_s<Itotal_number_of_pages * Ipage_size >::int_type_t;
 
	public:

		static constexpr virtual_address_value_type number_of_items_per_page = Ipage_size;

		//the bits that make up the local address
		static constexpr uint32_t local_address_bits = std::bit_width(Ipage_size - 1);

	public:

		struct virtual_address
		{
			virtual_address_value_type address;

			//prefix 
			virtual_address& operator++()
			{
				++address;

				return *this;
			}

			virtual_address& operator--()
			{
				--address;

				return *this;
			}

			//postfix plus
			virtual_address& operator++(int)
			{
				virtual_address temp{ address };

				++address;

				return temp;
			}

			virtual_address& operator--(int)
			{
				virtual_address_value_type temp{ address };

				--address;

				return temp;
			}

			auto operator<=>(const virtual_address& other) const = default;
		};


		using virtual_address_type = virtual_address;
		using real_node_address_type = real_address<real_address_value_type, local_address_bits>;
		
		//the address type to use when doing a combined space lookup into a virtual page
		using combined_virtual_page_addres_type = MiscUtilities::uint_s<Imax_number_of_pages_in_virtual_address_space + 1>::int_type_t;

		using page_handle_type = page_handle<page_address_value_type>;

	private:
		//other key bit values
		static constexpr virtual_address_value_type local_address_mask = (1 << local_address_bits) - 1;
		static constexpr virtual_address_value_type page_address_mask = ~local_address_mask;

		//pages need to be a power of 2 to work efficiently
		static_assert((1 << local_address_bits) == Ipage_size);

		//the all the pages assigned to this address space 
		std::array<page_handle_type, Imax_number_of_pages_in_virtual_address_space> pages_in_space;

		real_address_value_type convert_to_real_using_page_internal(virtual_address_value_type virtual_address, auto page_number) const;

	public:

		//what is the max virtual address possible 
		static constexpr virtual_address_value_type max_virtual_address = Imax_number_of_pages_in_virtual_address_space * Ipage_size;


		//constructor. all page handels need to be initialized as invalid
		constexpr virtual_memory_map();

		//get the index in a page of a virtual address
		static virtual_address_value_type get_sub_page_index(virtual_address_type virtual_address);

		//get the index in a page of a real address
		static real_address_value_type get_sub_page_index(real_node_address_type real_address);

		real_node_address_type resolve_address(virtual_address_type address) const;

		real_node_address_type resolve_address_using_virtual_page_offset(virtual_address_type address, auto virtual_page_number) const;

		real_node_address_type resolve_address_using_page_and_offset(virtual_address_value_type sub_page_offset, auto page_number) const;

		page_handle_type resolve_virtual_address_to_page_handle( virtual_address_type address) const;

		//turn a page number to the real address at the start of the page
		real_node_address_type resolve_page_number_to_real_address(auto page_number) const;

		//check that the address given has an alocated page file 
		bool does_address_have_page(virtual_address_type address) const;

		//extracts virtual page number from virtual address
		static constexpr virtual_address_value_type extract_page_number_from_virtual_address(virtual_address_type address);

		//extract page handle from real address 
		static constexpr page_handle_type extract_page_handle_from_real_address(real_node_address_type address);
		
		//check if address is first item in a memory page
		static bool is_first_item_in_real_page(virtual_address_type address);

		static bool is_first_item_in_real_page(real_node_address_type address);

		//check if a virtual page has a real page value
		bool does_virtual_page_have_real_page(auto virtual_page_number) const;

		//assigns a page to a given virtual address range 
		void add_page(auto virtual_page_number, page_handle_type page_to_add);

		//sets the page for the given virtual address but only if the bool is set to true 
		//this is done in a non branching optimal way
		void non_branching_add_page(auto virtual_page_number, page_handle_type page_to_add, bool apply_page);

		//gets a refference to a physical page that this virtual page map is using
		//this is to pass to the page tracker to return to the page pool
		//need to find a better way to structue this 
		page_handle_type& get_page_handle_to_return(auto virtual_page_number);

	};
	
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_address_value_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::convert_to_real_using_page_internal(virtual_address_value_type virtual_address, auto page_number) const
	{
		//, "the virtual address has a different virtual page number to the virtual page we are using, the virtual page should match that of the virtual address, the only reason it is not calculated is for optimization reasons"
		assert((virtual_address >> local_address_bits) == page_number);

		auto page_val = pages_in_space[page_number];

		real_address_value_type out_address = real_address_value_type(get_sub_page_index(virtual_address_type(virtual_address)));

		out_address |= page_val.get_page() << local_address_bits;

		return out_address;
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline constexpr virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::virtual_memory_map()
	{
		//need to loop over all the elements in the page handle array and init them
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::virtual_address_value_type 
		virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::get_sub_page_index(virtual_address_type virtual_address)
	{
		return virtual_address_value_type(virtual_address.address & local_address_mask);
	}


	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_address_value_type
		virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::get_sub_page_index(real_node_address_type real_address)
	{
		return real_address_value_type(real_address.address & local_address_mask);
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_node_address_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::resolve_address(virtual_address_type address) const
	{
		auto page_number = extract_page_number_from_virtual_address(address);

		return real_node_address_type(convert_to_real_using_page_internal(address.address, page_number));
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_node_address_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::resolve_address_using_virtual_page_offset(virtual_address_type address, auto virtual_page_number) const
	{
		//, "this function is an optimization over just passing in the address and calculating the page number on the fly, the passed in page number should match that of the virtual address"
		assert(extract_page_number_from_virtual_address(address) == virtual_page_number);

		return real_node_address_type(convert_to_real_using_page_internal(address.address, virtual_page_number));
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_node_address_type 
		virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::resolve_address_using_page_and_offset(virtual_address_value_type sub_page_offset, auto page_number) const
	{
		//check that the page we are accessing is valid 
		assert(sub_page_offset < number_of_items_per_page);

		return resolve_page_number_to_real_address(page_number) | sub_page_offset;
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::page_handle_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::resolve_virtual_address_to_page_handle(virtual_address_type address) const
	{
		//remove the lower bits we dont care about leaving only the virtual page number
		auto virtual_page_number = address.address >> local_address_bits;

		return pages_in_space[virtual_page_number];
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::real_node_address_type
		virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::resolve_page_number_to_real_address(auto page_number) const
	{
		//make sure page is valid 
		assert(does_address_have_page( virtual_address_type(page_number << local_address_bits)));

		//convert page to real address
		return real_node_address_type( pages_in_space[page_number].get_page() << local_address_bits);
	}

	
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline bool virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::does_address_have_page(virtual_address_type address) const
	{
		//remove the lower bits we dont care about leaving only the virtual page number
		auto virtual_page_number = address.address >> local_address_bits;

		return does_virtual_page_have_real_page(virtual_page_number);

		
	}
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline constexpr virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::virtual_address_value_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::extract_page_number_from_virtual_address(virtual_address_type address)
	{
		return address.address >> local_address_bits;
	}
	
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline constexpr virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::page_handle_type virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::extract_page_handle_from_real_address(real_node_address_type address)
	{
		auto page_handle_val = address >> local_address_bits;

		return page_handle_type(page_handle_val);
	}
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline bool virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::is_first_item_in_real_page(virtual_address_type address)
	{
		//get the page offset component of the virtual address
		auto page_offset = get_sub_page_index(address);

		//first item in a page should have an offset of 0, doing a boolean invert should return the correct val
		return !page_offset;
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline bool virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::is_first_item_in_real_page(real_node_address_type address)
	{
		//get the page offset component of the virtual address
		auto page_offset = get_sub_page_index(address);

		//first item in a page should have an offset of 0, doing a boolean invert should return the correct val
		return !page_offset;
	}
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline bool virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::does_virtual_page_have_real_page(auto virtual_page_number) const
	{
		//get the page address for virtual page number
		auto page_number = pages_in_space[virtual_page_number];

		//check if this page number is "invalid"
		bool page_alocated = page_number.is_valid();

		return page_alocated;
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline void virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::add_page(auto virtual_page_number, page_handle_type page_to_add)
	{
		//check we are assigning a vilid page
		assert(page_to_add.is_valid());

		pages_in_space[virtual_page_number].set_handle(page_to_add);
	}

	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline void virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::non_branching_add_page(auto virtual_page_number, page_handle_type page_to_add, bool apply_page)
	{
		//, "should not be applying pages that are not correctly alocated"
		assert(page_to_add.is_valid() || (apply_page == false));

		//this assumes the existing page is a invalid value which == max value
		//by adding 1 max value wraps arround to 0,
		//by adding the rest of the page number it should set the virtual page to the new page value
		//if apply page == 0 then we are just adding 0 and should make no change
		pages_in_space[virtual_page_number].branchless_set_handle(page_to_add, apply_page);
	}
	template<size_t Ipage_size, size_t Imax_number_of_pages_in_virtual_address_space, size_t Itotal_number_of_pages>
	inline  virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::page_handle_type& virtual_memory_map<Ipage_size, Imax_number_of_pages_in_virtual_address_space, Itotal_number_of_pages>::get_page_handle_to_return(auto virtual_page_number)
	{
		page_handle_type& page = pages_in_space[virtual_page_number];

		//check that the page handle is valid and has not already been destroyed
		//, "should not be handing out reffs to non allocated pages, this function is intended for getting pages to later destroy / return to the page pool"
		assert(page.is_valid());

		return page;
	}
}