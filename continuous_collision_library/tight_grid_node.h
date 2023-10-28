#pragma once
#include "base_types_definition.h"
#include "vector_types.h"

namespace ContinuousCollisionLibrary
{
	struct tight_grid_node
	{
		math_2d_util::ivec2d rect_min;
		math_2d_util::ivec2d rect_max;

		tight_grid_element_index index_of_first_agent;
	};
}

