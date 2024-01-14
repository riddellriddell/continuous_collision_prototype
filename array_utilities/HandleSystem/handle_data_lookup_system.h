
#pragma once
#include <array>
#include <assert.h>
#include <algorithm>

#include "handle_system.h"
#include "misc_utilities/int_type_selection.h"
#include "misc_utilities/bits_needed_for_unsigned_int.h"

namespace HandleSystem
{
	template<typename Thandle_type, typename Taddress_type>
	struct handle_data_lookup_system
	{
		//the max number of handles in this system
		static constexpr size_t max_handle_count = Thandle_type::max_handle_count;

		using handle_type = Thandle_type;

		//handle system to track what has access to what 
		using handle_system_type = handle_system<handle_type>;

		using handle_system_index_type = handle_type::handle_index_type;

	private:
		
		//lookup array storing all the data 
		//list of all available handles 
		handle_system_type handles;

		//lookup table for the address in memory for the given handle 
		std::array<Taddress_type, max_handle_count> handle_address_lookup;
	public:

		//get a handle from the syste
		handle_system_type::default_handle_type get_handle();

		//remove a handle 
		void remove_handle(handle_system_index_type index_of_handle_to_remove);

		//get handle address
		Taddress_type get_address_for_handle(handle_system_index_type index_of_handle);

		//update a handles address 
		void update_handle_address(handle_system_index_type index_of_handle_to_update, Taddress_type new_address);

	};

	template<typename Thandle_type, typename Taddress_type>
	inline handle_data_lookup_system<Thandle_type, Taddress_type>::handle_system_type::default_handle_type
		handle_data_lookup_system<Thandle_type, Taddress_type>::get_handle()
	{
		//get a handle from the handel system
		return handles.get_handle();
	}

	template<typename Thandle_type, typename Taddress_type>
	inline void handle_data_lookup_system<Thandle_type, Taddress_type>::remove_handle(handle_system_index_type index_of_handle_to_remove)
	{
		//check that address has been allocated 
		assert(!handles.is_handle_free(index_of_handle_to_update));

		//return handle to system
		handles.return_handle(index_of_handle_to_remove);
	}

	template<typename Thandle_type, typename Taddress_type>
	inline Taddress_type handle_data_lookup_system<Thandle_type, Taddress_type>::get_address_for_handle(handle_system_index_type index_of_handle)
	{
		//check that address has been allocated 
		assert(!handles.is_handle_free(index_of_handle));

		//return the data at the address
		return handle_address_lookup[index_of_handle];
	}

	template<typename Thandle_type, typename Taddress_type>
	inline void handle_data_lookup_system<Thandle_type, Taddress_type>::update_handle_address(handle_system_index_type index_of_handle_to_update, Taddress_type new_address)
	{
		//check that address has been allocated 
		assert(!handles.is_handle_free(index_of_handle_to_update));

		handle_address_lookup[index_of_handle_to_update] = new_address;
	}

}