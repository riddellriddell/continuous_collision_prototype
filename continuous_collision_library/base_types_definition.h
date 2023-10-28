#pragma once

namespace ContinuousCollisionLibrary
{
	using uint64 = unsigned long;
	using int64 =  long;
	using uint32 = unsigned int;
	using int32 = int;
	using uint16 = unsigned short;
	using int16 = short;
	using uint8 = unsigned char;

	enum class loose_grid_node_index :uint32 {};

	//index handle for a loose grid element
	enum class tight_grid_node_index :uint32 {};

	enum class tight_grid_element_index :uint32 {};
	
	enum class external_element_handle :uint32 {};
}