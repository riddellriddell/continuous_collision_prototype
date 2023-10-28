#pragma once
#include "base_types_definition.h"
#include "vector_types.h"

namespace ContinuousCollisionLibrary
{
	struct tight_grid_element
	{
		tight_grid_element_index next_element; //next element in the linked list

		external_element_handle external_handle; //handle for external elelment

		math_2d_util::fvec2d position; //pos of item 

		float radius; //radius of item
	};
}