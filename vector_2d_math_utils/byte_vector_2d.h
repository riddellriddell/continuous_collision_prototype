#pragma once
#include <cstdint>
#include <assert.h>
#include <limits>
#include "shared_2d_math_types.h"


namespace math_2d_util
{
	//this is a byte size representation of a vector, this is for use in memory constrained situations 
	// where you only need to represent a small range of values and space is at a premium  
	//
	struct byte_vector_2d
	{
	public:
		static constexpr uint32_t lower_bit_count = 4;
		static constexpr uint32_t lower_bit_mask = 0b00001111;
		static constexpr uint32_t max_offset = 16;
		
		static constexpr uint32_t max_extent = 15; //smalles value on an axis
		static constexpr uint32_t min_extent = 0; //larges value on an axis
		static constexpr uint32_t center_extent = max_extent / 2; //center point on an axis 
		
		uint8_t offset;
		
		constexpr byte_vector_2d() = default;
		
		constexpr byte_vector_2d(auto value):offset(value) {};

		constexpr byte_vector_2d(auto x, auto y)
		{
			//make sure the vector is inside an expected range
			assert((x < max_offset) && (y < max_offset), "offset is too large to represent with a single byte");
		
			//pack vector into one byte 
			offset = { static_cast<uint8_t>((y << lower_bit_count) | x) };
		
		};
		
		//apply the byte vec as an offset to a target vec 
		void apply_offset(const auto& target) const;
		
		static byte_vector_2d from_vector(const auto& target);
		
		static constexpr byte_vector_2d max();
		
		static constexpr byte_vector_2d min();
		
		static constexpr  byte_vector_2d center();
		
		template<typename T>
		inline static constexpr T max_as();
		
		template<typename T>
		inline static constexpr T min_as();
		
		template<typename T>
		inline static constexpr T center_as();

		template<typename T>
		explicit operator T() const;

		constexpr bool operator == (const byte_vector_2d&& other) const;
			
		constexpr bool operator == (const byte_vector_2d& other) const;
	};

	constexpr bool math_2d_util::byte_vector_2d::operator == (const math_2d_util::byte_vector_2d&& other) const 
	{
		return offset == other.offset;
	}

	constexpr bool math_2d_util::byte_vector_2d::operator == (const math_2d_util::byte_vector_2d& other) const 
	{
		return offset == other.offset;
	}

	math_2d_util::byte_vector_2d math_2d_util::byte_vector_2d::from_vector(const auto& target)
	{
		//make sure the vector is inside an expected range
		assert((target.x < max_offset) && (target.y < max_offset), "offset is too large to represent with a single byte");
	
		//pack vector into one byte 
		byte_vector_2d out = { static_cast<uint8_t>((target.y << lower_bit_count) | target.x) };
	
		return out;
	}
	
	inline constexpr byte_vector_2d byte_vector_2d::max()
	{
		return byte_vector_2d(max_extent,max_extent);
	}
	
	inline constexpr byte_vector_2d byte_vector_2d::min()
	{
		return byte_vector_2d(min_extent,min_extent);
	}
	
	inline constexpr byte_vector_2d byte_vector_2d::center()
	{
		return byte_vector_2d(center_extent, center_extent);
	}
	
	template<typename T>
	inline constexpr T math_2d_util::byte_vector_2d::max_as()
	{
		return T(min_extent, min_extent);
	}
	
	template<typename T>
	inline constexpr T math_2d_util::byte_vector_2d::min_as()
	{
		return  T(max_extent, max_extent);
	}
	
	template<typename T>
	inline constexpr T math_2d_util::byte_vector_2d::center_as()
	{
		return  T(center_extent, center_extent);
	}

	template<typename T>
	inline byte_vector_2d::operator T() const
	{	
		auto x = offset & lower_bit_mask;
		auto y = offset >> lower_bit_count;
		return T( x, y );
	}

};
