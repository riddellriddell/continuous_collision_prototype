#pragma once
#include <array>
#include <assert.h>
#include <algorithm>

#include "misc_utilities/int_type_selection.h"

namespace HandleSystem
{
	template<size_t Imax_valid_handle_count>
	struct default_handle_type
	{
	public:
		using handle_index_type = MiscUtilities::uint_s< Imax_valid_handle_count>::int_type_t;

		static constexpr handle_index_type max_handle_count = Imax_valid_handle_count;
		static constexpr handle_index_type max_handle_value = Imax_valid_handle_count -1;

		//when a handle is freed not all the information is needed about the past handle 
		//such as the handle type. the freed handle is a version without that extra data
		//and is intended as being smaller in size to make data access quicker 
		struct free_handle_type
		{
		private:
			//the freed handle index
			handle_index_type value;

		public:
			free_handle_type() {};
			free_handle_type(handle_index_type free_handle_index) : value(free_handle_index) {};

			handle_index_type get_index()const
			{
				return value;
			}
		};


	private:

		static constexpr handle_index_type invalid_handle_value = std::numeric_limits<handle_index_type>::max();

		handle_index_type value;

	public:

		default_handle_type() {};

		default_handle_type(handle_index_type handle_start_index):value(handle_start_index){};

		default_handle_type(free_handle_type free_handle_data_to_build_from):value(free_handle_data_to_build_from.value) {};

		free_handle_type convert_to_free_handle()
		{
			return free_handle_type(value);
		}

		static constexpr handle_index_type get_invalid_index()
		{
			return invalid_handle_value;
		}

		//get the index value of this handle 
		handle_index_type get_index() const
		{
			return value;
		}
	};


	template< typename Thandle_type>
	struct handle_system
	{
	private:

		//accessor to make it easy to get later
		using handle_type = Thandle_type;

		//data size big enough to store all values plus extra so we can have an invalid value 
		using handle_count_type = Thandle_type::handle_index_type;

		//list of all the free handles
		std::array<typename Thandle_type::free_handle_type, Thandle_type::max_handle_value> free_handles;
		handle_count_type free_handle_count;

	public:
			
		//func to get a new handle 
		Thandle_type get_handle();

		//func to return a handle for reuse
		void return_handle(handle_count_type handle_index_to_return);

		//check if handle is in free list
		//this function is slow and should not be used in runtime code 
		//it is intended for debug / assert 
		bool is_handle_free(handle_count_type handle_index_to_return) const;

	};
	
	template<typename Thandle_type>
	inline Thandle_type handle_system<Thandle_type>::get_handle()
	{
		//dont go below 0 
		if (free_handle_count == 0) [[unlikely]]
		{
			//return an invalid handle 
			return Thandle_type(Thandle_type::get_invalid_index());
		}

		//convert a free handle type to a handle and return that
		return Thandle_type(free_handles[--free_handle_count]);
	}
	
	template<typename Thandle_type>
	inline void handle_system<Thandle_type>::return_handle(handle_count_type handle_index_to_return)
	{
		//check handle is in expected ranges
		assert(handle_index_to_return < Thandle_type::max_handle_value);

		//check that we have not over returned values
		assert(free_handle_count < Thandle_type::max_handle_value);

		//check that the handle has not already been returned 
		assert(!is_handle_free(handle_index_to_return));

		free_handles[free_handle_count++] = handle_index_to_return;
	}
	
	template<typename Thandle_type>
	inline bool handle_system<Thandle_type>::is_handle_free(handle_count_type handle_index_to_return) const
	{
		return std::find_if(free_handles.begin(), free_handles.begin() + free_handle_count, [&](const auto& x) {return x.get_index() == handle_index_to_return; }) != free_handles.begin() + free_handle_count;
	}
}