#pragma once
#include "sector_grid_data_structure/sector_grid.h"
#include "sector_grid_data_structure/sector_grid_index.h"
#include "base_types_definition.h"
#include "array_utilities/WideNodeLinkedList/wide_node_linked_list.h"
#include "vector_2d_math_utils/vector_types.h"
#include "vector_2d_math_utils/rect_types.h"
#include "base_types_definition.h"
#include "vector_2d_math_utils/byte_vector_2d.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <bit>



namespace ContinuousCollisionLibrary
{

#pragma region OverlapFlagDef
	//------------------- Overlap Falg Definition ----------------------------------

	//bitflag array where each bit represents the relative position of a tile that overlaps this tile
	template<typename TFlagDataType, int IoverlapRegionWidth>
	struct overlap_flag_template
	{

		static constexpr uint32 width = IoverlapRegionWidth; //number of possible values on an axis
		static constexpr uint32 axis_max = width -1; //max value that can be stored per axis (1 less than width due to 0)
		static constexpr uint32 axis_center = axis_max / 2; //center point for an axis 

		//what is the maximum number of overlaps for this tile including itself 
		static constexpr uint32 max_overlaps = IoverlapRegionWidth * IoverlapRegionWidth;

		static constexpr math_2d_util::uivec2d max() { return math_2d_util::uivec2d{ axis_max ,axis_max }; };
		static constexpr math_2d_util::uivec2d center() { return math_2d_util::uivec2d{ axis_center,axis_center }; };

		TFlagDataType overlap_flag;

		//convert from a flag to an offset starting at the top left corner of the offset window
		static constexpr math_2d_util::uivec2d calcualte_offset_for_flag_index(uint32 flag_index)
		{
			//extract the offset 
			uint32 y_offset = flag_index / width;
			uint32 x_offset = flag_index - (y_offset * width);

			return math_2d_util::uivec2d{ x_offset , y_offset };
		}


		//helpter to itterate over all set bits in overlap flags
		struct iterator
		{
			TFlagDataType overlap_flag_copy;

			uint32 current_flag_index;

			static constexpr uint32 end = sizeof(TFlagDataType) * 8;

			// Define the iterator dereference operator.
			int operator*() const {
				return current_flag_index;
			}

			void next()
			{
				current_flag_index = std::countr_zero(overlap_flag_copy);
				
				//clear the bit so it does not get returned next time
				overlap_flag_copy ^= (uint64(1) << static_cast<uint64>(current_flag_index));
			}

			// Define the iterator pre-increment operator.
			iterator& operator++() {
				next();
				return *this;
			}

			// Define the iterator equality operators.
			bool operator==(const iterator& other) const {
				return current_flag_index == other.current_flag_index;
			}

			bool operator!=(const iterator& other) const {
				return current_flag_index != other.current_flag_index;
			}

			iterator(TFlagDataType overlap_flag) :overlap_flag_copy(overlap_flag), current_flag_index(end)
			{ 
				next();
			}

			//end index itterator
			iterator() :overlap_flag_copy(0), current_flag_index(end) {}
			
		};

		// Define begin and end functions to create iterators.
		iterator begin() const
		{
			return iterator(overlap_flag); //initalise itterator with bits, 
		}

		iterator end() const {
			return iterator(); // End iterator has no valid bit index.
		}

		//operators to combine flags
		overlap_flag_template<TFlagDataType, IoverlapRegionWidth> operator | (const overlap_flag_template<TFlagDataType, IoverlapRegionWidth>& other) const
		{
			return overlap_flag_template<TFlagDataType, IoverlapRegionWidth>{ overlap_flag | other.overlap_flag};
		}


		overlap_flag_template<TFlagDataType, IoverlapRegionWidth>& operator|=(const overlap_flag_template<TFlagDataType, IoverlapRegionWidth>& other) {
			overlap_flag |= other.overlap_flag;
			return *this;
		}

		//check if has same flag bits set 
		bool has_flags(overlap_flag_template<TFlagDataType, IoverlapRegionWidth> &flags_to_check_for)
		{
			return overlap_flag& flags_to_check_for.overlap_flag;
		}

		constexpr overlap_flag_template() = default;

		constexpr overlap_flag_template(TFlagDataType flag):overlap_flag(flag){};

		constexpr overlap_flag_template(math_2d_util::uivec2d& offset_to_encode)
		{
			//sanity check that the offsets are not larger than the largest possible flag width
			assert(offset_to_encode.x < width&& offset_to_encode.y < width, "Offset larget than allowed values");

			uint64 flag_offset = offset_to_encode.x + (width * offset_to_encode.y);

			assert(flag_offset < max_overlaps, "the uint 64 represeing all the tiles overlapping this tile can only store max_overlaps values staring at offset 0");

			auto flag = 1ull << flag_offset;

			overlap_flag = flag;
		}  
	};

#pragma endregion

#pragma region TypeDef

	using tile_local_bounds = math_2d_util::ubrect;

	using grid_dimensions = SectorGrid::sector_grid_dimensions<16, 16>;
	
	using overlap_flags = overlap_flag_template<uint64, 7>;

	static constexpr uint32 tile_overlap_max_width = 7;

	//the max posible representable range of overlaps
	//todo caluclate this at compile time using numeric limits and compile time sqrt 
	//static constexpr uint32 tile_overlap_rep_max = std::sqrt( std::numeric_limits<uint8>::max());
	static constexpr uint32 tile_overlap_rep_max = 16;

	static constexpr uint32 tile_overlap_half_rep_max = tile_overlap_rep_max /2;

	static constexpr uint32 tile_overlap_quater_rep_max = tile_overlap_rep_max / 4;


	//if you subtract this from a tile it will return the top left corner for the maximum rect size of a tile
	static constexpr math_2d_util::uivec2d tile_overlap_half_width_offset = { tile_overlap_max_width / 2, tile_overlap_max_width / 2 };

	static constexpr math_2d_util::uivec2d tile_overlap_offset = { tile_overlap_max_width, tile_overlap_max_width };

	static constexpr math_2d_util::uivec2d offset_for_0_to_7_vec = { tile_overlap_quater_rep_max, tile_overlap_quater_rep_max };



	static constexpr uint32 max_tile_overlaps_per_tile = tile_overlap_max_width * tile_overlap_max_width;

	static constexpr uint32 max_overlap_pairs_per_tile = (tile_overlap_max_width + (tile_overlap_max_width -1)) * (tile_overlap_max_width + (tile_overlap_max_width - 1));

	static constexpr uint32 max_overlap_pairs_per_sector = max_overlap_pairs_per_tile * grid_dimensions::sector_tile_count;

	static constexpr uint32 wide_node_linked_list_width = 8;

	using sector_overlap_list = ArrayUtilities::wide_node_linked_list<grid_dimensions::sector_tile_count, uint16, max_overlap_pairs_per_sector, wide_node_linked_list_width, math_2d_util::byte_vector_2d>;

#pragma endregion
 
	struct overlap_tracking_grid
	{

		//constructor 
		overlap_tracking_grid();
		
		//setup the initial data structures
		void initialize();
		
		//make bounds at index larget
		void update_bounds();
		
		//calculates the bitflag for a tile relative to another tile
		overlap_flags calculate_flag_for_tile(const math_2d_util::uivec2d & tile_to_create_flag_for, const math_2d_util::uivec2d & target_tile) const;

		//add flag to all tiles in rect
		void add_flag_to_tiles(
			math_2d_util::uivec2d& source_tile_cord, 
			const math_2d_util::uirect& add_to_area, 
			const math_2d_util::uirect& old_bounds, 
			const math_2d_util::uirect& new_bounds);
		
		//remove flag from all tiles in rect 
		void remove_flag_from_tiles(
			math_2d_util::uivec2d& source_tile_cord, 
			const math_2d_util::uirect& remove_area, 
			const math_2d_util::uirect& old_bounds, 
			const math_2d_util::uirect& new_bounds);
		
		//check if point is inside grid
		bool is_point_in_grid(const math_2d_util::uivec2d& point);
		
		bool is_rect_in_grid(const math_2d_util::uirect& rect);

		ContinuousCollisionLibrary::uint32 get_affinity_for_offset(const math_2d_util::uivec2d& offset) const;

		//helper for calculating sector grid indexes
		SectorGrid::sector_grid_helper<grid_dimensions> grid_helper;
		
		//structure holding the overlap flags for all tiles
		SectorGrid::template_sector_grid<overlap_flags, grid_dimensions> overlaps;

		//structure holding the bounds of a tile 
		SectorGrid::template_sector_grid<tile_local_bounds, grid_dimensions> bounds;
		
		//structure holding a list of all the intersecting tiles per sector
		std::array<sector_overlap_list, grid_dimensions::sector_grid_count> overlap_pairs;
	};

};

