#pragma once
#include <array>
#include <limits>
#include <numeric>
#include "misc_utilities/int_type_selection.h"

namespace ArrayUtilities
{
	template<size_t Icount, typename Thandle_type>
	struct fixed_free_list
	{
		//one is added to make sure that we have space for an invalid value type
		using index_type = MiscUtilities::uint_s<Icount + 1>::uint_t;

		static constexpr index_type  invalid_value = std::numeric_limits< index_type>::max();

		//fire a warning if we had to increase the size of the underlying type because of ICount being the max size of a type
		static_assert(sizeof(index_type) <= sizeof(Thandle_type));

		union list_type
		{
			index_type next;
			Thandle_type element;
		};

		std::array<Thandle_type, Icount> data;
		index_type free_start;


		fixed_free_list(): free_start(0)
		{
			// Fill data using std::iota
			std::iota(data.begin(), data.end(), static_cast<index_type>(1));

			// mark the last element with an invalid value 
			(*data.back()) = index_type = invalid_value;
		}

		// Accessor for operator[]
		Thandle_type& operator[](index_type index)
		{
			return data[index].element;
		}

		// Function to return an item to the free list
		void return_to_free_list(index_type index)
		{
			data[index].next = free_start; // Set next pointer to current free_start
			free_start = index;        // Update free_start to the returned index
		}

		//gets the index of the next free element 
		index_type get_free_element()
		{
			index_type takenIndex = free_start;
			free_start = data[free_start].next; // Update free_start to the next element in the free list
			return takenIndex;
		}

		static bool is_valid_index(index_type index)
		{
			//since it is max value or all 1's flipping will result in 0 if it is max value 
			//and not zero in all other cases 
			return ~index;
		}
	};

}