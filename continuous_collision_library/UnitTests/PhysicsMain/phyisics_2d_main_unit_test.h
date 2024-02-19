#pragma once

#include <memory>

#include "continuous_collision_library/2d_physics_main.h"



namespace ContinuousCollisionLibrary
{
	static class phyisics_2d_main_unit_test
	{
	public:
		static void run_test()
		{

			//using physics_main_type = phyisics_2d_main<std::numeric_limits<uint16>::max() - 1, 16>;
			using physics_main_type = phyisics_2d_main<254, 16>;

			std::unique_ptr<physics_main_type> paged_hirachical_list = std::make_unique<physics_main_type>();

		}
	};
};