#pragma once
#include "base_types_definition.h"
namespace ContinuousCollisionLibrary
{
	struct loose_grid_node
	{
		//index of next loose node 
		loose_grid_node_index next_node;

		//index of the first node overlapping this one
		tight_grid_node_index overlapping_tight_node;
	};
}