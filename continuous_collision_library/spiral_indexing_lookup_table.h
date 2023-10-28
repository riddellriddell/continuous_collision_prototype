#pragma once
#include "base_types_definition.h"
#include "vector_types.h"
#include <array>
#include <algorithm>

namespace ContinuousCollisionLibrary
{

	static constexpr uint32 to_lookup_index(uint32 width, const math_2d_util::ivec2d& coordinate)
	{
		return coordinate.x + (coordinate.y * width);
	}

	static constexpr bool is_valid_coordinate(uint32 width, const math_2d_util::ivec2d& coordinate)
	{
		return (coordinate.x < width) && (coordinate.x >= 0) && (coordinate.y < width) && (coordinate.y >= 0);
	}

	template<uint32 Iwidth, uint32 IcenterX, uint32 IcenterY>
	static constexpr std::array<uint32, Iwidth* Iwidth> calculate_spiral_index_lookup()
	{
		//direction enum
		enum class direction :  uint32
		{
			RIGHT, 
			DOWN, 
			LEFT, 
			UP, 
			COUNT
		};

		std::array<math_2d_util::ivec2d, static_cast<uint32>(direction::COUNT)> itteration_direction_vec =
		{
			math_2d_util::ivec2d{1,0},
			math_2d_util::ivec2d{0,1},
			math_2d_util::ivec2d{-1,0},
			math_2d_util::ivec2d{0,-1}
		};

		//create lookup
		std::array<uint32, Iwidth* Iwidth> spiral_index_lookup = {};

		math_2d_util::ivec2d center = { IcenterX ,IcenterY };

		//set start point to last index 
		spiral_index_lookup[to_lookup_index(center)] = (Iwidth * Iwidth) - 1;

		//
		direction itteration_direction = direction::RIGHT;
		uint32 tiles_per_side = 2;
		uint32 spiral_index = 0;
		math_2d_util::ivec2d itteration_cord = center - (math_2d_util::ivec2d(1,1);

		for (int ishell = 0; ishell < Iwidth; ++ishell)
		{
			//update the number of tiles per side
			tiles_per_side = 2 + (ishell * 2);

			for (int idirection = 0; idirection < static_cast<uint32>(direction::COUNT); ++idirection)
			{
				itteration_direction = static_cast<direction>(idirection);

				//itterate over all tiles on a side and set index 
				for (int iside = 0; iside < tiles_per_side; ++iside)
				{
					//check if cord is valid
					if (is_valid_coordinate(itteration_cord))
					{
						uint32 lookup_index = to_lookup_index(itteration_cord);

						spiral_index_lookup[lookup_index] = spiral_index++;
					}

					//advance to the next cord
					itteration_cord += itteration_direction_vec[static_cast<uint32>(itteration_direction)];
				}
			}
		}

		return spiral_index_lookup;

	}

	template<uint32 Iwidth, uint32 IcenterX, uint32 IcenterY>
	static constexpr std::array<uint8, Iwidth* Iwidth> calculate_afinity_for_tiles()
	{
		static constexpr std::array<uint32, Iwidth* Iwidth> spiral_index_lookup = calculate_spiral_index_lookup<Iwidth, IcenterX, IcenterY>();

		uint32 lowwer_bit_masks = 0b111;

		//create lookup
		std::array<uint8, Iwidth* Iwidth> spiral_afinity_lookup = {};

		//store only the bottom 3 bits which go from 0 to 1 
		for (int i = 0; i < spiral_afinity_lookup.size; ++i)
		{
			spiral_afinity_lookup[i] = spiral_index_lookup[i] & lowwer_bit_masks;
		}

		return spiral_afinity_lookup;
	}


	template<uint32 Iwidth, uint32 IcenterX, uint32 IcenterY>
	struct spiral_index_lookup_table
	{
		//lookup table for converting from byte index in a 16 x 16 grid to a spiral index centered on the given xy value 
		static constexpr std::array<uint8, Iwidth* Iwidth> spiral_index_lookup = calculate_afinity_for_tiles<Iwidth, IcenterX,IcenterY>();
	};
}