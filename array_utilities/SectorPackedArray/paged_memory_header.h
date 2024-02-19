#pragma once
#include <array>
#include <limits>
#include <numeric>
#include <algorithm>

#include "misc_utilities/int_type_selection.h"
#include "array_utilities/SectorPackedArray/shared_virtual_memory_types.h"

namespace ArrayUtilities
{
	template<size_t Inumber_of_pages>
	struct paged_memory_header
	{
		//sanity check that this page header will have a valid number of pages
		static_assert(Inumber_of_pages > 0);

		//the number type needed to index all the pages
		using page_index_type = MiscUtilities::uint_s<Inumber_of_pages + 1>::int_type_t;
		using page_handle_type = page_handle<page_index_type>;

		paged_memory_header();

		void reset();

		//get a free page handle
		page_handle_type allocate();

		//similar to alocate but usses a bool to apply the alocation in a non branching way
		page_handle_type branchless_allocate(bool do_allocation);

		//return an allocated handle 
		void free(page_handle_type& default_handle_type);

		//return a handle but in a non branching way
		void branchless_free(page_handle_type& default_handle_type, bool do_free);

		//how many pages are there left 
		constexpr page_index_type remaining_page_count() const;

	private:

		//list of all the memory pages that are not in use 
		page_index_type free_page_count;
		std::array<page_index_type, Inumber_of_pages> free_pages;

	};

	template<size_t Inumber_of_pages>
	inline paged_memory_header<Inumber_of_pages>::paged_memory_header()
	{
		reset();
	}

	template<size_t Inumber_of_pages>
	void paged_memory_header<Inumber_of_pages>::reset()
	{
		free_page_count = free_pages.size();
		std::iota(free_pages.rbegin(), free_pages.rend(), 0);
	}

	template<size_t Inumber_of_pages>
	inline  paged_memory_header<Inumber_of_pages>::page_handle_type paged_memory_header<Inumber_of_pages>::allocate()
	{
		//check that there are still pages to create 
		assert(free_page_count != 0);

		return page_handle(free_pages[--free_page_count]);
	}

	template<size_t Inumber_of_pages>
	inline paged_memory_header<Inumber_of_pages>::page_handle_type paged_memory_header<Inumber_of_pages>::branchless_allocate(bool do_allocation)
	{
		//check that there are still pages to create 
		assert(free_page_count != 0 || !do_allocation);

		//optionally decrement the number of free pages 
		free_page_count -= do_allocation;

		//returns the page if do_allocation is true otherwise it reutns an invalid address 
		return page_handle(free_pages[free_page_count]);
	}

	template<size_t Inumber_of_pages>
	inline void paged_memory_header<Inumber_of_pages>::free(page_handle_type& default_handle_type)
	{
		//check that the page is valid 
		assert(default_handle_type.is_valid());

		//check we have not retuned to many pages 
		assert(free_page_count <= Inumber_of_pages);

		//check that it has not already been returned
		assert(std::find(free_pages.begin(), free_pages.begin() + free_page_count, default_handle_type.get_page()) == (free_pages.begin() + free_page_count));

		free_pages[free_page_count++] = default_handle_type.get_page();

		//make the handle invalid 
		default_handle_type.destroy();
	}
	
	template<size_t Inumber_of_pages>
	inline void paged_memory_header<Inumber_of_pages>::branchless_free(page_handle_type& default_handle_type, bool do_free)
	{
		//check that the page is valid 
		assert(default_handle_type.is_valid());

		//check we have not retuned to many pages 
		assert(free_page_count <= Inumber_of_pages);

		//check that it has not already been returned
		assert(std::find(free_pages.begin(), free_pages.begin() + free_page_count, default_handle_type.get_page()) == (free_pages.begin() + free_page_count));

		free_pages[free_page_count] = default_handle_type.get_page();

		free_page_count += do_free;

		//make the handle invalid 
		default_handle_type.branchless_destroy(do_free);
	}
	template<size_t Inumber_of_pages>
	inline constexpr paged_memory_header<Inumber_of_pages>::page_index_type paged_memory_header<Inumber_of_pages>::remaining_page_count() const
	{
		return free_page_count;
	}
}