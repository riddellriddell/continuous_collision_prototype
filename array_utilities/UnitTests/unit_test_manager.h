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
#include <memory>
#include "array_utilities/paged_2d_array.h"
#include "array_utilities/SectorPackedArray/paged_memory_header.h"
#include "array_utilities/SectorPackedArray/virtual_memory_map.h"
#include "array_utilities/paged_wide_node_linked_list.h"

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

			typename header_type::real_node_address_type real_address_05;

			//add enough to index 1 to require a new page
			for (int i = array_header.y_axis_count[1]; i <= header_type::page_size; ++i)
			{
				real_address_05 = array_header.push_back(1);
			}

			//check that the last address was put onto page 3
			assert(real_address_05.address == (header_type::page_size * 2));

			//alocate all pages and make sure we never get the same val back twice 
			std::unordered_set<header_type::real_node_address_type::address_value_type> handle_set;

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
	
		static void run_paged_wide_node_linked_list_unit_test()
		{
				// Tdatatype,
			static constexpr size_t Iroot_entries_count = 16 * 16 * 16 * 16;
			static constexpr size_t Inode_width = 8;
			static constexpr size_t Imax_entries_per_root = (std::numeric_limits<uint16_t>::max()) ;
			static constexpr size_t Imax_entries_per_root_group = (std::numeric_limits<uint16_t>::max() );
			static constexpr size_t Imax_global_entries = (std::numeric_limits<uint16_t>::max());
			static constexpr size_t Ipage_size = 16*16;
			static constexpr size_t Iroot_node_group_size = 16 * 16;

			using paged_link_list_type = paged_wide_node_linked_list<int, Iroot_entries_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>;

			//deubg total pages needed 
			auto pages_needed = calculate_memory_pages_for_wide_linked_list(Iroot_entries_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size);

			std::unique_ptr<paged_link_list_type> paged_hirachical_list = std::make_unique<paged_link_list_type>();

			assert(paged_hirachical_list->get_total_node_count() >= (std::numeric_limits<uint16_t>::max()/ Inode_width));

			//try and get a node for a page
			auto node_index = paged_hirachical_list->get_free_node(0);

			//return node
			paged_hirachical_list->return_node(0,node_index);

			//try and get and return the node againg to make sure it was reurned correctly 
			node_index = paged_hirachical_list->get_free_node(0);

			paged_hirachical_list->return_node(0, node_index);

			std::vector< paged_link_list_type::node_link_type> nodes_allocated;

			//get more than a full page worth of nodes from the first root group
			for (uint32_t i = 0; i < Ipage_size -1; ++i)
			{
				nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));
			}

			//allocate an extra node that will fill the page 
			nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));

			//get the extra node that will force the allocation of an extra page
			nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));

			//un alocate the last node returning the extra page that was allocated
			paged_hirachical_list->return_node(0, nodes_allocated.back());
			nodes_allocated.pop_back();

			//re allocatee the last node to make sure it was returned correctly
			nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));

			//un alocate the last node again
			paged_hirachical_list->return_node(0, nodes_allocated.back());
			nodes_allocated.pop_back();

			//un allocate the next node forcing the second page back into the free node list
			paged_hirachical_list->return_node(0, nodes_allocated.back());
			nodes_allocated.pop_back();

			//re alocate again
			nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));

			//un alocate again
			paged_hirachical_list->return_node(0, nodes_allocated.back());
			nodes_allocated.pop_back();

			//remove all the remaining nodex
			for (uint32_t i = nodes_allocated.size() - 1; i < nodes_allocated.size(); --i)
			{
				paged_hirachical_list->return_node(0, nodes_allocated[i]);
				nodes_allocated.pop_back();
			}

			nodes_allocated.clear();

			//re fill from the front but un fill in the same order 
			for (uint32_t i = 0; i < Ipage_size; ++i)
			{
				nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));
			}
			

			//un fill in same order 
			for (uint32_t i = 0; i < Ipage_size; ++i)
			{
				paged_hirachical_list->return_node(0, nodes_allocated[i]);
			}

			nodes_allocated.clear();

			//add nodes in 2 neighbouring sectors
			auto root_group_1_node = paged_hirachical_list->get_free_node(0);
			auto root_group_2_node = paged_hirachical_list->get_free_node(Iroot_node_group_size);

			//return the nodes
			paged_hirachical_list->return_node(0, root_group_1_node);
			paged_hirachical_list->return_node(Iroot_node_group_size, root_group_2_node);

			//add 2 pages worth of nodes then return all the first page worth of nodex 
			for (uint32_t i = 0; i < Ipage_size; ++i)
			{
				nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));
			}

			nodes_allocated.push_back(paged_hirachical_list->get_free_node(0));

			//return all the nodes in the first page, this should return it back to the node pool
			for (uint32_t i = 0; i < Ipage_size; ++i)
			{
				paged_hirachical_list->return_node(0, nodes_allocated[i]);
			}

			//paged_hirachical_list->empty_page_count();
		}

	};
}
