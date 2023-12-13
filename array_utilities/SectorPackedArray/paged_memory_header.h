#pragma once
#include <array>
#include <limits>
#include <numeric>
#include <algorithm>

#include "misc_utilities/int_type_selection.h"


template<typename Tnode_index_type>
struct page_handle
{
	static constexpr invalid_page_value = std::numeric_limits<Tnode_index_type>::max();


private:

	Tnode_index_type page_number;

	bool is_valid()
	{
		return page_number != invalid_page_value;
	}

	void destroy()
	{
		check(is_valid());
		page_number = invalid_page_value;
	}

public:

	page_handle(Tnode_index_type _page_number) :page_number(_page_number) {};


	//accessor to get the page 
	Tnode_index_type get_page()
	{
		check(is_valid());

		return page_number;
	}
};


template<size_t Inumber_of_pages>
struct paged_memory_header
{
	//the number type needed to index all the pages
	using page_index_type = MiscUtilities::uint_s<Inumber_of_pages + 1>::int_type_t;
	using page_handle_type = page_handle<page_index_type>;

	paged_memory_header();

	void reset();
	
	//get a free page handle
	page_handle_type allocate();

	//return an allocated handle 
	void free(page_handle_type& handle);

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
	std::iota(free_pages.begin(), free_pages.end(), 0);
}

template<size_t Inumber_of_pages>
inline  paged_memory_header<Inumber_of_pages>::page_handle_type paged_memory_header<Inumber_of_pages>::allocate()
{
	//check that there are still pages to create 
	check(free_page_count != 0);

	return page_handle(free_pages[--free_page_count]);
}

template<size_t Inumber_of_pages>
inline void paged_memory_header<Inumber_of_pages>::free(page_handle_type& handle)
{
	//check that the page is valid 
	check(handle.is_valid());

	//check we have not retuned to many pages 
	check(free_page_count <= Inumber_of_pages);

	//check that it has not already been returned
	check(std::find(free_pages.begin, free_pages.begin + free_page_count, handle.get_page()));

	free_pages[free_page_count++] = handle.get_page();

	//make the handle invalid 
	handle.destroy();
}
