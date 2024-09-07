#pragma once
#define NOMINMAX
#include <Windows.h>
#include <vector>
#include < functional >
#include "vector_2d_math_utils/vector_types.h"

class debug_draw_interface
{
public:
	//type for colour
	using colour_type = COLORREF;
private:
	//colour array 
	std::vector<colour_type> screen_pixles;
	
	uint32_t screen_width;
	uint32_t screen_height;

	math_2d_util::fvec2d view_offset = {};

	float view_zoom = 1;
	
public:

	static constexpr COLORREF to_colour(uint8_t r, uint8_t g, uint8_t b)
	{
		return RGB(r,g,b);
	}

	uint32_t get_width() const { return screen_width; };
	uint32_t get_height() const { return screen_height; };

	math_2d_util::fvec2d get_top_left() const 
	{
		return apply_inverse_view_offset(math_2d_util::fvec2d(0,0));
	}

	math_2d_util::fvec2d get_bottom_right() const
	{
		return apply_inverse_view_offset(math_2d_util::fvec2d(get_width(), get_height()));
	}


	void add_offset(math_2d_util::fvec2d offset);

	void add_zoom(float zoom);

	float get_zoom()
	{
		return view_zoom;
	}

	colour_type* get_data_ptr() { return screen_pixles.data(); };

	//resize 
	void resize(uint32_t new_width, uint32_t new_height);
	
	//clear 
	void clear_to(colour_type colour);
	
	//draw line
	void draw_line_from_to(math_2d_util::fvec2d from, math_2d_util::fvec2d to, colour_type colour);
	

	//draw box 
	void draw_box(math_2d_util::fvec2d min, math_2d_util::fvec2d max, colour_type colour);

	//draw circle 
	void draw_circle(math_2d_util::fvec2d center, float radius, colour_type colour);

	void draw_grid(math_2d_util::fvec2d min, math_2d_util::ivec2d axis_count, float cell_size, colour_type colour);

	void draw_screen_space_line(math_2d_util::ivec2d from, math_2d_util::ivec2d to, colour_type colour);

	void draw_dotted_screen_space_line(math_2d_util::ivec2d from, math_2d_util::ivec2d to, colour_type colour, uint32_t pixels_per_dot = 5, uint32_t pixels_per_space = 0);

	void draw_screen_space_box(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour);

	void draw_screen_space_box_outline(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour);

	void draw_screen_space_circle(math_2d_util::ivec2d center, int32_t radius, colour_type colour);

	void draw_sceen_space_circle_outline(math_2d_util::ivec2d center, int32_t radius, colour_type colour);


private:

	math_2d_util::fvec2d apply_view_offset(math_2d_util::fvec2d point) const;

	math_2d_util::fvec2d apply_inverse_view_offset(math_2d_util::fvec2d screen_space_point) const;

	void draw_screen_space_box_outline_internal(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour,std::function<void(math_2d_util::ivec2d, math_2d_util::ivec2d, colour_type colour)> line_draw_func);

	void draw_sceen_space_grid_outline_internal(math_2d_util::ivec2d min, math_2d_util::ivec2d axis_count, float cell_size, colour_type colour, std::function<void(math_2d_util::ivec2d, math_2d_util::ivec2d, colour_type colour)> line_draw_func);

};	

