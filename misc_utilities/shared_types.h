#pragma once
#include <cstdint>

namespace MiscUtilities
{
	//basic directions enum
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
}