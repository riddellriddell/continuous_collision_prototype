#pragma once
#include "sector_grid_base_type_definition.h"
#include <concepts>
#include <array>
#include <immintrin.h>
#include "sector.h"
#include "sector_grid_dimensions.h"
#include "vector_2d_math_utils/vector_types.h"
#include "vector_2d_math_utils/rect_types.h"
#include "sector_grid_index.h"
#include <assert.h>

namespace SectorGrid
{
	template<sector_grid_dimension_concept TSectorGridDimensions>
	struct sector_grid_helper
	{
		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;

		static constexpr combined_index sub_sector_bits = std::bit_width((TSectorGridDimensions::sector_w * TSectorGridDimensions::sector_w) -1);
		static constexpr combined_index sector_mask = (~0 << sub_sector_bits);
		static constexpr combined_index sub_sector_mask = ~sector_mask;

		static constexpr combined_index sub_tile_bits_per_axis = std::bit_width(TSectorGridDimensions::sector_w - 1);
		static constexpr combined_index sector_bits_per_axis = std::bit_width(TSectorGridDimensions::sectors_grid_w - 1);

		//values for converting from xy
		static constexpr combined_index axis_sub_sector_mask = ~(~0 << sub_tile_bits_per_axis);
		static constexpr combined_index axis_sector_mask = ~axis_sub_sector_mask;
		static constexpr combined_index x_axis_sub_sector_shift = 0;
		static constexpr combined_index y_axis_sub_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index x_axis_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index y_axis_sector_shift = sub_tile_bits_per_axis + sector_bits_per_axis;


		//values for converting from index
		static constexpr combined_index x_sub_sector_mask = axis_sub_sector_mask;

		static constexpr combined_index y_sub_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index y_sub_sector_mask = x_sub_sector_mask << sub_tile_bits_per_axis;

		//value to convert from sector index to sector bounds 
		static constexpr combined_index sector_index_x_mask = TSectorGridDimensions::sectors_grid_w - 1;
	

		//check that the x and y masks dont overlap
		static_assert((x_sub_sector_mask& y_sub_sector_mask) == 0);


		static constexpr combined_index x_sector_shift = sub_tile_bits_per_axis;
		static constexpr combined_index y_sector_shift = sub_tile_bits_per_axis + sector_bits_per_axis;

		static constexpr combined_index x_sector_mask = (~(~0 << sector_bits_per_axis)) << (sub_tile_bits_per_axis * 2);
		static constexpr combined_index y_sector_mask = x_sector_mask << sector_bits_per_axis;

		using sector_tile_index_type = sector_tile_index<TSectorGridDimensions>;


		//helper function to convert index to xy co ordinate 
		template<typename Treturn_type = math_2d_util::uivec2d>
		Treturn_type to_xy(const sector_tile_index<TSectorGridDimensions>& index) const;

		template<typename Tcord_type = math_2d_util::uivec2d>
		sector_tile_index_type from_xy(const Tcord_type& xy) const;

		uint32 to_sector_index(const sector_tile_index<TSectorGridDimensions>& index) const;

		template<typename Tcord_type = math_2d_util::uivec2d>
		uint32 to_sector_index(const Tcord_type& xy) const;

		template<typename Tcord_type = math_2d_util::uivec2d>
		uint32 to_sector_index_from_sector_xy(const Tcord_type& xy) const;

		uint32 to_sub_sector_index(const sector_tile_index<TSectorGridDimensions>& index) const;

		math_2d_util::uirect sector_bounds(const auto sector_index);

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
			std::array<TDataType, TSectorGridDimensions::tile_count> tile_data;
			std::array<sector<TDataType, TSectorGridDimensions>, TSectorGridDimensions::sector_grid_count> sector_data;

		}data;

		//make sure no new data elements have been added to the sector struct 
		static_assert(sizeof(std::array<TDataType, TSectorGridDimensions::tile_count>) == sizeof(std::array < sector<TDataType, TSectorGridDimensions>, TSectorGridDimensions::sector_grid_count>));

		//get a ref to the data at a sector tile index
		TDataType& get_ref_to_data(const sector_tile_index<TSectorGridDimensions>& index);

		void set_data(const sector_tile_index<TSectorGridDimensions>& index, const TDataType& data_to_set);

		constexpr template_sector_grid() = default;

		//max capacity of all the tiles in this lookup
		constexpr decltype(TSectorGridDimensions::tile_count) size()
		{
			return TSectorGridDimensions::tile_count;
		}

		//clears all the data to 0 
		constexpr void clear_data()
		{
			std::fill_n(data.tile_data.data(), size(), TDataType{});
		}
	};

	template<typename TDataType, sector_grid_dimension_concept TSectorGridDimensions>
	inline TDataType& template_sector_grid<TDataType, TSectorGridDimensions>::get_ref_to_data(const sector_tile_index<TSectorGridDimensions>& index)
	{
		return data.tile_data[index.index];
	}

	template<typename TDataType, sector_grid_dimension_concept TSectorGridDimensions>
	inline void template_sector_grid<TDataType, TSectorGridDimensions>::set_data(const sector_tile_index<TSectorGridDimensions>& index, const TDataType& data_to_set)
	{
		data.tile_data[index.index] = data_to_set;
	}



	template<sector_grid_dimension_concept TSectorGridDimensions>
	template<typename Treturn_type>
	inline Treturn_type sector_grid_helper<TSectorGridDimensions>::to_xy(const sector_tile_index<TSectorGridDimensions>& tile_index) const
	{
		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;

		//mask and shirf the sector x
		uint32 x_sector_bits = (tile_index.index & x_sector_mask);
		uint32 x_tile_bits = (tile_index.index & x_sub_sector_mask);
		uint32 x_out = (x_sector_bits >> x_sector_shift) | x_tile_bits;

		uint32 y_sector_bits = (tile_index.index & y_sector_mask);
		uint32 y_tile_bits = (tile_index.index & y_sub_sector_mask);

		uint32 y_out = (y_sector_bits >> y_sector_shift) | (y_tile_bits >> y_sub_sector_shift);

		return Treturn_type(static_cast<Treturn_type::axis_type>(x_out), static_cast<Treturn_type::axis_type>(y_out));
	}


	template<sector_grid_dimension_concept TSectorGridDimensions>
	template<typename Tcord_type>
	inline sector_tile_index<TSectorGridDimensions> sector_grid_helper<TSectorGridDimensions>::from_xy(const Tcord_type& xy) const
	{
		//make sure no negative numbers are passed in as that will break the calculations of the index 
		assert(xy.x >= 0 && xy.y >= 0);

		using combined_index = sector_tile_index<TSectorGridDimensions>::combined_index;

		const combined_index x_sub_tile_component = xy.x & axis_sub_sector_mask;
		const combined_index y_sub_tile_component = xy.y & axis_sub_sector_mask;

		const combined_index x_sector_component = xy.x & (axis_sector_mask);
		const combined_index y_sector_component = xy.y & (axis_sector_mask);

		const combined_index out_index_val =
			x_sub_tile_component |
			(y_sub_tile_component << y_axis_sub_sector_shift) |
			(x_sector_component << x_axis_sector_shift) |
			(y_sector_component << y_axis_sector_shift);

		assert(out_index_val < TSectorGridDimensions::tile_count );// "address falls outside grid");

		sector_tile_index<TSectorGridDimensions> out_index = sector_tile_index<TSectorGridDimensions>{ out_index_val };

		return out_index;
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	inline uint32 sector_grid_helper<TSectorGridDimensions>::to_sector_index(const sector_tile_index<TSectorGridDimensions>& index) const
	{
		return static_cast<uint32>(index.index >> sub_sector_bits);
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	template<typename Tcord_type>
	inline uint32 sector_grid_helper<TSectorGridDimensions>::to_sector_index(const Tcord_type& xy) const
	{
		//make sure no negative numbers are passed in as that will break the calculations of the index 
		assert(xy.x >= 0 && xy.y >= 0);

		//extract the sector component 
		const uint32 x_sector_component = xy.x & (axis_sector_mask);
		uint32 y_sector_component = xy.y & (axis_sector_mask);

		//offset the y component 
		y_sector_component = y_sector_component << sector_bits_per_axis;

		//combine the components 
		uint32 combined = y_sector_component | x_sector_component;

		return combined >> sub_tile_bits_per_axis;
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	template<typename Tcord_type>
	inline uint32 sector_grid_helper<TSectorGridDimensions>::to_sector_index_from_sector_xy(const Tcord_type& xy) const
	{
		//make sure no negative numbers are passed in as that will break the calculations of the index 
		assert(xy.x >= 0 && xy.y >= 0);

		//offset the y component 
		uint32 y_sector_component = xy.y << sector_bits_per_axis;

		//combine the components 
		uint32 combined = y_sector_component | xy.x;

		return combined;
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	inline uint32 sector_grid_helper<TSectorGridDimensions>::to_sub_sector_index(const sector_tile_index<TSectorGridDimensions>& index) const
	{
		return static_cast<uint32>(index.index & sub_sector_mask);
	}

	template<sector_grid_dimension_concept TSectorGridDimensions>
	inline math_2d_util::uirect sector_grid_helper<TSectorGridDimensions>::sector_bounds(const auto sector_index)
	{
		math_2d_util::uirect out_sector_bounds;
		
		out_sector_bounds.min.x = TSectorGridDimensions::sector_w * (sector_index & sector_index_x_mask);
		out_sector_bounds.min.y = TSectorGridDimensions::sector_w * (sector_index >> sector_bits_per_axis);



		out_sector_bounds.max.x = out_sector_bounds.min.x + TSectorGridDimensions::sector_w;
		out_sector_bounds.max.y = out_sector_bounds.min.y + TSectorGridDimensions::sector_w;
		
		return out_sector_bounds;
	}

}

