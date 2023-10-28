#pragma once
#include "sector_grid_dimensions.h"
#include "int_type_selection.h"
#include <concepts>
#include <bit>
#include <bitset>
#include <limits>
#include "vector_types.h"
#include "sector_grid_base_type_definition.h"

namespace SectorGrid
{

	template<sector_grid_dimension_concept TSectorGridDimensions>
	struct sector_tile_index
	{
		//define the smallest int type able to hold the target number 
		typedef  MiscUtilities::uint_s<TSectorGridDimensions::sector_tile_count>::int_type_t tile_t; //type needed to hold the max value for tiles
		typedef  MiscUtilities::uint_s<TSectorGridDimensions::sector_grid_count_max>::int_type_t sector_t; //type needed to hold max value for sectors 

		static constexpr uint64 max_possible_tile_index = std::numeric_limits<tile_t>::max() * std::numeric_limits<sector_t>::max();

		typedef  MiscUtilities::uint_s<max_possible_tile_index>::int_type_t combined_index; //type needed to hold combined types 

#pragma pack(push,1)
		struct index_components
		{
			tile_t  sector_sub_tile_index;
			sector_t sector_index;
		};
#pragma pack(pop)

		union
		{
			combined_index index;
			index_components components;
		};

	};
	
}