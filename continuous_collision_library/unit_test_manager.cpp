#include "pch.h"
#include "unit_test_manager.h"
#include "overlap_tracking_grid.h"
#include "sector_grid_data_structure/sector_grid_dimensions.h"

namespace ContinuousCollisionLibrary
{
	void unit_test_manager::test_overlap_tracking_grid()
	{
		//definition of the target dimensions of the grid system
		using grid_dimension_type = SectorGrid::sector_grid_dimensions<16, 16>;

		overlap_tracking_grid<grid_dimension_type>* overlap_tracking_grid = new ContinuousCollisionLibrary::overlap_tracking_grid< grid_dimension_type>();
	}

}