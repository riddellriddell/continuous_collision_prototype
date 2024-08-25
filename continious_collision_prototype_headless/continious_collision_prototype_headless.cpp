
// continious_collision_prototype_headless.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "misc_utilities/delata_time_util.h"

//#include "array_utilities/WideNodeLinkedList/UnitTests/wide_node_linked_list_unit_tests.h"
//#include "array_utilities/UnitTests/unit_test_manager.h"
//#include "continuous_collision_library/UnitTests/OverlapTrackingUnitTests/OverlapTrackingUnitTest.h"


#include "continuous_collision_library/2d_physics_main.h"

#include "continuous_collision_library/UnitTests/PhysicsMain/phyisics_2d_main_unit_test.h"

int main()
{
    std::cout << "Starting Test!\n";

    std::cout << "Making Physics System\n";
    using physics_main_type = ContinuousCollisionLibrary::phyisics_2d_main<std::numeric_limits<ContinuousCollisionLibrary::uint16>::max() - 1,8>;
    //using physics_main_type = phyisics_2d_main<254, 16>;

   
    std::unique_ptr<physics_main_type> physics_main = std::make_unique<physics_main_type>();

    //std::cout << "Running Tests\n";
    //test the physics system
    //ContinuousCollisionLibrary::phyisics_2d_main_unit_test::run_test();


    //setup the physics library 
    //setup_physics_main();

    std::cout << "Spawning all items\n";
    //setup physics 
    physics_main->setup_physics_random(65, 200);

    static constexpr int itterations = 100;

    std::cout << "Starting Stepping System\n";

    //run in loop for x number of steps
    for (int i = 0; i < itterations; ++i)
    {
        std::cout << "calculatingStep! " << i << " \n";
        //update the physics 
        physics_main->update_physics();
    }

    std::cout << "TestFinished!\n";

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
