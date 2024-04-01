#pragma once
#include <array>
#include <algorithm>
#include <tuple>

#include "vector_2d_math_utils/rect_types.h"
#include "vector_2d_math_utils/vector_types.h"

#include "base_types_definition.h"

#include "misc_utilities/int_type_selection.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"
#include "misc_utilities/grid_function_helper.h"
#include "misc_utilities/grid_utilities.h"

#include "continuous_collision_library/overlap_tracking_grid.h"
#include "array_utilities/fixed_free_list.h"
#include "array_utilities/paged_2d_array.h"
#include "array_utilities/handle_tracked_2d_paged_array.h"
#include "array_utilities/HandleSystem/handle_system.h"
#include "array_utilities/fixed_size_vector.h"
#include "array_utilities/paged_wide_node_linked_list.h"

#include "sector_grid_data_structure/sector_grid.h"
#include "sector_grid_data_structure/sector_grid_dimensions.h"
#include "sector_grid_data_structure/sector_grid_index.h"
#include "DebugWindowsDrawHelper/debug_draw_interface.h"


namespace ContinuousCollisionLibrary
{
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	class phyisics_2d_main
	{
		using phyisics_2d_main_type = phyisics_2d_main<Imax_objects, Iworld_sector_x_count>;
		
		//definition of the target dimensions of the grid system
		using grid_dimension_type = SectorGrid::sector_grid_dimensions<Iworld_sector_x_count, 16>;

		using tile_coordinate_type = SectorGrid::sector_tile_index<grid_dimension_type>;

		//use the sector tile helper to convert to sector
		using sector_grid_helper_type = SectorGrid::sector_grid_helper<grid_dimension_type>;

		//sector grid instance this currently holds nothing but am thinking about making sizing at load not compile 
		sector_grid_helper_type grid_helper;

		static constexpr size_t page_size_for_collider_data = 1024;

		static constexpr size_t max_sectors_internal = grid_dimension_type::sector_grid_count_max;

		using sector_count_type = MiscUtilities::uint_s<max_sectors_internal>::int_type_t;

		static constexpr sector_count_type sector_count = max_sectors_internal;

		static constexpr size_t bits_needed_to_store_handle_count = MiscUtilities::bits_needed_to_represent_number(Imax_objects);
		
		float time_step = 1.0f / 60.0f;

	public:
		//temp handle type definition
		using handle_type = HandleSystem::default_handle_type<Imax_objects>;

	private:

		using handle_data_lookup_system_type = ArrayUtilities::fixed_free_list<Imax_objects, handle_type>;

		//the system to manage all the handles and data lookup addresses 
		handle_data_lookup_system_type handle_manager;

		struct collision_data_ref
		{
			float& x;
			float& y;
			
			float& velocity_x;
			float& velocity_y;
						
			float& radius;


			auto get_as_tuple()
			{
				return std::tie(x, y, velocity_x, velocity_y, radius);
			}

			//auto get_as_tuple()
			//{
			//	return std::tie(x);
			//}
		};

		//static_assert(sector_count == 256);
		//static_assert(Imax_objects == 254);
		//static_assert(page_size_for_collider_data == 1024);

		//data for all the colliders that is tied to / tracked with unique handle id's from the handle manager
		using collision_data_container_type = ArrayUtilities::handle_tracked_2d_paged_array<handle_type, sector_count, Imax_objects, Imax_objects, page_size_for_collider_data, collision_data_ref>;

		//data for all the items in the grid
		collision_data_container_type collision_data_container;

	public:

		//buffer for new data to add to the grid
		struct new_collider_data
		{
			friend class phyisics_2d_main_type;

		private:

			//this value is set on add to the add buffer 
			handle_type owner;

		public:
			math_2d_util::fvec2d position;

			math_2d_util::fvec2d velocity;

			float radius;

			auto get_as_tuple()
			{
				return std::tie(position.x, position.y, velocity.x, velocity.y, radius);
			}

			new_collider_data() {}

		private:
			new_collider_data(handle_type _owner, math_2d_util::fvec2d _position, math_2d_util::fvec2d _velocity, float _radius) :owner(_owner), position(_position), velocity(_velocity), radius(_radius)
			{

			}
		};

	private:


		//buffer to hold all data being added to the grid 
		using new_collider_header_type = ArrayUtilities::paged_2d_array_header < sector_count, Imax_objects, Imax_objects, page_size_for_collider_data>;

		using new_collider_data_containter_type = std::array<new_collider_data, new_collider_header_type::max_total_entries>;

		new_collider_header_type collider_to_add_header;
		new_collider_data_containter_type colliders_to_add_data;
		
		//which sectors have items queued to be added into the game
		ArrayUtilities::fixed_size_vector_array<sector_count_type, max_sectors_internal> sectors_with_queued_items;

		//sector_count_type number_of_active_sectors;
		//std::array<sector_count_type, max_sectors_internal> active_sectors;

		//grid tracking collisions 
		overlap_tracking_grid overlap_grid;

		//assuming average tile will have at max 6? 
		static constexpr size_t node_width = 8;

		//assuming half of the tiles in each sector are full on average
		static constexpr size_t page_size = (grid_dimension_type::sector_tile_count / 2) ;

		//list of all the agents in each tile in each sector
		using per_tile_collider_list_type = ArrayUtilities::paged_wide_node_linked_list<handle_type, grid_dimension_type::tile_count, node_width, Imax_objects, Imax_objects, Imax_objects, page_size, grid_dimension_type::sector_tile_count>;

		//agent lookup
		per_tile_collider_list_type colliders_in_tile_tracker;


		//transfer buffer types for each sector
		enum class transfer_buffer_types : uint8_t
		{
			LEFT,
			UP,
			RIGHT,
			DOWN,
			OTHER, //this included diagonal
			COUNT
		};

		//5 buffers for each sector, on for each cardinal direction + an everything else buffer
		static constexpr uint32_t number_of_transfer_buffers = grid_dimension_type::sector_grid_count * static_cast<uint32_t>(transfer_buffer_types::COUNT);

		//node width, assuming average unit is the size of a tile the worst case for a single frame is 16 units  
		static constexpr uint32_t transfer_buffer_size = grid_dimension_type::sector_w;

		//transfer buffers per sector
		struct sector_transfer_buffers
		{
		private:
			std::array< ArrayUtilities::fixed_size_vector_array<new_collider_data, transfer_buffer_size>, static_cast<uint32_t>(transfer_buffer_types::COUNT)> transfer_buffers;

		public:

			//maximum number of object that can be transfered into a single sector normally
			static constexpr uint32_t max_item_transfer = static_cast<uint32_t>(transfer_buffer_types::COUNT) * transfer_buffer_size;

			template<transfer_buffer_types buffer>
			ArrayUtilities::fixed_size_vector_array<new_collider_data, transfer_buffer_size>& get_ref_to_buffer()
			{
				//make sure the buffer is in the expected range 
				static_assert(buffer < transfer_buffer_types::COUNT);

				return transfer_buffers[static_cast<uint32_t>(buffer)];
			}

			ArrayUtilities::fixed_size_vector_array<new_collider_data, transfer_buffer_size>& get_ref_to_buffer(transfer_buffer_types buffer)
			{
				//make sure the buffer is in the expected range 
				assert(buffer < transfer_buffer_types::COUNT);

				return transfer_buffers[static_cast<uint32_t>(buffer)];
			}

			template<transfer_buffer_types buffer>
			uint32_t get_buffer_size()
			{
				return get_ref_to_buffer<buffer>().size();
			}

			ArrayUtilities::fixed_size_vector_array<new_collider_data, transfer_buffer_size>& get_buffer_for_transfer(math_2d_util::ivec2d& from, math_2d_util::ivec2d& to)
			{
				auto diff = to - from;

				//if we are traveling vertical
				bool vertical = diff.y;

				bool diagnal = diff.x && vertical;

				uint32_t combined = (diff.x | diff.y) + 1;

				//combine them togeather 
				uint32_t cardinal_index = combined + vertical;

				//other detect
				uint32_t cardinal_or_other_index = diagnal ? combined : static_cast<uint32_t>(transfer_buffer_types::COUNT);

				assert(cardinal_or_other_index < static_cast<uint32_t>(transfer_buffer_types::COUNT));

				return transfer_buffers[cardinal_or_other_index];
			}

			void clear()
			{
				std::for_each(transfer_buffers.begin(), transfer_buffers.end(), [&](auto& buffer_to_clear)
					{
						buffer_to_clear.clear();
					});
			}
		};

		//temp buffer for moving objects from one sector to another 
		std::array<sector_transfer_buffers, grid_dimension_type::sector_grid_count> sector_transfer_buffer_groups;

		//temp buffer 
		std::array<ArrayUtilities::fixed_size_vector_array<typename collision_data_container_type::real_address_type, sector_transfer_buffers::max_item_transfer >, grid_dimension_type::sector_grid_count> sector_transfer_removal_address_groups;

		public:

		//try add a handle to the simulation
		handle_type try_queue_item_to_add( new_collider_data&& data_for_new_collider);

		private:

		//add all the queued items for a sector into the physics system 
		void add_items_from_sector(sector_count_type sector_index);

		void add_collider_data_to_sector_data(const new_collider_data& data_to_add, sector_count_type sector_index);

		void add_collider_handle_to_tile_tracker(const new_collider_data& data_to_add);

		//add all the new items 
		void add_items_from_all_sectors();

		//update the grid overlap system
		void update_bounds_in_sector(sector_count_type sector_index);

		//update the bounds in all sectors 
		void update_bounds_in_all_sectors();

		//move all objects in sector 
		void update_positions_in_sector(sector_count_type sector_index);

		//copy all objects that have left the sector to the sector transfer buffer
		template<typename Tedge_info>
		void transfer_items_between_sectors(sector_count_type sector_index);

		//move all objects in the world
		void update_all_positions();

		public:
		//add queued items 
		void update_physics();

		//debug draw tool
		void draw_debug(debug_draw_interface& draw_interface);

	};


	
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::handle_type 
		phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::try_queue_item_to_add(new_collider_data&& data_for_new_collider)
	{
		//try and get a free handle 
		typename handle_data_lookup_system_type::index_type index = handle_manager.get_free_element();

		//check that index is valid 
		if (!handle_data_lookup_system_type::is_valid_index(index))
		{
			//if the handle is invalid dont queue data and just return an invalid handle
			return handle_type::get_invalid_index();
		}

		//create the handle 
		handle_type handle = handle_type(index);

		//convert from position to sector and tile 

		//convert to tile
		math_2d_util::ivec2d tile_xy = static_cast<math_2d_util::ivec2d>(data_for_new_collider.position);

		//use grid helper to get sector
		uint32 sector_index = grid_helper.to_sector_index(tile_xy);

		//check if this is the first item in this sector
		bool is_first_in_sector = !collider_to_add_header.y_axis_count[sector_index];

		//if this is the first in the sector add to the active sector list
		sectors_with_queued_items.push_back(sector_index, is_first_in_sector);

		//alocate space in the in buffer for the data 
		auto address_to_add_item_at = collider_to_add_header.push_back(sector_index);

		//store the data about the new item to add 
		colliders_to_add_data[address_to_add_item_at.address] = std::move(data_for_new_collider);

		//update the handle 
		colliders_to_add_data[address_to_add_item_at.address].owner = handle;

		return handle;
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_items_from_sector(sector_count_type sector_index)
	{
		//get index iterator from the item add header 
		std::for_each(collider_to_add_header.begin(sector_index), collider_to_add_header.end(sector_index), [&](auto real_address_to_add)
			{
				//get a ref struct to the existing data
				const new_collider_data& existing_data = colliders_to_add_data[real_address_to_add.address];

				add_collider_data_to_sector_data(existing_data, sector_index);

			});

		//inject the handle into the per tile trakcer
		std::for_each(collider_to_add_header.begin(sector_index), collider_to_add_header.end(sector_index), [&](auto real_address_to_add)
			{
				//get a ref struct to the existing data
				const new_collider_data& existing_data = colliders_to_add_data[real_address_to_add.address];

				add_collider_handle_to_tile_tracker(existing_data);
			});

		//clear all queued items for sector
		collider_to_add_header.clear_axis(sector_index);
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_collider_data_to_sector_data(const new_collider_data& data_for_new_collider, sector_count_type sector_index)
	{
		//add to the target sector 
		auto [destination__real_address, _, __] = collision_data_container.insert(data_for_new_collider.owner, sector_index);

		//get the ref struct to the data we are writing to
		auto data_ref = collision_data_container.get(destination__real_address);

		//copy across the values (could maybe make a meta function for this)
		data_ref.x = data_for_new_collider.position.x;
		data_ref.y = data_for_new_collider.position.y;

		data_ref.velocity_x = data_for_new_collider.velocity.x;
		data_ref.velocity_y = data_for_new_collider.velocity.y;

		data_ref.radius = data_for_new_collider.radius;
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_collider_handle_to_tile_tracker(const new_collider_data& data_for_new_collider)
	{
		//get the handle 
		auto handle = data_for_new_collider.owner;

		//convert to tile
		math_2d_util::ivec2d tile_xy = static_cast<math_2d_util::ivec2d>(data_for_new_collider.position);

		//use grid helper to get sector
		auto sector_index = grid_helper.from_xy(tile_xy);

		//inject the handle into the grid structure 
		colliders_in_tile_tracker.add(sector_index.index, handle);
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::add_items_from_all_sectors()
	{
		//single threaded version.
		//loop through all sectors and add their items into the simulation
		std::for_each(sectors_with_queued_items.begin(), sectors_with_queued_items.end(), [&](auto sector_index)
			{
				add_items_from_sector(sector_index);
			});

		sectors_with_queued_items.clear();
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_bounds_in_sector(sector_count_type sector_index)
	{
		//get the iterators for the sector
		auto begin_itr = colliders_in_tile_tracker.get_active_nodes_in_group_start(sector_index);
		auto end_itr = colliders_in_tile_tracker.get_active_nodes_in_group_end(sector_index);

		//sector offset 
		auto tile_offset = sector_index * grid_dimension_type::sector_tile_count;

		//loop through all active nodes in the sector
		std::for_each(begin_itr, end_itr, [&](auto root_index)
			{
				//convert from subtile to global tile 
				auto world_tile = root_index + tile_offset;

				//get the iterator for the tile
				auto tile_start = colliders_in_tile_tracker.get_root_node_start(world_tile);
				auto tile_end = colliders_in_tile_tracker.end();

				//the rect for the tile
				math_2d_util::irect new_bounds = math_2d_util::irect::inverse_max_size_rect();

				using sector_type_type = typename sector_grid_helper_type::sector_tile_index_type;

				//convert to a tile type
				sector_type_type tile = sector_type_type{ world_tile };

				//get the tils coordinate
				math_2d_util::ivec2d tile_coordinate = grid_helper.to_xy<math_2d_util::ivec2d>(tile);

				//calculate the bounds for each agent
				std::for_each(tile_start, tile_end,
					[&](auto handle)
					{
						//get ref struct 
						typename collision_data_container_type::handle_reference_wrapper ref_struct = collision_data_container.get(handle);

						//calcualte x min max
						float x_min = ref_struct.x - ref_struct.radius;
						float x_max = ref_struct.x + ref_struct.radius;

						//calculate y min max 
						float y_min = ref_struct.y - ref_struct.radius;
						float y_max = ref_struct.y + ref_struct.radius;

						new_bounds.min.x = std::min(new_bounds.min.x, static_cast<int32>(x_min));
						new_bounds.max.x = std::max(new_bounds.max.x, static_cast<int32>(x_max));

						new_bounds.min.y = std::min(new_bounds.min.y, static_cast<int32>(y_min));
						new_bounds.max.y = std::max(new_bounds.max.y, static_cast<int32>(y_max));

					});

				//update the bounds of the tile
				overlap_grid.update_bounds(tile_coordinate, tile, new_bounds);

			});
	}


	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_bounds_in_all_sectors()
	{
		//single threaded version.
		//loop through all sectors and add their items into the simulation
		std::for_each(sectors_with_queued_items.begin(), sectors_with_queued_items.end(), [&](auto sector_index)
			{
				update_bounds_in_sector(sector_index);
			});
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_all_positions()
	{
		for (uint32_t is = 0; is < grid_dimensions::sector_grid_count; ++is)
		{
			update_positions_in_sector(is);

			
		}

		MiscUtilities::grid_function_helper::template_edge_corner_and_fill_function(grid_dimension_type::sectors_grid_w, grid_dimension_type::sectors_grid_w, 0, 0, [&]<typename edge_info>(uint32 x, uint32 y)
		{
			transfer_items_between_sectors<edge_info>((grid_dimension_type::sectors_grid_w * y) + x);
		});
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	template<typename Tedge_info>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::transfer_items_between_sectors(sector_count_type sector_index)
	{


		using grid_utility = MiscUtilities::grid_navigation_helper<grid_dimension_type::sectors_grid_w>;

		//define sector bounds 
		math_2d_util::uirect sector_bounds = grid_helper.sector_bounds(sector_index);

		static constexpr uint32_t diagonal_count = static_cast<uint32_t>(grid_utility::grid_directions::DIAGONAL_COUNT);

		uint32_t items_entering_sector = 0;

		//diagonal addresses that hold items moving over sector edge
		std::array<uint8_t, sector_transfer_buffers::max_item_transfer * diagonal_count> indexes_of_diag_items;
		std::array<uint8_t, diagonal_count> end_index_for_diagonal_transfer{};

		{
			//gather all diagonal entry cases
			if constexpr (!Tedge_info::is_on_left_edge() && !Tedge_info::is_on_up_edge())
			{
				auto& transfer_buffer_in_dir = sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::UP_LEFT>(sector_index)].get_ref_to_buffer<transfer_buffer_types::OTHER>();

				for (uint32_t ix = 0; ix < transfer_buffer_in_dir.size(); ++ix)
				{
					//check if the object is in this sector
					bool is_in_sector = math_2d_util::rect_2d_math::is_overlapping(sector_bounds, math_2d_util::uivec2d(transfer_buffer_in_dir[ix].position));

					//add to list for 
					indexes_of_diag_items[items_entering_sector] = ix;

					items_entering_sector += is_in_sector;
				};
			}

			end_index_for_diagonal_transfer[static_cast<uint32_t>(grid_utility::grid_directions::UP_LEFT) - static_cast<uint32_t>(grid_utility::grid_directions::DIAGONAL_START)] = items_entering_sector;

			if constexpr (!Tedge_info::is_on_right_edge() && !Tedge_info::is_on_up_edge())
			{
				auto& transfer_buffer_in_dir = sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::UP_RIGHT>(sector_index)].get_ref_to_buffer<transfer_buffer_types::OTHER>();

				for (uint32_t ix = 0; ix < transfer_buffer_in_dir.size(); ++ix)
				{
					//check if the object is in this sector
					bool is_in_sector = math_2d_util::rect_2d_math::is_overlapping(sector_bounds, math_2d_util::uivec2d(transfer_buffer_in_dir[ix].position));

					//add to list for 
					indexes_of_diag_items[items_entering_sector] = ix;

					items_entering_sector += is_in_sector;
				};
			}
				
			end_index_for_diagonal_transfer[static_cast<uint32_t>(grid_utility::grid_directions::UP_RIGHT) - static_cast<uint32_t>(grid_utility::grid_directions::DIAGONAL_START)] = items_entering_sector;

			if constexpr (!Tedge_info::is_on_right_edge() && !Tedge_info::is_on_down_edge())
			{
				auto& transfer_buffer_in_dir = sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::DOWN_RIGHT>(sector_index)].get_ref_to_buffer<transfer_buffer_types::OTHER>();

				for (uint32_t ix = 0; ix < transfer_buffer_in_dir.size(); ++ix)
				{
					//check if the object is in this sector
					bool is_in_sector = math_2d_util::rect_2d_math::is_overlapping(sector_bounds, math_2d_util::uivec2d(transfer_buffer_in_dir[ix].position));

					//add to list for 
					indexes_of_diag_items[items_entering_sector] = ix;

					items_entering_sector += is_in_sector;
				};
			}
			
			end_index_for_diagonal_transfer[static_cast<uint32_t>(grid_utility::grid_directions::DOWN_RIGHT) - static_cast<uint32_t>(grid_utility::grid_directions::DIAGONAL_START)] = items_entering_sector;


			if constexpr (!Tedge_info::is_on_left_edge() && !Tedge_info::is_on_down_edge())
			{
				auto& transfer_buffer_in_dir = sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::DOWN_LEFT>(sector_index)].get_ref_to_buffer<transfer_buffer_types::OTHER>();

				for (uint32_t ix = 0; ix < transfer_buffer_in_dir.size(); ++ix)
				{
					//check if the object is in this sector
					bool is_in_sector = math_2d_util::rect_2d_math::is_overlapping(sector_bounds, math_2d_util::uivec2d(transfer_buffer_in_dir[ix].position));

					//add to list for 
					indexes_of_diag_items[items_entering_sector] = ix;

					items_entering_sector += is_in_sector;
				};
			}

			end_index_for_diagonal_transfer[static_cast<uint32_t>(grid_utility::grid_directions::DOWN_RIGHT) - static_cast<uint32_t>(grid_utility::grid_directions::DIAGONAL_START)] = items_entering_sector;
		}


		if constexpr (!Tedge_info::is_on_left_edge())
		{
			items_entering_sector += sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::LEFT>(sector_index)].get_buffer_size<transfer_buffer_types::RIGHT>();
		}

		if constexpr (!Tedge_info::is_on_up_edge())
		{
			items_entering_sector += sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::UP>(sector_index)].get_buffer_size<transfer_buffer_types::DOWN>();
		}

		if constexpr (!Tedge_info::is_on_right_edge())
		{
			items_entering_sector += sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::RIGHT>(sector_index)].get_buffer_size<transfer_buffer_types::LEFT>();
		}

		if constexpr (!Tedge_info::is_on_down_edge())
		{
			items_entering_sector += sector_transfer_buffer_groups[grid_utility::get_address_for_direction< grid_utility::grid_directions::DOWN>(sector_index)].get_buffer_size<transfer_buffer_types::UP>();
		}

		//compare the number of items entering vs those leaving and get the difference 
		uint32_t transfer_dif = items_entering_sector - sector_transfer_removal_address_groups[sector_index].size();

		//allocate extra room to bulk insert items into if needed 
		//collision_data_container.

		//convert to index in map
		//auto root_index = grid_helper.from_xy(math_2d_util::uivec2d(x, y));
		//
		//if constexpr (!Tedge_info::is_on_left_edge())
		//{
		//	//transfer items in from the left buffer 
		//    int32_t left_offset = MiscUtilities::grid_navigation_helper<grid_dimension_type::sectors_grid_w>::get_offset_for_direction<MiscUtilities::grid_navigation_helper::grid_directions::LEFT>();
		//
		//	//sector to the left of current 
		//	sector_count_type left_sector_index = sector_index - left_offset;
		//
		//	//get the right transfer buffer 
		//	sector_transfer_buffers& sector_transfer_buffer_groups[left_sector_index].get_ref_to_buffer<transfer_buffer_types::RIGHT>()
		//}

	}


	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_physics()
	{
		//add any new items to the simulation 
		add_items_from_all_sectors();

		//move all objects 
		update_all_positions();

		//update the tile boundry overlaps 
		update_bounds_in_all_sectors();


		//object ask the physics system for a handle to a phys object
		
		//there is a map from the handle to the address in per sector data 

		//each object in the world registers movement commands and uses their handle to pass movement forces into the system

		//update the agent positions inside a sector 

		//loop through all the active sectors and update the posititons of all the active agents 

		//for each sector, for each active tile
		
		//get the list of all the other active tiles they have potential overlaps with
		
		//for each agent in the tile

		//check if agent overlaps with the bounds of the other overlap
		
		//gather all the agent id's inside the tile
		
		//use the agent id's to gather the real addresses for each of the potential collider list   
		
		//loop through the real addresses and compare flags to see what kind of collisions we are talking about 
		
		//for first pass do a discrete sphere check and if collision detected add to list of collisions 

		//loop through list of collisions and calculate repulsion forces 

		//sum up all the repulsion forces 

		//apply force to agent 
		
	}
	
	//move all objects 
	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::update_positions_in_sector(sector_count_type sector_index)
	{
		{
			//get the page data iterators
			auto page_begin_itr = collision_data_container.get_tight_packed_data().get_array_header().page_begin(sector_index);
			auto page_end_itr = collision_data_container.get_tight_packed_data().get_array_header().page_end(sector_index);

			//loop through all the pages that a sectors data is in
			std::for_each(page_begin_itr, page_end_itr, [&](auto& page_address_and_count)
				{
					//flip velocity if it will take the agent off the map
					for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
					{
						//get ref struct 
						collision_data_ref ref_struct = collision_data_container.get(real_address);

						float new_max_edge = (ref_struct.x + ref_struct.radius) + ref_struct.velocity_x * time_step;

						float new_min_edge = (ref_struct.x - ref_struct.radius) + ref_struct.velocity_x * time_step;

						//check if this will take the object off the bottom of the map
						bool will_take_off_map = new_max_edge > grid_dimension_type::tile_w || new_min_edge < 0;

						//flip the x velocity 
						ref_struct.velocity_x = will_take_off_map ? -ref_struct.velocity_x : ref_struct.velocity_x;
					}

					for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
					{
						//get ref struct 
						collision_data_ref ref_struct = collision_data_container.get(real_address);

						float new_max_edge = (ref_struct.y + ref_struct.radius) + ref_struct.velocity_y * time_step;

						float new_min_edge = (ref_struct.y - ref_struct.radius) + ref_struct.velocity_y * time_step;

						//check if this will take the object off the bottom of the map
						bool will_take_off_map = new_max_edge > grid_dimension_type::tile_w || new_min_edge < 0;

						//flip the x velocity 
						ref_struct.velocity_y = will_take_off_map ? -ref_struct.velocity_y : ref_struct.velocity_y;
					}

					//cant be bothered writing anothre iterator 
					//apply x movement
					for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
					{
						//get ref struct 
						collision_data_ref ref_struct = collision_data_container.get(real_address);

						//apply the x velocity to the x position 
						ref_struct.x += ref_struct.velocity_x * time_step;
					}

					//apply y movement 
					for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
					{
						//get ref struct 
						collision_data_ref ref_struct = collision_data_container.get(real_address);

						//apply the x velocity to the x position 
						ref_struct.y += ref_struct.velocity_y * time_step;
					}

				});
		}

		//create page buffer 
		{
			//get the page data iterators
			auto page_begin_itr = collision_data_container.get_tight_packed_data().get_array_header().page_begin(sector_index);
			auto page_end_itr = collision_data_container.get_tight_packed_data().get_array_header().page_end(sector_index);

			//define sector bounds 
			math_2d_util::uirect sector_bounds = grid_helper.sector_bounds(sector_index);

			//loop through all the pages that a sectors data is in
			std::for_each(page_begin_itr, page_end_itr, [&](auto& page_address_and_count)
				{
					using sector_change_tuple_type = std::tuple<collision_data_container_type::real_address_type, handle_type, math_2d_util::ivec2d, math_2d_util::ivec2d>;

					ArrayUtilities::fixed_size_vector_array< sector_change_tuple_type, collision_data_container_type::paged_array_type::page_size> items_changing_tile;

					ArrayUtilities::fixed_size_vector_array< sector_change_tuple_type, collision_data_container_type::paged_array_type::page_size> items_exiting_sector;

					//shift items in grid tracker 
					for (auto real_address = page_address_and_count.page_start_address; real_address < page_address_and_count.page_start_address + page_address_and_count.items_in_page; ++real_address)
					{
						
						//get ref struct
						auto ref_struct = collision_data_container.get(real_address);

						//old tile
						math_2d_util::uivec2d old_tile(static_cast<uint32_t>(ref_struct.x - (ref_struct.velocity_x * time_step)), static_cast<uint32_t>(ref_struct.y - (ref_struct.velocity_y * time_step)));

						//new tile
						math_2d_util::uivec2d new_tile(static_cast<uint32_t>(ref_struct.x), static_cast<uint32_t>(ref_struct.y));

						bool changed_tile = old_tile != new_tile;


						//optionally add this handle to the list of items leaving this sector
						if (changed_tile)
						{
							//check if this item is being inserted into another tile in this sector
							bool exiting_sector = !math_2d_util::rect_2d_math::is_overlapping(sector_bounds, new_tile);

							if (exiting_sector)
							{
								items_exiting_sector.push_back(sector_change_tuple_type(real_address, ref_struct.handle, old_tile, new_tile ));
							}
							else
							{
								items_changing_tile.push_back(sector_change_tuple_type(real_address, ref_struct.handle, old_tile, new_tile ));
							}
						}
					}

					//loop through all the tiles that have changed position and move them in the lookup structure
					std::for_each(items_changing_tile.cbegin(), items_changing_tile.cend(), [&](const sector_change_tuple_type& sector_change_info)
						{
							
							//convert to indexes
							auto from_address = grid_helper.from_xy(std::get<2>(sector_change_info));
							auto to_address = grid_helper.from_xy(std::get<3>(sector_change_info));
							auto handle = std::get<1>(sector_change_info);

							//remove from old tile and put in new tile
							colliders_in_tile_tracker.remove(from_address.index, handle);

							colliders_in_tile_tracker.add(to_address.index, handle);

							
						});

					//loop through all items that are leaving the sector
					std::for_each(items_exiting_sector.cbegin(), items_exiting_sector.cend(), [&](const sector_change_tuple_type& sector_change_info)
						{
							//convert to indexes
							auto from_coordinate = std::get<2>(sector_change_info);
							auto to_coordinate = std::get<3>(sector_change_info);

							auto from_address = grid_helper.from_xy(from_coordinate);
							auto handle = std::get<1>(sector_change_info);
							auto real_address = std::get<0>(sector_change_info);

							//get buffer to insert into
							auto& transfer_buffer_to_add_to = sector_transfer_buffer_groups[sector_index].get_buffer_for_transfer(from_coordinate, to_coordinate);

							//get the buffer to track items leaving the sector
							auto& address_of_items_leaving = sector_transfer_removal_address_groups[sector_index];

							//remove from old tile and put in new tile
							colliders_in_tile_tracker.remove(from_address.index, handle);

							//get ref struct
							collision_data_ref ref_struct = collision_data_container.get(real_address);

							new_collider_data transfer_data = new_collider_data(handle, math_2d_util::fvec2d(ref_struct.x, ref_struct.y), math_2d_util::fvec2d(ref_struct.velocity_x, ref_struct.velocity_y), ref_struct.radius);

							//add to correct buffer
							transfer_buffer_to_add_to.push_back(transfer_data);
							address_of_items_leaving.push_back(real_address);

						});
					
				});
		}
	}

	template<size_t Imax_objects, size_t Iworld_sector_x_count>
	inline void phyisics_2d_main<Imax_objects, Iworld_sector_x_count>::draw_debug(debug_draw_interface& draw_interface)
	{
		//draw a grid for all the tiles
		draw_interface.draw_grid(math_2d_util::fvec2d(0, 0), math_2d_util::ivec2d(grid_dimensions::tile_w, grid_dimensions::tile_w), 1.0f, debug_draw_interface::to_colour(200, 200, 200));

		//draw the grid for all the sectors
		draw_interface.draw_grid(math_2d_util::fvec2d(0, 0), math_2d_util::ivec2d(grid_dimensions::sectors_grid_w, grid_dimensions::sectors_grid_w), grid_dimensions::sectors_grid_w, debug_draw_interface::to_colour(150, 150, 150));


		//draw all the objects 
		auto itr_start = collision_data_container.get_tight_packed_data().get_array_header().begin();
		auto itr_end = collision_data_container.get_tight_packed_data().get_array_header().end();

		std::for_each(itr_start, itr_end, [&](auto& real_address)
			{
				collision_data_ref ref_struct = collision_data_container.get(real_address);

				draw_interface.draw_circle(math_2d_util::fvec2d(ref_struct.x, ref_struct.y), ref_struct.radius, debug_draw_interface::to_colour(255, 0, 0));
			});

	}
}