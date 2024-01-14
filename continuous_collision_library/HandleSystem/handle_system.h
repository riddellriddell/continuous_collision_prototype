#pragma once
#include <array>
#include <assert.h>
#include <algorithm>

#include "misc_utilities/int_type_selection.h"

namespace HandleSystem
{
	template<size_t Ihandel_address_bits>
	struct handle_system
	{
		constexpr size_t max_handle_value = (1 << Ihandel_address_bits) -1;
		constexpr size_t max_valid_handle_values = max_handle_value - 1;

		struct handle
		{
			using handle_value_type = MiscUtilities::uint_s< max_handle_value>::uint_t;

			static constexpr handle_value_type invalid_handle_value = std::numeric_limits<handle_value_type>::max();

			handle_value_type value;
		};

	private:

		//data size big enough to store all values plus extra so we can have an invalid value 
		using handle_count_type = MiscUtilities::uint_s<Inumber_of_handles + 1>::uint_t;

		//list of all the free handles
		std::array<handle, max_valid_handle_values> free_handles;
		handle_count_type free_handle_count;

	public:
			
		//func to get a new handle 
		handle get_handle();

		//func to return a handle for reuse
		void return_handle(handle::handle_value_type handle_index_to_return);

		//check if handle is in free list
		//this function is slow and should not be used in runtime code 
		//it is intended for debug / assert 
		bool is_handle_free(handle::handle_value_type handle_index_to_return) const;

	};
	
	template<size_t Ihandel_address_bits>
	inline handle_system<Ihandel_address_bits>::handle handle_system<Ihandel_address_bits>::get_handle()
	{
		//dont go below 0 
		if (free_handle_count == 0) [[unlikely]]
		{
			return handle{ handle::invalid_handle_value};
		}

		return free_handles[--free_handle_count];
	}
	
	template<size_t Ihandel_address_bits>
	inline void handle_system<Ihandel_address_bits>::return_handle(handle::handle_value_type handle_index_to_return)
	{
		//check handle is in expected ranges
		assert(handle_index_to_return < max_valid_handle_values);

		//check that we have not over returned values
		assert(free_handle_count < max_valid_handle_values);

		//check that the handle has not already been returned 
		assert(!is_handle_free(handle_index_to_return));

		free_handles[free_handle_count++] = handle_index_to_return;
	}
	template<size_t Ihandel_address_bits>
	inline bool handle_system<Ihandel_address_bits>::is_handle_free(handle::handle_value_type handle_index_to_return) const
	{
		return std::find(free_handles.begin(), free_handles.begin() + free_handle_count, handle{ handle_index_to_return }) != free_handles.begin() + free_handle_count;
	}
}