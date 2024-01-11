#pragma once
#include <array>
#include <assert.h>

#include "misc_utilities/int_type_selection.h"

namespace HandleSystem
{
	template<size_t Inumber_of_handles>
	struct handle_system
	{
		struct handle
		{
			using handle_value_type = MiscUtilities::uint_s<Inumber_of_handles>::uint_t;

			static constexpr handle_value_type invalid_handle_value = std::numeric_limits<handle_value_type>::max();

			handle_value_type value;
		};

	private:
		using handle_count_type = MiscUtilities::uint_s<Inumber_of_handles>::uint_t;

		//list of all the free handles
		std::array<handle, Inumber_of_handles> free_handles;
		handle_count_type free_handle_count;

	public:
			
		//func to get a new handle 
		handle get_handle();

		//func to return a handle for reuse
		void return_handle(handle handle_to_return);
	};
	
	template<size_t Inumber_of_handles>
	inline handle_system<Inumber_of_handles>::handle handle_system<Inumber_of_handles>::get_handle()
	{
		//dont go below 0 
		if (free_handle_count == 0) [[unlikely]]
		{
			return handle{ handle::invalid_handle_value};
		}

		return free_handles[--free_handle_count];
	}
	
	template<size_t Inumber_of_handles>
	inline void handle_system<Inumber_of_handles>::return_handle(handle handle_to_return)
	{
		//check handle is in expected ranges
		assert(handle_to_return.value < Inumber_of_handles);

		//check that we have not over returned values
		assert(free_handle_count.value < Inumber_of_handles);

		//check that the handle has not already been returned 
		assert(std::find(free_handles.begin(), free_handles.end(), handle_to_return) == free_handles.end());

		free_handles[free_handle_count++] = handle_to_return;
	}
}