#pragma once

#include <cinttypes>
#include <array>

//the purpose of this class is to help performing a computation on grid data efficiently 

namespace MiscUtilities
{
	template<int32_t Iwidth, int32_t Iheight = Iwidth, typename Tint_type = int32_t>
	struct grid_navigation_helper
	{
		enum class grid_directions : uint8_t
		{
			LEFT,
			UP,
			RIGHT,
			DOWN,
			UP_LEFT,
			UP_RIGHT,
			DOWN_RIGHT,
			DOWN_LEFT,
			COUNT,
			CARDINAL_COUNT = UP_LEFT,
			DIAGONAL_START = UP_LEFT,
			DIAGONAL_COUNT = COUNT - DIAGONAL_START
		};

		//using index_value_type = 

		static constexpr std::array< Tint_type, static_cast<uint32_t>(grid_directions::COUNT)> all_direction_offset
		{
			Tint_type(-1), //left
			Tint_type(-Iwidth), //up
			Tint_type(1), //right
			Tint_type(Iwidth), //down
			Tint_type(-1) + Tint_type(-Iwidth), //up left
			Tint_type(1) + Tint_type(-Iwidth), //up right
			Tint_type(1) + Tint_type(Iwidth), //down right
			Tint_type(-1) + Tint_type(Iwidth), //down left
		};

		//get the offset for a direction
		template< grid_directions Edirection >
		static constexpr Tint_type get_offset_for_direction()
		{
			return all_direction_offset[static_cast<uint32_t>(Edirection)];
		}

		static constexpr Tint_type get_offset_for_direction(grid_directions direction)
		{
			return all_direction_offset[static_cast<uint32_t>(direction)];
		}

		template<grid_directions Edirection>
		static constexpr Tint_type get_address_for_direction( auto address)
		{
			return address += all_direction_offset[static_cast<uint32_t>(Edirection)];
		}
	};
}