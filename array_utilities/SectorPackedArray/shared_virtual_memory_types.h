#pragma once


namespace ArrayUtilities
{
	template<typename Tnode_index_type>
	struct page_handle
	{
	private:
		static constexpr invalid_page_value = std::numeric_limits<Tnode_index_type>::max();

		Tnode_index_type page_number;

	public:

		bool is_valid()
		{
			return page_number != invalid_page_value;
		}

		void destroy()
		{
			check(is_valid());
			page_number = invalid_page_value;
		}

		void branchless_destroy(bool apply_destroy)
		{
			check(is_valid() && apply_destroy);
			page_number -= ((page_number + 1) * apply_destroy);
		}

		page_handle(Tnode_index_type _page_number) :page_number(_page_number) {};

		//accessor to get the page 
		Tnode_index_type get_page()
		{
			check(is_valid());

			return page_number;
		}

		void branchless_set_handle(page_handle new_handle_value, bool apply_page)
		{
			assert(apply_page && (new_handle_value.is_valid() == false), "This function assumes if your setting the page address that there is not a page already mapped to that address");


			//this assumes the existing page is a invalid value which == max value
			//by adding 1 max value wraps arround to 0,
			//by adding the rest of the page number it should set the virtual page to the new page value
			//if apply page == 0 then we are just adding 0 and should make no change
			page_number += ((new_handle_value.page_number + 1) * apply_page);
		}
	};
}