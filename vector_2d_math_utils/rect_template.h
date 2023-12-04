#pragma once

#include <algorithm>
#include "template_vector_2d.h"
//template structure for making rects

namespace math_2d_util
{
	template<typename T>
	struct template_rect_2d
	{
		//the underlying type for the min and max values 
		using number_type = T;
		using vector_type = template_vector_2d<T>;

		//xy coordinates for min and max bounds
		template_vector_2d<T> min;
		template_vector_2d<T> max;

		//helper values
		static constexpr template_rect_2d<T> max_size_rect()
		{
			return { template_vector_2d<T>::min(), template_vector_2d<T>::max() };
		}

		//rect turned inside out, this can be usefull if you want to stop a rect from returning true from bounds comparisons
		static constexpr template_rect_2d<T> inverse_max_size_rect()
		{
			return{ template_vector_2d<T>::max(), template_vector_2d<T>::min() };
		}

		constexpr template_rect_2d() = default;
		constexpr template_rect_2d(T min_x, T min_y, T max_x, T max_y):min(min_x, min_y), max(max_x, max_y) {};
		constexpr template_rect_2d(const template_vector_2d<T> &_min, const template_vector_2d<T> &_max) :min(_min), max(_max) {};

		//returns a rect where both the min and max are at the center tile
		//this rect is considered zero sized as min is inclusive and max is exclusive 
		static constexpr template_rect_2d center_rect()
		{
			return template_rect_2d<T>(vector_type::center(), vector_type::center());
		}
	};

	struct rect_2d_math
	{
		//check if the two rects overlap
		//this is a "half open interval" where min is inclusive but max is exclusive and the values from min to max -epsilon / 1 are considered inside the rect 
		template<typename T>
		static bool is_overlapping(const template_rect_2d<T>& rect_a, const template_rect_2d<T>& rect_b);

		//create a new rect offset by the given vector 
		template<typename Treturn_type, typename Trect, typename Toffset>
		static Treturn_type get_offset_rect_as(const Trect& rect_to_offset, const Toffset& offset_by);

		//get the top left corner of the overlapping region between two rects
		template<typename T>
		static math_2d_util::template_vector_2d<T> get_top_left_corner_of_overlap(const template_rect_2d<T>& rect_a, const template_rect_2d<T>& rect_b);
	};

	template<typename T>
	inline bool rect_2d_math::is_overlapping(const template_rect_2d<T>& rect_a, const template_rect_2d<T>& rect_b)
	{
		//min max
		bool in_right = rect_a.min.x < rect_b.max.x;
		bool in_bottom = rect_a.min.y < rect_b.max.y;
		bool in_left = rect_a.max.x > rect_b.min.x;
		bool in_top = rect_a.max.y > rect_b.min.y;


		return (in_right & in_bottom & in_left & in_top);
	};

	template<typename Treturn_type, typename Trect, typename Toffset>
	inline Treturn_type rect_2d_math::get_offset_rect_as(const Trect& rect_to_offset, const Toffset& offset_by)
	{
		Treturn_type offset_rect;

		offset_rect.min.x = rect_to_offset.min.x + offset_by.x;
		offset_rect.max.x = rect_to_offset.max.x + offset_by.x;

		offset_rect.min.y = rect_to_offset.min.y + offset_by.y;
		offset_rect.max.y = rect_to_offset.max.y + offset_by.y;

		return offset_rect;
	};

	template<typename T>
	inline math_2d_util::template_vector_2d<T> rect_2d_math::get_top_left_corner_of_overlap(const template_rect_2d<T>& rect_a, const template_rect_2d<T>& rect_b)
	{
		assert(is_overlapping(rect_a, rect_b), "Attempting to get overlap for non overlapping rects");

		math_2d_util::template_vector_2d<T> top_left_corner;

		top_left_corner.x = std::max(std::min(rect_a.max.x, rect_b.min.x), rect_a.min.x);
		top_left_corner.y = std::max(std::min(rect_a.max.y, rect_b.min.y), rect_a.min.y);

		return top_left_corner;
	}
}