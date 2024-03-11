#pragma once

#include <type_traits>
#include <concepts>
#include <limits>

namespace math_2d_util
{
	template<typename T>
	struct template_vector_2d
	{
		using axis_type = T;
		static constexpr T max_extent = std::numeric_limits<T>::max(); //larges value on an axis
		static constexpr T min_extent = std::numeric_limits<T>::lowest();  //smalles value on an axis
		static constexpr T center_extent = (min_extent == 0) ? (max_extent / 2) : 0; //center point on an axis 

		T x;
		T y;


		constexpr template_vector_2d<T>() = default;
		constexpr template_vector_2d<T>(T _val) :x(_val), y(_val) {};
		constexpr template_vector_2d<T>(T _x, T _y) :x(_x), y(_y) {};

		//return the largest possible vector 
		static constexpr  template_vector_2d<T> max()
		{
			return template_vector_2d<T>(max_extent, max_extent);
		}

		static constexpr  template_vector_2d<T> min()
		{
			return template_vector_2d<T>(min_extent, min_extent);
		}

		static constexpr  template_vector_2d<T> center()
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


		template_vector_2d<T> operator*(const template_vector_2d<T>& other) const
		{
			return template_vector_2d<T>(x * other.x, y * other.y);
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

		// Overload the compound subtraction operator (-=) to subtract another vector from this vector
		template_vector_2d<T>& operator*=(const template_vector_2d<T>& other) {
			x *= other.x;
			y *= other.y;
			return *this; // Return a reference to this vector after the subtraction
		}

		template_vector_2d<T> operator-(const auto other) const
		{
			return template_vector_2d<T>(x - other, y - other);
		}

		template_vector_2d<T> operator+(const auto other) const
		{
			return template_vector_2d<T>(x - other, y - other);
		}

		template_vector_2d<T> operator*(const auto other) const
		{
			return template_vector_2d<T>(x * other, y * other);
		}

		// Overload the compound addition operator (+=) to add another vector to this vector
		template_vector_2d<T>& operator+=(const auto other) {
			x += other;
			y += other;
			return *this; // Return a reference to this vector after the addition
		}

		// Overload the compound subtraction operator (-=) to subtract another vector from this vector
		template_vector_2d<T>& operator-=(const auto other) {
			x -= other;
			y -= other;
			return *this; // Return a reference to this vector after the subtraction
		}

		// Overload the compound subtraction operator (-=) to subtract another vector from this vector
		template_vector_2d<T>& operator*=(const auto other) {
			x *= other;
			y *= other;
			return *this; // Return a reference to this vector after the subtraction
		}

		template<typename TTarget_type>
		TTarget_type convert_to() const
		{
			TTarget_type output = {};

			output.x = static_cast<decltype(output.x)>(x);
			output.y = static_cast<decltype(output.y)>(y);

			return output;
		};


		// Overload the is equal == opperator to return true if both vectors hold the same values 
		bool constexpr operator == (const template_vector_2d<T>& other) const
		{
			return (x == other.x) && (y == other.y); // Return a reference to this vector after the subtraction
		}


		////conversion overlaods 
		template<typename TConvertTo>
		explicit operator TConvertTo() const;

		constexpr  T lenght_sqr()
		{
			return (x * x) + (y * y);
		}

	};
	
	template<typename T>
	template<typename TConvertTo>
	template_vector_2d<T>::operator TConvertTo() const
	{
		TConvertTo converted;
	
		converted.x = static_cast<decltype(converted.x)>(this->x);
		converted.y = static_cast<decltype(converted.y)>(this->y);
		return converted;
	};

}

