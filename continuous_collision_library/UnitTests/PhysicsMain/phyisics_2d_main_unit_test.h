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

			using physics_main_type = phyisics_2d_main<std::numeric_limits<uint16>::max() - 1, 16>;
			//using physics_main_type = phyisics_2d_main<254, 16>;

			std::unique_ptr<physics_main_type> paged_hirachical_list = std::make_unique<physics_main_type>();

			physics_main_type::new_collider_data colider_to_add01;

			colider_to_add01.position = math_2d_util::fvec2d(0.9f);
			colider_to_add01.velocity = math_2d_util::fvec2d(0.0f);

			colider_to_add01.radius = 0.5f;
			
			physics_main_type::new_collider_data colider_to_add02;


			colider_to_add02.position = math_2d_util::fvec2d(69.0f);
			colider_to_add02.velocity = math_2d_util::fvec2d(69.0f);

			colider_to_add02.radius = 1.0f;

			//queue up a new item 
			auto colider_handle01 = paged_hirachical_list->try_queue_item_to_add( std::move(colider_to_add01));
			auto colider_handle02 = paged_hirachical_list->try_queue_item_to_add(std::move(colider_to_add02));

			paged_hirachical_list->update_physics();

		}
	};
};