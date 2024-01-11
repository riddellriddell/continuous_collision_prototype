#pragma once
#include <cinttypes>

//utility to get the number of bits needed to store a number 
namespace MiscUtilities
{
	constexpr uint32_t bits_needed_to_represent_number(size_t number)
	{
		return number > 0
			? 1 + bits_needed_to_represent_number(number / 2)
			: 0;
	}
}