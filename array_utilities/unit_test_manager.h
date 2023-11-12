#pragma once
#include "base_types_definition.h"
#include "small_list.h"
#include "free_list.h"
#include "wide_node_linked_list.h"

#include <iostream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <assert.h>
#include <ranges>
#include <algorithm>
#include <numeric>

namespace ArrayUtilities
{
	class unit_test_manager
	{
	public:
		
		static void run_test()
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

		static void run_wide_node_linked_list_test()
		{
			//create wide node linked list
			auto widenode_list = wide_node_linked_list<std::numeric_limits<uint8>::max(), uint16, 255 * 255, 8, uint8>();

			uint32 item_to_add_01 = 69; //nice
			uint32 index_to_add_01 = 0;

			widenode_list.add(index_to_add_01, static_cast<uint8>(item_to_add_01));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_01);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_01); });

				assert(found_added_item, "the value of x was not correctly retrieved");
			}

			//remove the value inserted 
			widenode_list.remove(index_to_add_01, static_cast<uint8>(item_to_add_01));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_01);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_01); });

				assert(!found_added_item, "the value of x should have been removed");
			}


			uint32 item_to_add_02 = 96; //spicy
			uint32 index_to_add_02 = 1;

			widenode_list.add(index_to_add_02, static_cast<uint8>(item_to_add_02));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_02);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_02); });

				assert(found_added_item, "the value of x was not correctly retrieved");
			}

			//remove the value inserted 
			widenode_list.remove(index_to_add_02, static_cast<uint8>(item_to_add_02));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_02);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_02); });

				assert(!found_added_item, "the value of x should have been removed");
			}

			//add 9 items forcing the creation of 2 nodex 
			constexpr uint32 items_to_add_count_03 = 9;
			std::array<uint32, items_to_add_count_03> items_to_add_03;

			uint32 index_to_add_03 = 3;

			//crate array of all the items to add
			std::iota(items_to_add_03.begin(), items_to_add_03.end(), 0);

			//add all the items to the array 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x) {widenode_list.add(index_to_add_03,x); });

			//check to see if all the items were added 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x)
				{
					bool found_added_item = false;

					auto begin = widenode_list.get_root_node_start(index_to_add_03);
					auto end = widenode_list.end();

					std::for_each(begin, end, [&](auto& item) {found_added_item |= (item == x); });

					assert(found_added_item, "the value of x should have been found");
				});

			uint32 item_to_remove_03 = 4;

			//remove a single item in the middle 
			widenode_list.remove(index_to_add_03, item_to_remove_03);

			//check to see 4 is removed and all the other items remain 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x)
				{
					bool found_added_item = false;

					auto begin = widenode_list.get_root_node_start(index_to_add_03);
					auto end = widenode_list.end();

					std::for_each(begin, end, [&](auto& item) {found_added_item |= (item == x); });

					if (x == item_to_remove_03)
					{
						assert(!found_added_item, "the value of item_to_remove_03 should have been removed");
					}
					else
					{
						assert(found_added_item, "the value of x should have been found");
					}
				});

			//remove all the remaining items 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x)
				{
					auto begin = widenode_list.get_root_node_start(index_to_add_03);
					auto end = widenode_list.end();

					if (x != item_to_remove_03)
					{
						widenode_list.remove(index_to_add_03, x);
					}
				});

			//check all the items have been removed 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x)
				{
					bool found_added_item = false;

					auto begin = widenode_list.get_root_node_start(index_to_add_03);
					auto end = widenode_list.end();

					std::for_each(begin, end, [&](auto& item) {found_added_item |= (item == x); });

					assert(!found_added_item, "the value of x should have been removed");
				});
		}
	};
}
