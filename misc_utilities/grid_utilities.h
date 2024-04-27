#pragma once

#include <cinttypes>
#include <array>
#include "shared_types.h"

//the purpose of this class is to help performing a computation on grid data efficiently 

namespace MiscUtilities
{
	template<int32_t Iwidth, int32_t Iheight = Iwidth, typename Tint_type = int32_t>
	struct grid_navigation_helper
	{
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

		template<size_t Inum>
		static constexpr std::array<Tint_type,Inum> get_offset_for_direction(std::array<grid_directions, Inum> directions)
		{
			std::array<Tint_type, Inum> output = {};

			for (int i = 0; i < Inum; ++i)
			{
				output[i] = get_offset_for_direction(directions[i]);
			}
			return output;
		}

		template<grid_directions Edirection>
		static constexpr Tint_type get_address_for_direction( auto address)
		{
			return address += all_direction_offset[static_cast<uint32_t>(Edirection)];
		}

		template<typename Treturn>
		static constexpr auto get_non_blocked_cardinal_directions2()
		{
			using edge_info = Treturn;

			return true;
		}


		template<typename Tin_type,  typename Treturn>
		static constexpr Treturn get_non_blocked_cardinal_directions3()
		{
			using edge_info = Treturn;

			return true;
		}

		
		static constexpr auto get_non_blocked_cardinal_directions(auto type_insatnce)
		{
			using edge_info = decltype(type_insatnce);

			return true;

			//static constexpr uint32_t valid_direction_count =
			//	Tedge_info::is_on_left_edge() +
			//	Tedge_info::is_on_up_edge() +
			//	Tedge_info::is_on_right_edge() +
			//	Tedge_info::is_on_down_edge();
			
			//return valid_direction_count;

			//std::array< grid_directions, valid_direction_count> valid_direction_array;
			//
			//uint32_t write_index = 0;
			//
			//if constexpr (!Tedge_info::is_on_left_edge())
			//{
			//	valid_direction_array[write_index++] = grid_directions::LEFT;
			//}
			//
			//if constexpr (!Tedge_info::is_on_up_edge())
			//{
			//	valid_direction_array[write_index++] = grid_directions::UP;
			//}
			//
			//if constexpr (!Tedge_info::is_on_right_edge())
			//{
			//	valid_direction_array[write_index++] = grid_directions::RIGHT;
			//}
			//
			//if constexpr (!Tedge_info::is_on_down_edge())
			//{
			//	valid_direction_array[write_index++] = grid_directions::DOWN;
			//}
			//
			//return valid_direction_array;

		};
		
		/*
		template<typename Tedge_info>
		static constexpr auto get_flipped_non_blocked_cardinal_directions()
		{
			static constexpr uint32_t valid_direction_count =
				Tedge_info::is_on_left_edge() +
				Tedge_info::is_on_up_edge() +
				Tedge_info::is_on_right_edge() +
				Tedge_info::is_on_down_edge();

			std::array< grid_directions, valid_direction_count> valid_direction_array;

			uint32_t write_index = 0;

			if constexpr (!Tedge_info::is_on_left_edge())
			{
				valid_direction_array[write_index++] = grid_directions::RIGHT;
			}

			if constexpr (!Tedge_info::is_on_up_edge())
			{
				valid_direction_array[write_index++] = grid_directions::DOWN;
			}

			if constexpr (!Tedge_info::is_on_right_edge())
			{
				valid_direction_array[write_index++] = grid_directions::LEFT;
			}

			if constexpr (!Tedge_info::is_on_down_edge())
			{
				valid_direction_array[write_index++] = grid_directions::UP;
			}

			return valid_direction_array;

		}
		*/
	};
}