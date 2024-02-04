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
		using index_type = MiscUtilities::uint_s<Icount>::uint_t;

		static constexpr invalid_value = 

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
			std::iota(data.begin(), data.end(), static_cast<Thandle_type>(1));
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
	};

}