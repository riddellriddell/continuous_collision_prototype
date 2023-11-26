#pragma once
#include "sector_grid_base_type_definition.h"
#include <concepts>
#include <array>
#include <immintrin.h>
#include "sector.h"
#include "sector_grid_dimensions.h"
#include "vector_2d_math_utils/vector_types.h"
#include "sector_grid_index.h"
#include <assert.h>

namespace SectorGrid
{
	template<sector_grid_dimension_concept TSectorGridDimensions>
	struct sector_grid_helper
	{
		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;

		static constexpr combined_index sub_tile_bits_per_axis = std::bit_width(TSectorGridDimensions::sector_w - 1);

		static constexpr combined_index x_sub_sector_mask = ~(~0 << sub_tile_bits_per_axis);
		static constexpr combined_index y_sub_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index y_sub_sector_mask = x_sub_sector_mask << sub_tile_bits_per_axis;

		//check that the x and y masks dont overlap
		static_assert((x_sub_sector_mask& y_sub_sector_mask) == 0);


		static constexpr combined_index sector_bits_per_axis = std::bit_width(TSectorGridDimensions::sectors_grid_w - 1);

		static constexpr combined_index x_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index y_sector_shift = x_sector_shift + sector_bits_per_axis;

		static constexpr combined_index x_sector_mask = (~(~0 << sector_bits_per_axis)) << (x_sector_shift);
		static constexpr combined_index y_sector_mask = x_sector_mask << sector_bits_per_axis;

		//helper function to convert index to xy co ordinate 
		math_2d_util::uivec2d to_xy(const sector_tile_index<TSectorGridDimensions>& index) const;

		sector_tile_index<TSectorGridDimensions> from_xy(const math_2d_util::uivec2d& xy) const;

	};

	/// <summary>
	/// this stuct manages a grid of smaller grids called sectors with each sub grid being a 1d array mapped to a 2d tile array
	/// </summary>
	/// <typeparam name="TDataType"></typeparam>
	/// <typeparam name="TSectorGridDimensions"></typeparam>
	template<typename TDataType, sector_grid_dimension_concept TSectorGridDimensions>
	struct  template_sector_grid
	{
	public:
		union tile_sector_data_union
		{
			int test0;
			TDataType text1;
			//std::array<TDataType, TSectorGridDimensions::tile_count> tile_data;
			//std::array<sector<TDataType, TSectorGridDimensions>, TSectorGridDimensions::sector_grid_count> sector_data;

		}data;

		//make sure no new data elements have been added to the sector struct 
		static_assert(sizeof(std::array<TDataType, TSectorGridDimensions::tile_count>) == sizeof(std::array < sector<TDataType, TSectorGridDimensions>, TSectorGridDimensions::sector_grid_count>));

		//get a ref to the data at a sector tile index
		TDataType& get_ref_to_data(const sector_tile_index<TSectorGridDimensions>& index);

		constexpr template_sector_grid() = default;

	};

	template<typename TDataType, sector_grid_dimension_concept TSectorGridDimensions>
	inline TDataType& template_sector_grid<TDataType, TSectorGridDimensions>::get_ref_to_data(const sector_tile_index<TSectorGridDimensions>& index)
	{
		auto trash = TDataType();
		return trash;//data.tile_data[index.index];
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	inline math_2d_util::uivec2d sector_grid_helper<TSectorGridDimensions>::to_xy(const sector_tile_index<TSectorGridDimensions>& tile_index) const
	{

		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;


		//mask and shirf the sector x
		auto x_sector_bits = (tile_index.index & x_sector_mask);
		auto x_tile_bits = (tile_index.index & x_sub_sector_mask);
		auto x_out = (x_sector_bits >> x_sector_shift) | x_tile_bits;

		auto y_sector_bits = (tile_index.index & y_sector_mask);
		auto y_tile_bits = (tile_index.index & y_sub_sector_mask);

		auto y_out = (y_sector_bits >> x_sector_shift) | (y_tile_bits >> y_sub_sector_shift);

		return math_2d_util::uivec2d{x_out,y_out};
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	inline sector_tile_index<TSectorGridDimensions> sector_grid_helper<TSectorGridDimensions>::from_xy(const math_2d_util::uivec2d& xy) const
	{

		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;


		const combined_index x_sub_tile_component = xy.x & x_sub_sector_mask;
		const combined_index y_sub_tile_component = xy.y & x_sub_sector_mask;

		const combined_index x_sector_component = xy.x & (~x_sub_sector_mask);
		const combined_index y_sector_component = xy.y & (~x_sub_sector_mask);

		const combined_index out_index_val =
			x_sub_tile_component |
			(y_sub_tile_component << y_sub_sector_shift) |
			(x_sector_component << x_sector_shift) |
			(y_sector_component << y_sector_shift);

		assert(out_index_val < TSectorGridDimensions::tile_count, "address falls outside grid");

		sector_tile_index<TSectorGridDimensions> out_index = sector_tile_index<TSectorGridDimensions>{ out_index_val };

		return out_index;
	}

}

