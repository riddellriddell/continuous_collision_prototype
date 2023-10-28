#pragma once
#include <concepts>
namespace math_2d_util
{

	//use to make sure template type has an x and y 
	template <typename TConTarget>
	concept has_x_and_y_concept = requires(TConTarget t) {
		{ t.x } ->  std::same_as<decltype(t.x)>;
		{ t.y } -> std::same_as<decltype(t.y)>;
	};
}