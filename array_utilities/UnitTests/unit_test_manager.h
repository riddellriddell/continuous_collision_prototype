#pragma once
#include "../base_types_definition.h"
#include "../small_list.h"
#include "../free_list.h"

#include <iostream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <assert.h>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include "array_utilities/paged_2d_array.h"
#include "array_utilities/SectorPackedArray/paged_memory_header.h"
#include "array_utilities/SectorPackedArray/virtual_memory_map.h"

namespace ArrayUtilities
{
	class unit_test_manager
	{
	public:
		
		static void run_small_and_free_list_test()
		{
			//test small list 
			SmallList<int> list_test = {};

			FreeList<int> free_list_test = {};

			std::srand(std::time(nullptr)); // use current time as seed for random generator

			//add random values to list up to capacity 
			for (int i = 0; i < list_test.capacity(); i++)
			{
				int random_variable = std::rand();

				list_test.push_back(random_variable);
			}

			for (int i = 0; i < list_test.size(); ++i)
			{
				free_list_test.insert(list_test[i]);
			}
		}

		static void run_virtual_memory_header_test()
		{
			constexpr size_t page_size = 256;
			constexpr size_t real_page_count = 256;
			constexpr size_t virtual_page_count = 256;
			using mem_map_type = virtual_memory_map<page_size, virtual_page_count, real_page_count>;

			mem_map_type virtual_mem_map;

			constexpr size_t page_count = 256;
			using header_type = paged_memory_header<page_count>;

			header_type page_mem_header;

			//get a memory page
			auto page_to_add_01 = page_mem_header.allocate();

			//check page valid 
			assert(page_to_add_01.is_valid());

			//add a virtual page to the mem map for virtual addresses 0 - page size
			virtual_mem_map.non_branching_add_page(0, page_to_add_01, true);

			//create a virtual memory address
			auto virtual_mem_address_01 =  mem_map_type::virtual_address_type(0);

			//translate the virtual address to a real address
			auto real_mem_address_01 = virtual_mem_map.resolve_address(virtual_mem_address_01);

			//this should have mapped to the page we just added 
			assert(real_mem_address_01.address == 0);

			//get 2 more pages skipping one and adding the other to the memory map
			auto page_to_add_02 = page_mem_header.allocate();
			auto page_to_add_03 = page_mem_header.allocate();

			//add a virtual page to the mem map for virtual addresses  page_size - page_size * 2
			virtual_mem_map.non_branching_add_page(1, page_to_add_03, true);

			//create a virtual memory address in the second virtual page space 
			auto virtual_mem_address_02 = mem_map_type::virtual_address_type{ mem_map_type::number_of_items_per_page};

			//translate the virtual address to a real address
			auto real_mem_address_02 = virtual_mem_map.resolve_address(virtual_mem_address_02);

			//the real address should have been mapped to the third page 
			assert(real_mem_address_02.address == (mem_map_type::number_of_items_per_page * 2));

			//un map the seccond page and re map it to a different page val
			auto& handle_to_return = virtual_mem_map.get_page_handle_to_return(1);

			page_mem_header.branchless_free(handle_to_return, true);

			//check that the handle was returned properly 
			assert(handle_to_return.is_valid() == false);

			//re map in the other page handel to the virtual memory
			virtual_mem_map.non_branching_add_page(1,page_to_add_02, true);

			//re resolve the adderss
			auto real_mem_address_03 = virtual_mem_map.resolve_address(virtual_mem_address_02);

			//check that the mem map has correctly mapped the virtual address to a value on the second page
			assert(real_mem_address_03.address == mem_map_type::number_of_items_per_page);

		}


		static void run_paged_memory_header_test()
		{
			constexpr size_t page_count = 256;
			using header_type = paged_memory_header<page_count>;

			header_type page_mem_header;

			//get number of pages available
			auto free_page_count = page_mem_header.remaining_page_count();

			//take a page from the mem tracker 
			auto page = page_mem_header.allocate();

			//check that we have a valid page 
			assert(page.is_valid() == true);

			//check that the number of pages went down by one
			assert(free_page_count == (page_mem_header.remaining_page_count() + 1));

			//return the page handle to the allocator
			page_mem_header.free(page);

			//check that the page was destroyed
			assert(page.is_valid() == false);

			//check that the number of pages returned to the original value
			assert(free_page_count == page_mem_header.remaining_page_count());

			//alocate all pages and make sure we never get the same val back twice 
			std::unordered_set<header_type::page_index_type> handle_set;
			
			for (int i = 0; i < free_page_count; ++i)
			{
				auto new_handle = page_mem_header.allocate();

				//check we are getting a valid handle
				assert(new_handle.is_valid());

				auto insert_result = handle_set.insert(new_handle.get_page());

				//check if we have already recieved this handle
				assert(insert_result.second);

			}

			//check that the number of available handles is now 0
			assert(page_mem_header.remaining_page_count() == 0);

			//repeate the same process in reverse
			for (auto it = handle_set.begin(); it != handle_set.end(); ++it)
			{
				auto new_handle = header_type::page_handle_type{ *it };

				//check we are getting a valid handle
				assert(new_handle.is_valid());

				page_mem_header.free(new_handle);

				//check the handle has proprerly been returned 
				assert(!new_handle.is_valid());
			}

			//check that all the pages have been correctly returned 
			assert(free_page_count == page_mem_header.remaining_page_count());

			//clear up the set
			handle_set.clear();

			//repeate the get handle process to make sure the page trakcer still works  

			for (int i = 0; i < free_page_count; ++i)
			{
				auto new_handle = page_mem_header.allocate();

				//check we are getting a valid handle
				assert(new_handle.is_valid());

				auto insert_result = handle_set.insert(new_handle.get_page());

				//check if we have already recieved this handle
				assert(insert_result.second);

			}


		}

		static void run_paged_2d_array_test()
		{
			using header_type = paged_2d_array_header<8, std::numeric_limits<uint8_t>::max() -1, std::numeric_limits<uint8_t>::max() - 1, (std::numeric_limits<uint8_t>::max() + 1) /2>;
			
			header_type array_header;

			//add a single item into the 0 axis y array 
			auto real_address_01 = array_header.push_back(0);

			//check if item was added 
			assert(array_header.y_axis_count[0] == 1);

			//check that the real adderss makes sense
			assert(real_address_01.address == 0);

			//remove item from first array
			array_header.pop_back(0);

			//check that item was removed 
			assert(array_header.y_axis_count[0] == 0);

			//add another item in at the next x index
			auto real_address_02 = array_header.push_back(1);

			//check if item was added 
			assert(array_header.y_axis_count[1] == 1);

			//this should be going into the same location as the last one 
			assert(real_address_02.address == 0);

			//re add the first item back in at x index 0
			auto real_address_03 = array_header.push_back(0);

			//check that item was added 
			assert(array_header.y_axis_count[0] == 1);
			assert(array_header.y_axis_count[1] == 1);

			//check that the real address is in its own page
			assert(real_address_03.address == header_type::page_size);

			//add a seccond item to the second idnex 
			auto real_address_04 = array_header.push_back(1);

			//check address
			assert(real_address_04.address == 1);

			typename header_type::real_address_type real_address_05;

			//add enough to index 1 to require a new page
			for (int i = array_header.y_axis_count[1]; i <= header_type::page_size; ++i)
			{
				real_address_05 = array_header.push_back(1);
			}

			//check that the last address was put onto page 3
			assert(real_address_05.address == (header_type::page_size * 2));

			//alocate all pages and make sure we never get the same val back twice 
			std::unordered_set<header_type::real_address_type::address_value_type> handle_set;

			//add an item to all x axis entries 
			for (int i = 0; i < array_header.y_axis_count.size(); ++i)
			{
				auto real_address_0x = array_header.push_back(i);

				//make sure none of the addersses were duplicated 

				auto insert_result = handle_set.insert(real_address_0x.address);

				//check if we have already recieved this handle
				assert(insert_result.second);
			}
		}		
	};
}
