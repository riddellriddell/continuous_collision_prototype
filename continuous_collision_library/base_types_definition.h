#pragma once
#include <stdint.h>

namespace ContinuousCollisionLibrary
{
	using uint64 = uint64_t;
	using int64 = int64_t;
	using uint32 = uint32_t;
	using int32 = int32_t;
	using uint16 = uint16_t;
	using int16 = int16_t;
	using uint8 = uint8_t;

	enum class loose_grid_node_index :uint32 {};

	//index handle for a loose grid element
	enum class tight_grid_node_index :uint32 {};

	enum class tight_grid_element_index :uint32 {};
	
	enum class external_element_handle :uint32 {};
}