#pragma once
#include <cinttypes>
#include <functional>

#include "shared_types.h"
//the purpose of this class is to help performing a computation on grid data efficiently 

namespace MiscUtilities
{
	struct grid_function_helper
	{

		struct on_edge
		{
			bool left;
			bool up;
			bool right;
			bool down;
			
			template<grid_directions edge>
			bool is_on_edge() const
			{

				if constexpr (edge == grid_directions::LEFT)
				{
					return left;
				}

				if constexpr (edge == grid_directions::UP)
				{
					return up;
				}

				if constexpr (edge == grid_directions::RIGHT)
				{
					return right;
				}

				if constexpr (edge == grid_directions::DOWN)
				{
					return down;
				}

				return false;
			}
			
			template<grid_directions corner>
			bool is_on_corner() const
			{
				if constexpr (corner == grid_directions::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (corner == grid_directions::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (corner == grid_directions::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (corner == grid_directions::DOWN_RIGHT)
				{
					return down && right;
				}

				return false;
			}

			bool is_in_center()
			{
				return !(up || left || right || down);
			}
					
		};

		template<bool left, bool up, bool right, bool down>
		struct on_edge_template
		{
			template<grid_directions edge>
			static constexpr bool is_edge()
			{
				if constexpr (edge == grid_directions::LEFT)
				{
					return left;
				}

				if constexpr (edge == grid_directions::UP)
				{
					return up;
				}

				if constexpr (edge == grid_directions::RIGHT)
				{
					return right;
				}

				if constexpr (edge == grid_directions::DOWN)
				{
					return down;
				}

				return false;
			}

			template<grid_directions edge>
			static constexpr bool is_edge_direction()
			{
				if constexpr (edge == grid_directions::LEFT)
				{
					return left && !(up || down || right);
				}

				if constexpr (edge == grid_directions::UP)
				{
					return up && !(left || right || down);
				}

				if constexpr (edge == grid_directions::RIGHT)
				{
					return right && !(up || down || left);
				}

				if constexpr (edge == grid_directions::DOWN)
				{
					return down && !(up || left || right);
				}

				if constexpr (edge == grid_directions::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (edge == grid_directions::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (edge == grid_directions::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (edge == grid_directions::DOWN_RIGHT)
				{
					return down && right;
				}

				return false;
			}

			template<grid_directions corner>
			static constexpr bool is_corner()
			{
				if constexpr (corner == grid_directions::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (corner == grid_directions::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (corner == grid_directions::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (corner == grid_directions::DOWN_RIGHT)
				{
					return down && right;
				}

				return false;
			}

			static constexpr bool is_left()
			{
				return is_edge_direction<grid_directions::LEFT>();
			}

			static constexpr bool is_up_left()
			{
				return is_edge_direction<grid_directions::UP_LEFT>();
			}

			static constexpr bool is_up()
			{
				return is_edge_direction<grid_directions::UP>();
			}

			static constexpr bool is_up_right()
			{
				return is_edge_direction<grid_directions::UP_RIGHT>();
			}

			static constexpr bool is_right()
			{
				return is_edge_direction<grid_directions::RIGHT>();
			}

			static constexpr bool is_down_right()
			{
				return is_edge_direction<grid_directions::DOWN_RIGHT>();
			}

			static constexpr bool is_down()
			{
				return is_edge_direction<grid_directions::DOWN>();
			}

			static constexpr bool is_down_left()
			{
				return is_edge_direction<grid_directions::DOWN_LEFT>();
			}

			static constexpr bool is_center()
			{
				return !(up || left || right || down);
			}
		
			static constexpr bool is_on_left_edge()
			{
				return is_edge<grid_directions::LEFT>();
			}

			static constexpr bool is_on_up_edge()
			{
				return is_edge<grid_directions::UP>();
			}

			static constexpr bool is_on_right_edge()
			{
				return is_edge<grid_directions::RIGHT>();
			}

			static constexpr bool is_on_down_edge()
			{
				return is_edge<grid_directions::DOWN>();
			}
		
			static consteval auto get_non_edge_cardinal_directions()
			{
				std::array< grid_directions, !left + !up + !right + !down> non_edge_directions{};

				uint32_t write_index = 0;

				if (!left)
				{
					non_edge_directions[write_index++] = grid_directions::LEFT;
				}

				if (!up)
				{
					non_edge_directions[write_index++] = grid_directions::UP;
				}

				if (!right)
				{
					non_edge_directions[write_index++] = grid_directions::RIGHT;
				} 

				if (!down)
				{
					non_edge_directions[write_index++] = grid_directions::DOWN;
				}

				return non_edge_directions;
			}

			static consteval auto get_non_edge_diagonal_directions()
			{
				std::array< grid_directions, (!left && !up) + (!up && !right) + (!right && !down) + (!down && !left)> non_edge_directions;

				uint32_t write_index = 0;

				if (!left && !up)
				{
					non_edge_directions[write_index++] = grid_directions::UP_LEFT;
				}

				if (!up && !right)
				{
					non_edge_directions[write_index++] = grid_directions::UP_RIGHT;
				}

				if (!right && !down)
				{
					non_edge_directions[write_index++] = grid_directions::DOWN_RIGHT;
				}

				if (!down && !left)
				{
					non_edge_directions[write_index++] = grid_directions::DOWN_LEFT;
				}

				return non_edge_directions;
			}

			static consteval auto get_non_edge_directions()
			{
				constexpr auto non_edge_cardinal = get_non_edge_cardinal_directions();
				constexpr auto non_edge_diagonal = get_non_edge_diagonal_directions();

				std::array<grid_directions, non_edge_cardinal.size() + non_edge_diagonal.size()> output{};

				// Copy elements from arr1
				std::copy(non_edge_cardinal.begin(), non_edge_cardinal.end(), output.begin());

				// Copy elements from arr2
				std::copy(non_edge_diagonal.begin(), non_edge_diagonal.end(), output.begin() + non_edge_cardinal.size());

				return output;
			}


			template<size_t Inum>
			static consteval std::array< grid_directions, Inum> get_directions_array_flipped(std::array<grid_directions, Inum> in_direction_array)
			{
				std::array< grid_directions, Inum> output{};

				for (int i = 0; i < Inum; i++)
				{
					switch (in_direction_array[i])
					{
					case grid_directions::LEFT:
						output[i] = grid_directions::RIGHT;
						break;
					case grid_directions::UP_LEFT:
						output[i] = grid_directions::DOWN_RIGHT;
						break;
					case grid_directions::UP:
						output[i] = grid_directions::DOWN;
						break;
					case grid_directions::UP_RIGHT:
						output[i] = grid_directions::DOWN_LEFT;
						break;
					case grid_directions::RIGHT:
						output[i] = grid_directions::LEFT;
						break;
					case grid_directions::DOWN_RIGHT:
						output[i] = grid_directions::UP_LEFT;
						break;
					case grid_directions::DOWN:
						output[i] = grid_directions::UP;
						break;
					case grid_directions::DOWN_LEFT:
						output[i] = grid_directions::UP_RIGHT;
						break;

					}
				}

				return output;
			}

		};


		//this function iterates over all tiles in a grid passing in a coordinate as well as a struct that indicates which edge a tile is on
		static void edge_corner_and_fill_function(const uint32_t width,const uint32_t height, const uint32_t start_x, const uint32_t start_y, std::function<void(const uint32_t, const uint32_t,const on_edge)> grid_func)
		{
			//setup edge indicator
			static constexpr on_edge up_left{ true,true,false,false };
			static constexpr on_edge up{ false,true,false,false };
			static constexpr on_edge up_right{ false,true,true,false };
			static constexpr on_edge left{ true,false,false,false};
			static constexpr on_edge center{ false,false,false,false };
			static constexpr on_edge right{ false,false,true,false };
			static constexpr on_edge down_left{ true,false,false,true };
			static constexpr on_edge down{ false,false,false,true };
			static constexpr on_edge down_right{ false,false,true,true };

			const uint32_t x_max = start_x + width - 1;
			const uint32_t y_max = start_y + height - 1;

			//do the top left corner 
			grid_func(start_x, start_y, up_left);
			
			//do the top
			for (int ix = start_x + 1; ix < x_max; ++ix)
			{
				grid_func(ix, start_y, up);
			}

			//do the top right corner
			grid_func(x_max, start_y, up_right);

			//do the center 
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				for (uint32_t ix = start_x + 1; ix < x_max; ++ix)
				{
					//do the center
					grid_func(ix, iy, center);
				}
			}

			//do the left edge
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				grid_func(start_x, iy, left);
			}
			
			//do the right edge 
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				grid_func(x_max, iy, right);
			}

			//do the bottom left corner 
			grid_func(start_x, y_max, down_left);

			//do the bottom 
			for (int ix = start_x + 1; ix < x_max; ++ix)
			{
				grid_func(ix, y_max, down);
			}

			//do the bottom right corner 
			grid_func(x_max, y_max, down_right);
		}

		//this function iterates over all tiles in a grid passing in a coordinate as well as a struct that indicates which edge a tile is on
		static void template_edge_corner_and_fill_function(const uint32_t width, const uint32_t height, const uint32_t start_x, const uint32_t start_y, auto grid_func)
		{

			const uint32_t x_max = start_x + width - 1;
			const uint32_t y_max = start_y + height - 1;

			//do the top left corner 
			grid_func.operator() < on_edge_template< true, true, false, false> > (start_x, start_y);

			//do the top
			for (int ix = start_x + 1; ix < x_max; ++ix)
			{
				grid_func.operator() <on_edge_template < false, true, false, false>>(ix, start_y);
			}

			//do the top right corner
			grid_func.operator() < on_edge_template < false, true, true, false> > (x_max, start_y);

			//do the center 
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				for (uint32_t ix = start_x + 1; ix < x_max; ++ix)
				{
					//do the center
					grid_func.operator() < on_edge_template< false, false, false, false> > (ix, iy);
				}
			}

			//do the left edge
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				grid_func.operator() < on_edge_template< true, false, false, false> > (start_x, iy);
			}

			//do the right edge 
			for (uint32_t iy = start_y + 1; iy < y_max; ++iy)
			{
				grid_func.operator() < on_edge_template< false, false, true, false> > (x_max, iy);
			}

			//do the bottom left corner 
			grid_func.operator() < on_edge_template< true, false, false, true> > (start_x, y_max);

			//do the bottom 
			for (int ix = start_x + 1; ix < x_max; ++ix)
			{
				grid_func.operator() < on_edge_template< false, false, false, true>>(ix, y_max);
			}

			//do the bottom right corner 
			grid_func.operator() < on_edge_template< false, false, true, true> > (x_max, y_max);

		}
	};
}