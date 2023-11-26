#pragma once
#include "sector_grid_base_type_definition.h"
#include <array>

namespace SectorGrid
{
	//this is a wrapper for the internal data and only exists to make it easier to work with sectors and not access outside their bounds 
	template <typename TDataType, typename TSectorGridDimensions>
	struct sector
	{
		sector() = default;

		//all the data held by the sector
		std::array<TDataType, TSectorGridDimensions::sector_tile_count> data;

	};

}