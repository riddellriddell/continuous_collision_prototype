#include "pch.h"
#include "unit_test_manager.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>

void SectorGrid::unit_test_manager::run_basic_test()
{
	using dimensions = sector_grid_dimensions<8, 16>;

	std::unique_ptr<template_sector_grid<uint32, dimensions>> grid_instance(new template_sector_grid<uint32, dimensions>());

	std::srand(std::time(nullptr)); // use current time as seed for random generator


	for (uint32 i = 0; i < grid_instance->data.tile_data.size(); i++)
	{
		int random_variable = std::rand();

		grid_instance->data.tile_data[i] = static_cast<uint32>(random_variable);
	}

	for (uint32 i = 0; i < grid_instance->data.tile_data.size(); i++)
	{
		std::cout << "tile value" << grid_instance->data.tile_data[i];
	}
}
