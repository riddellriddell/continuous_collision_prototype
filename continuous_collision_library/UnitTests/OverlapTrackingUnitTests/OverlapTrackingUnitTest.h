#pragma once
#include <limits>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <ranges>
#include "../../overlap_tracking_grid.h"

namespace ContinuousCollisionLibrary
{
	static class overlap_tracking_unit_test
	{
	public:
		static void run_test()
		{
			//create an overlap tracking grid 
			auto overlap_grid = new ContinuousCollisionLibrary::overlap_tracking_grid();
			
			//setup grid
			//overlap_grid->initialize();

		}
	};
}
