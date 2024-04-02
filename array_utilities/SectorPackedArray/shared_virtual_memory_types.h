#pragma once
#include <assert.h>

namespace ArrayUtilities
{
	template<typename Tnode_index_type>
	struct page_handle
	{
	private:
		static constexpr Tnode_index_type invalid_page_value = std::numeric_limits<Tnode_index_type>::max();

		Tnode_index_type page_number;

	public:

		/// <summary>
		/// exposes the underlying data type 
		/// </summary>
		using data_type = Tnode_index_type;

		constexpr bool is_valid() const
		{
			return page_number != invalid_page_value;
		}

		constexpr void destroy()
		{
			assert(is_valid());
			page_number = invalid_page_value;
		}

		constexpr void branchless_destroy(bool apply_destroy)
		{
			assert(is_valid() && apply_destroy);
			page_number -= ((page_number + 1) * apply_destroy);
		}

		constexpr page_handle() :page_number(invalid_page_value) {};

		constexpr page_handle(Tnode_index_type _page_number) :page_number(_page_number) {};

		//accessor to get the page 
		constexpr Tnode_index_type get_page() const
		{
			assert(is_valid());

			return page_number;
		}

		//same as get page but wont throw asserts as you are explicitly expecting invalid values here 
		constexpr Tnode_index_type get_page_expecting_invalid() const
		{
			return page_number;
		}

		constexpr void set_handle(page_handle new_handle_value)
		{
			assert(new_handle_value.is_valid(), "This function assumes if your setting the page address that there is not a page already mapped to that address");

			//dont try and beat the compiler 
			page_number = new_handle_value.get_page();

		}

		constexpr void branchless_set_handle(page_handle new_handle_value, bool apply_page)
		{
			assert(!apply_page || (new_handle_value.is_valid() == true), "This function assumes if your setting the page address that there is not a page already mapped to that address");

			//this assumes the existing page is a invalid value which == max value
			//by adding 1 max value wraps arround to 0,
			//by adding the rest of the page number it should set the virtual page to the new page value
			//if apply page == 0 then we are just adding 0 and should make no change
//			page_number += ((new_handle_value.page_number + 1) * apply_page);

			//dont try and beat the compiler 
			page_number = apply_page ? new_handle_value.get_page() : page_number;

		}
	
		//creates an invalid page handle 
		static constexpr page_handle< Tnode_index_type> invalid_page()
		{
			return page_handle<Tnode_index_type>{ invalid_page_value };
		}
	
	};
}