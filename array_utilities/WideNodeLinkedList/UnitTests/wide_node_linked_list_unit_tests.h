#pragma once
#include <limits>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <ranges>
#include "../wide_node_linked_list.h"
#include "../../base_types_definition.h"

namespace ArrayUtilities
{
	class wide_node_linked_lists_unit_test
	{
	public:
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

				//, "the value of x was not correctly retrieved"
				assert(found_added_item);
			}

			//remove the value inserted 
			widenode_list.remove(index_to_add_01, static_cast<uint8>(item_to_add_01));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_01);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_01); });

				//, "the value of x should have been removed"
				assert(!found_added_item);
			}


			uint32 item_to_add_02 = 96; //spicy
			uint32 index_to_add_02 = 1;

			widenode_list.add(index_to_add_02, static_cast<uint8>(item_to_add_02));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_02);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_02); });

				//, "the value of x was not correctly retrieved"
				assert(found_added_item);
			}

			//remove the value inserted 
			widenode_list.remove(index_to_add_02, static_cast<uint8>(item_to_add_02));

			{
				bool found_added_item = false;

				auto begin = widenode_list.get_root_node_start(index_to_add_02);
				auto end = widenode_list.end();

				std::for_each(begin, end, [&](auto& x) {found_added_item = (x == item_to_add_02); });

				//, "the value of x should have been removed"
				assert(!found_added_item);
			}

			//add 9 items forcing the creation of 2 nodex 
			constexpr uint32 items_to_add_count_03 = 9;
			std::array<uint32, items_to_add_count_03> items_to_add_03;

			uint32 index_to_add_03 = 3;

			//crate array of all the items to add
			std::iota(items_to_add_03.begin(), items_to_add_03.end(), 0);

			//add all the items to the array 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x) {widenode_list.add(index_to_add_03, x); });

			//check to see if all the items were added 
			std::for_each(items_to_add_03.begin(), items_to_add_03.end(), [&](auto& x)
				{
					bool found_added_item = false;

					auto begin = widenode_list.get_root_node_start(index_to_add_03);
					auto end = widenode_list.end();

					std::for_each(begin, end, [&](auto& item) {found_added_item |= (item == x); });

					//, "the value of x should have been found"
					assert(found_added_item);
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
						//, "the value of item_to_remove_03 should have been removed"
						assert(!found_added_item);
					}
					else
					{
						//, "the value of x should have been found"
						assert(found_added_item);
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

					//, "the value of x should have been removed"
					assert(!found_added_item);
				});

			//add items to several different nodes
			//add 9 items forcing the creation of 2 nodex 
			constexpr uint32 index_to_add_count_04 = 255;
			std::array<uint32, index_to_add_count_04> index_to_add_04;

			constexpr uint32 item_to_add_count_04 = 255;
			std::array<uint32, item_to_add_count_04> items_to_add_04;

			//crate array of all the items to add
			std::iota(index_to_add_04.begin(), index_to_add_04.end(), 0);
			std::iota(items_to_add_04.begin(), items_to_add_04.end(), 0);

			//add all the items 
			std::for_each(index_to_add_04.begin(), index_to_add_04.end(), [&](auto& index)
				{

					//add all the items to the array 
					std::for_each(items_to_add_04.begin(), items_to_add_04.end(), [&](auto& item)
						{
							widenode_list.add(index, item);
						});
				});

			//check all the items exist 
			std::for_each(index_to_add_04.begin(), index_to_add_04.end(), [&](auto& index)
				{

					//add all the items to the array 
					std::for_each(items_to_add_04.begin(), items_to_add_04.end(), [&](auto& item)
						{
							bool found_added_item = false;

							auto begin = widenode_list.get_root_node_start(index);
							auto end = widenode_list.end();

							std::for_each(begin, end, [&](auto& x) {found_added_item |= (x == item); });

							//, "the value of x should exist"
							assert(found_added_item);
						});
				});

			//add all the items 
			std::for_each(index_to_add_04.begin(), index_to_add_04.end(), [&](auto& index)
				{

					//add all the items to the array 
					std::for_each(items_to_add_04.begin(), items_to_add_04.end(), [&](auto& item)
						{
							widenode_list.remove(index, item);
						});
				});

			//check all the items were removed
			std::for_each(index_to_add_04.begin(), index_to_add_04.end(), [&](auto& index)
				{

					//add all the items to the array 
					std::for_each(items_to_add_04.begin(), items_to_add_04.end(), [&](auto& item)
						{
							bool found_added_item = false;

							auto begin = widenode_list.get_root_node_start(index);
							auto end = widenode_list.end();

							std::for_each(begin, end, [&](auto& x) {found_added_item |= (x == item); });

							//, "the value of x should exist"
							assert(!found_added_item);
						});
				});

		}
	};
}