#pragma once
#include "sector_grid_dimensions.h"
#include "sector_grid.h"
#include "base_types_definition.h"
#include "loose_grid_node.h"
#include "tight_grid_node.h"
#include "tight_grid_element.h"
#include "free_list.h"


namespace ContinuousCollisionLibrary
{
	using grid_dimensions = SectorGrid::sector_grid_dimensions<16, 16>;

	struct loose_tight_grid
	{
		//the loose grid structure
		SectorGrid::template_sector_grid<loose_grid_node_index, grid_dimensions> loose_sector_grid;
		
		//list for holding loose sector nodes, this is a linked list for all the tight cells that overlap a loose cell
		ArrayUtilities::FreeList<loose_grid_node> loose_nodes;
		
		//the tight grid structure 
		SectorGrid::template_sector_grid<tight_grid_node, grid_dimensions> tight_sector_grid;
		
		//in place linked list of all the elements that are in a cell
		ArrayUtilities::FreeList<tight_grid_element> items_in_grid;
	};
}

