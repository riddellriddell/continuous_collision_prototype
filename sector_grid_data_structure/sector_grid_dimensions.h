#pragma once
#include "sector_grid_base_type_definition.h"

namespace SectorGrid
{
	/// <summary>
	/// this struct is a helper stuct that defines the size of sectors in a sector grid along with the size of the grid
	/// </summary>
	template<uint32 TGridSectorW, uint32 TSectorTileW>
	struct sector_grid_dimensions
	{
		static constexpr uint32 sectors_grid_w = TGridSectorW;
		static constexpr uint32 sector_grid_count = sectors_grid_w * sectors_grid_w;

		static constexpr uint32 sector_grid_count_max = sector_grid_count;

		static constexpr uint32 sector_w = TSectorTileW;
		static constexpr uint32 sector_tile_count = sector_w * sector_w;


		static constexpr uint32 tile_w = sectors_grid_w * sector_w;
		static constexpr uint32 tile_count = tile_w * tile_w;

		static constexpr uint32 tile_count_max = tile_count;
	};

	//concept ensuring grid templates are using the correct types
	template<typename T>
	concept sector_grid_dimension_concept = requires(T a)
	{
		{ sector_grid_dimensions{ a } };
	};
}
