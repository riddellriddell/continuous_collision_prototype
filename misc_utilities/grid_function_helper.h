#pragma once
#include <cinttypes>
#include <functional>

//the purpose of this class is to help performing a computation on grid data efficiently 

namespace MiscUtilities
{
	struct grid_function_helper
	{
		enum class edge_enum
		{
			LEFT,
			UP,
			RIGHT,
			DOWN,

			UP_LEFT,
			UP_RIGHT,
			DOWN_LEFT,
			DOWN_RIGHT,

			COUNT
		};

		struct on_edge
		{
			bool left;
			bool up;
			bool right;
			bool down;
			
			template<edge_enum edge>
			bool is_on_edge() const
			{

				if constexpr (edge == edge_enum::LEFT)
				{
					return left;
				}

				if constexpr (edge == edge_enum::UP)
				{
					return up;
				}

				if constexpr (edge == edge_enum::RIGHT)
				{
					return right;
				}

				if constexpr (edge == edge_enum::DOWN)
				{
					return down;
				}

				return false;
			}
			
			template<edge_enum corner>
			bool is_on_corner() const
			{
				if constexpr (corner == edge_enum::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (corner == edge_enum::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (corner == edge_enum::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (corner == edge_enum::DOWN_RIGHT)
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
			template<edge_enum edge>
			static constexpr bool is_edge()
			{
				if constexpr (edge == edge_enum::LEFT)
				{
					return left;
				}

				if constexpr (edge == edge_enum::UP)
				{
					return up;
				}

				if constexpr (edge == edge_enum::RIGHT)
				{
					return right;
				}

				if constexpr (edge == edge_enum::DOWN)
				{
					return down;
				}

				return false;
			}

			template<edge_enum edge>
			static constexpr bool is_edge_direction()
			{
				if constexpr (edge == edge_enum::LEFT)
				{
					return left && !(up || down || right);
				}

				if constexpr (edge == edge_enum::UP)
				{
					return up && !(left || right || down);
				}

				if constexpr (edge == edge_enum::RIGHT)
				{
					return right && !(up || down || left);
				}

				if constexpr (edge == edge_enum::DOWN)
				{
					return down && !(up || left || right);
				}

				if constexpr (edge == edge_enum::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (edge == edge_enum::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (edge == edge_enum::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (edge == edge_enum::DOWN_RIGHT)
				{
					return down && right;
				}

				return false;
			}

			template<edge_enum corner>
			static constexpr bool is_corner()
			{
				if constexpr (corner == edge_enum::UP_LEFT)
				{
					return left && up;
				}

				if constexpr (corner == edge_enum::UP_RIGHT)
				{
					return up && right;
				}

				if constexpr (corner == edge_enum::DOWN_LEFT)
				{
					return down && left;
				}

				if constexpr (corner == edge_enum::DOWN_RIGHT)
				{
					return down && right;
				}

				return false;
			}

			static constexpr bool is_left()
			{
				return is_edge_direction<edge_enum::LEFT>();
			}

			static constexpr bool is_up_left()
			{
				return is_edge_direction<edge_enum::UP_LEFT>();
			}

			static constexpr bool is_up()
			{
				return is_edge_direction<edge_enum::UP>();
			}

			static constexpr bool is_up_right()
			{
				return is_edge_direction<edge_enum::UP_RIGHT>();
			}

			static constexpr bool is_right()
			{
				return is_edge_direction<edge_enum::RIGHT>();
			}

			static constexpr bool is_down_right()
			{
				return is_edge_direction<edge_enum::DOWN_RIGHT>();
			}

			static constexpr bool is_down()
			{
				return is_edge_direction<edge_enum::DOWN>();
			}

			static constexpr bool is_down_left()
			{
				return is_edge_direction<edge_enum::DOWN_LEFT>();
			}

			static constexpr bool is_center()
			{
				return !(up || left || right || down);
			}
		
			static constexpr bool is_on_left_edge()
			{
				return is_edge<edge_enum::LEFT>();
			}

			static constexpr bool is_on_up_edge()
			{
				return is_edge<edge_enum::UP>();
			}

			static constexpr bool is_on_right_edge()
			{
				return is_edge<edge_enum::RIGHT>();
			}

			static constexpr bool is_on_down_edge()
			{
				return is_edge<edge_enum::DOWN>();
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