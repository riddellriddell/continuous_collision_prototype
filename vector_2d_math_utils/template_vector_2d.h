#pragma once

#include <type_traits>
#include <concepts>
#include <limits>

namespace math_2d_util
{


	template<typename T>
	struct template_vector_2d
	{

		static constexpr T max_extent = std::numeric_limits<T>::max(); //smalles value on an axis
		static constexpr T min_extent = std::numeric_limits<T>::lowest(); //larges value on an axis
		static constexpr T center_extent = (min_extent == 0) ? (max_extent / 2) : 0; //center point on an axis 

		T x;
		T y;


		constexpr template_vector_2d():x(0), y(0) {};
		constexpr template_vector_2d<T>(T _x, T _y) :x(_x), y(_y) {};

		//return the largest possible vector 
		constexpr  template_vector_2d<T> max()
		{
			return template_vector_2d<T>(max_extent, max_extent);
		}

		constexpr  template_vector_2d<T> min()
		{
			return template_vector_2d<T>(min_extent, min_extent);
		}

		constexpr  template_vector_2d<T> center()
		{
			return template_vector_2d<T>(center_extent, center_extent);
		}

		//math operations
		template_vector_2d<T> operator-(const template_vector_2d<T>& other) const
		{
			return template_vector_2d<T>(x - other.x, y - other.y);
		}

		template_vector_2d<T> operator+(const template_vector_2d<T>& other) const
		{
			return template_vector_2d<T>(x + other.x, y + other.y);
		}

		// Overload the compound addition operator (+=) to add another vector to this vector
		template_vector_2d<T>& operator+=(const template_vector_2d<T>& other) {
			x += other.x;
			y += other.y;
			return *this; // Return a reference to this vector after the addition
		}

		// Overload the compound subtraction operator (-=) to subtract another vector from this vector
		template_vector_2d<T>& operator-=(const template_vector_2d<T>& other) {
			x -= other.x;
			y -= other.y;
			return *this; // Return a reference to this vector after the subtraction
		}


		// Overload the is equal == opperator to return true if both vectors hold the same values 
		bool operator ==(const template_vector_2d<T>& other) 
		{
			return (x == other.x) && (y == other.y); // Return a reference to this vector after the subtraction
		}

		//conversion overlaods 
		 template<typename TConvertTo>
		 operator TConvertTo() const;
	};
	
	template<typename T>
	template<typename TConvertTo>
	template_vector_2d<T>::operator TConvertTo() const
	{
		TConvertTo converted;

		converted.x = static_cast<decltype(converted.x)>(this.x);
		converted.y = static_cast<decltype(converted.y)>(this.y);
		return converted;
	};

}

