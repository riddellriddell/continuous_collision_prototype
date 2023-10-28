#pragma once
#include "base_types_definition.h"
#include "small_list.h"
#include "free_list.h"
//#include "wide_node_linked_list.h"

#include <cstdlib>
#include <ctime>

namespace ArrayUtilities
{
	class unit_test_manager
	{
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
			//wide_node_linked_list<std::numeric_limits<uint8>::max,
		}
	};
}
