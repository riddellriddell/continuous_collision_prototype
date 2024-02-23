#pragma once
#include "sector_grid_dimensions.h"
#include "misc_utilities/int_type_selection.h"
#include <concepts>
#include <bit>
#include <bitset>
#include <limits>
#include "vector_2d_math_utils/vector_types.h"
#include "sector_grid_base_type_definition.h"
#include "misc_utilities/type_static_assert_helper.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"

namespace SectorGrid
{

	template<sector_grid_dimension_concept TSectorGridDimensions>
	struct sector_tile_index
	{
		//MiscUtilities::debug_error_type<decltype(TSectorGridDimensions::sector_tile_count)> type_error0;
		//MiscUtilities::debug_error_type<decltype(TSectorGridDimensions::sector_grid_count_max)> type_error1;

		static constexpr uint32 bits_needed_for_tiles = MiscUtilities::bits_needed_to_represent_number(TSectorGridDimensions::sector_tile_count);
		static constexpr uint32 bits_needed_for_Sectors = MiscUtilities::bits_needed_to_represent_number(TSectorGridDimensions::sector_grid_count_max);
		static constexpr uint32 total_bits_needed = bits_needed_for_Sectors + bits_needed_for_tiles;

		static constexpr size_t max_possible_tile_index = (1 << total_bits_needed) - 1;

		using combined_index = MiscUtilities::uint_s<max_possible_tile_index>::int_type_t ; //type needed to hold combined types 
 
		//MiscUtilities::debug_error_type<decltype(TSectorGridDimensions::sector_grid_count_max)> type_error1;

		combined_index index;
	};
	
}