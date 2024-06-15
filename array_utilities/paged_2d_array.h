#pragma once
#include <stdint.h>
#include <iterator>
//#include <math.h>
#include <algorithm>
#include <array>
#include <limits>
#include <assert.h>
#include <bit>

#include "array_utilities/SectorPackedArray/virtual_memory_map.h"
#include "array_utilities/SectorPackedArray/paged_memory_header.h"
#include "misc_utilities/int_type_selection.h"


namespace ArrayUtilities
{

	static constexpr size_t calculate_number_of_pages_needed(size_t number_of_x_axis_items, size_t max_y_items, size_t max_total_y_items, size_t page_size)
	{
		size_t max_y_axis_pages = ((max_y_items - 1) / page_size) + 1;

		//how many items do i need to add to a single page to require the max number of pages for that axis
		size_t max_items_needed_for_max_y_pages = ((max_y_axis_pages - 1) * page_size) + 1;

		//how many pages can be filled if we just put one item in each x axis entry 
		size_t wasted_on_partial_fill = (std::min(max_total_y_items, (number_of_x_axis_items)));

		size_t entries_remaining = max_total_y_items - wasted_on_partial_fill;

		//after 1 item has been added  to a axis, how many extra items to get max pages on that axis
		size_t additional_items_needed_to_get_max_pages = (max_items_needed_for_max_y_pages - 1);
		
		//number of y axises we can fill to max pages per axis 
		//one is subtraceted as we already alocate one on the wasted on spread calc
		size_t full_filled = (additional_items_needed_to_get_max_pages > 0) ? entries_remaining / additional_items_needed_to_get_max_pages : 0;

		entries_remaining -= (full_filled * (max_items_needed_for_max_y_pages - 1));

		//take the remaining entries and fill one y axis until max pages is allocated or as much as possible 
		size_t last_axis_page_count = (((entries_remaining + 1) - 1) / page_size) + 1;

		//the final number of pages is as follows
		//the total number of pages we can fill by adding only one item 
		//plus the extra pages we can create by filling each y axis to the max 
		//plus the pages we can create with the remaining itmes 
		size_t total_pages_needed = wasted_on_partial_fill + (full_filled * (max_y_axis_pages - 1)) + (last_axis_page_count - 1);

		//make sure sizes are sane
		assert((total_pages_needed > 0) || max_total_y_items == 0);

		//the worst case is we add one agent to every sector and add all the agents to a singel sector plus one
		assert(total_pages_needed < (max_total_y_items / max_y_axis_pages) + 1 + number_of_x_axis_items);

		return total_pages_needed;
	}

	template<size_t Inumber_of_x_axis_items,size_t Imax_y_items,size_t Imax_total_y_items, size_t Ipage_size>
	struct paged_2d_array_header
	{

		//total amount of data that needs to be alocated 
		//the worst case is one item in every x axis cell and then one cell with all the remaining items 
		static constexpr size_t max_pages = calculate_number_of_pages_needed(Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size);
		static constexpr size_t max_y_axis_pages = ((Imax_y_items - 1) / Ipage_size) + 1;

		static_assert(max_pages < ((Imax_total_y_items / Ipage_size) + 1 + Inumber_of_x_axis_items));


		//sanity check that we are alocating at leas one page if we have more than 0 items to store
		static_assert((max_pages > 0) || Imax_total_y_items == 0);

		//the type we use to track how full the y axises are full
		using y_axis_count_type = MiscUtilities::uint_s<Imax_total_y_items + 1>::int_type_t;
		using x_axis_count_type = MiscUtilities::uint_s<Inumber_of_x_axis_items + 1>::int_type_t;


		static constexpr y_axis_count_type max_y_items = Imax_y_items;

		//this array header supports accessing arrays using an x,y address or a combined index  
		//to support this there are 2 sets of virtual memory maps mapped to the same memory 
		//the correct one is accessed using type punning 
		//these 2 usings are used to define the 2 different map types 
		using y_axis_virtual_memory_map_type = virtual_memory_map<Ipage_size, max_y_axis_pages, max_pages>;
		
		using combined_address_virtual_memory_map_type = virtual_memory_map<Ipage_size, max_y_axis_pages* Inumber_of_x_axis_items, max_pages>;

		//make sure both combined and single are using the same representation for page handles
		static_assert(std::is_same<y_axis_virtual_memory_map_type::page_handle_type, combined_address_virtual_memory_map_type::page_handle_type>::value);

		//the address type returned when pulling from the mem map
		using real_node_address_type = y_axis_virtual_memory_map_type::real_node_address_type;
	
		using virtual_y_axis_node_adderss_type = y_axis_virtual_memory_map_type::virtual_address_type;

		using virtual_combined_node_adderss_type = combined_address_virtual_memory_map_type::virtual_address_type;


		//the data type returned when push or popping addresses from the mem map
		using address_return_type = std::tuple<real_node_address_type, virtual_combined_node_adderss_type, virtual_y_axis_node_adderss_type>;

		static constexpr y_axis_count_type invalid_y_axis_value = std::numeric_limits<y_axis_count_type>::max();
		static constexpr x_axis_count_type invalid_x_axis_value = std::numeric_limits<x_axis_count_type>::max();

		static constexpr uint32_t page_size = Ipage_size;

		//get the end address for an x axis index
		virtual_y_axis_node_adderss_type get_virtual_end_address_for_x_axis(x_axis_count_type x_axis_index);

		//if a memory page is not allocated for an address one is allocated then the real address is resolved and returned
		real_node_address_type allocate_page_for_address_and_return_real_address(virtual_combined_node_adderss_type virtual_address);

		//adds an item to the back of the array and if required alocates new memory pagex 
		real_node_address_type push_back(x_axis_count_type x_axis_index);

		//adds an item to the end of the sub array and returns the address details
		address_return_type push_back_and_return_address(x_axis_count_type x_axis_index);
		
		//removes the last item in a y axis array and frees pages if needed 
		void pop_back(x_axis_count_type x_axis_index);

		//remove the last element and return the address details for it 
		address_return_type pop_back_and_return_address(x_axis_count_type x_axis_index);

		//expand the array address space by an amount 
		//will throw an error if new axis count less than start count
		void expand(x_axis_count_type x_axis_index, y_axis_count_type new_y_axis_count);

		//find the read write address 
		real_node_address_type find_address(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type virtual_address) const;
		real_node_address_type find_address(virtual_combined_node_adderss_type virtual_address) const;
		real_node_address_type find_address(real_node_address_type virtual_address) const;

		//clear all items in an axis
		void clear_axis(x_axis_count_type axis_index);

		//clear all items in the entire array
		void clear_all_axis();
		
		//the maximum number of items neede for all pages 
		static constexpr size_t max_total_entries = max_pages * Ipage_size;

		//page tracker that tracks what pages are available
		paged_memory_header<max_pages> paged_memory_tracker;

		//make the page tracker is creating handles the same as the type expected by the virtual memory system 
		static_assert(std::is_same<paged_memory_header<max_pages>::page_handle_type, combined_address_virtual_memory_map_type::page_handle_type>::value);

		//the number of items in the y axis for each x axis 
		//todo make private 
		std::array<y_axis_count_type, Inumber_of_x_axis_items> y_axis_count = {};

		const std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>& get_y_axis_memory_map() const;
	private:
		combined_address_virtual_memory_map_type& get_all_axis_memory_map_internal();
	public:
		y_axis_count_type num() const;

		const combined_address_virtual_memory_map_type& get_all_axis_memory_map() const;

		static constexpr virtual_combined_node_adderss_type convert_from_y_axis_to_combined_virtual_address(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type y_axis_virtual_address);
		
		static constexpr x_axis_count_type convert_from_combined_virtual_address_to_x(virtual_combined_node_adderss_type combined_address);

	private:
		//array of all the address mappings 
		std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items> virtual_memory_lookup;

		//make sure the per y axis memory lookup array is the same size as the all axis memeory lookup
		static_assert(sizeof(std::array<y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>) == sizeof(combined_address_virtual_memory_map_type));
	
	public:

#pragma region Validation

		//is virtual address valid
		bool is_address_valid(virtual_combined_node_adderss_type address) const;

		bool is_address_valid(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type y_axis_virtual_address) const;

#pragma endregion 


#pragma region Iterator

		//this iterator is for iterating over all the real addresses in a single y axis
		class y_real_address_iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = real_node_address_type;
			using difference_type = std::ptrdiff_t;
			using reference = real_node_address_type&;
			using parent_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;

			x_axis_count_type axis_to_iterate_over;
			virtual_y_axis_node_adderss_type virtual_index;
			real_node_address_type real_index;
			const parent_type& parent_ref;

			//an array of all the real addresses in the 

			explicit y_real_address_iterator(const parent_type& parent, x_axis_count_type x_axis, virtual_y_axis_node_adderss_type y_virtual_address = virtual_y_axis_node_adderss_type(0)) : parent_ref(parent), axis_to_iterate_over(x_axis), virtual_index(y_virtual_address), real_index(0)
			{
				
			}

		private:
			void next()
			{
				//we can safely go to the next address
				++real_index;
				//move the virtual index to the next page
				++virtual_index;		
			}
		public:


			real_node_address_type& operator*()
			{
				//check if we have moved to the next page
				if (y_axis_virtual_memory_map_type::is_first_item_in_real_page(real_index))
				{
					//get new real address
					real_index = parent_ref.find_address(axis_to_iterate_over, virtual_index);
				}

				//do some extra checks to make sure we are accessing a valid address
				assert(parent_ref.is_address_valid(axis_to_iterate_over, virtual_index));

				return real_index;
			}

			y_real_address_iterator& operator++() {
				next();
				return *this;
			}

			y_real_address_iterator operator++(int) {
				y_real_address_iterator tmp = *this;
				next();
				return tmp;
			}

			y_real_address_iterator operator+(uint32_t n) const 
			{
				auto iterator = y_real_address_iterator(parent_ref, axis_to_iterate_over, virtual_y_axis_node_adderss_type(virtual_index.address + n));
				return iterator;
			}

			y_real_address_iterator& operator+=(uint32_t n)
			{
				//offset the address
				virtual_index += n;
				
				//get new real address
				real_index = parent_ref.find_address(axis_to_iterate_over, virtual_index);

				return *this;
			}

			friend bool operator==(const y_real_address_iterator& lhs, const y_real_address_iterator& rhs)
			{
				return lhs.virtual_index == rhs.virtual_index;
			}

			friend bool operator!=(const y_real_address_iterator& lhs, const y_real_address_iterator& rhs) {
				return !(lhs == rhs);
			}

		private:

		};

		//this will return a start address and the number of items allocated in that page 
		//use this to work with simd or other tools that need blocks of addresses up front
		class y_real_page_address_iterator
		{
		public:
			struct page_start_and_count
			{
				real_node_address_type page_start_address;
				y_axis_count_type items_in_page;
			};

			using iterator_category = std::forward_iterator_tag;
			using value_type = page_start_and_count;
			using difference_type = std::ptrdiff_t;
			using reference = page_start_and_count&;
			using parent_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;

			x_axis_count_type axis_to_iterate_over;
			y_axis_count_type first_empty_page;
			y_axis_count_type current_page;
			y_axis_count_type number_of_items_in_last_page;

			value_type real_address_and_count;
			
			const parent_type& parent_ref;

			explicit y_real_page_address_iterator(const parent_type& parent, x_axis_count_type x_axis) : 
				parent_ref(parent), 
				axis_to_iterate_over(x_axis), 
				first_empty_page(parent.y_axis_count[axis_to_iterate_over] >> y_axis_virtual_memory_map_type::local_address_bits),
				current_page(0),
				number_of_items_in_last_page(y_axis_virtual_memory_map_type::get_sub_page_index(virtual_y_axis_node_adderss_type(parent.y_axis_count[axis_to_iterate_over])))
			{

			}

			void setup_first_value()
			{
			}

			void setup_end_value()
			{
				//setup current page to be one more than the last active page
				// but also the first page if there is nothing in it
				// this is so we can correctly compate begin iterator to end and detect when we have reached the last page 
				current_page = (parent_ref.y_axis_count[axis_to_iterate_over] + (y_axis_virtual_memory_map_type::number_of_items_per_page - 1)) >> y_axis_virtual_memory_map_type::local_address_bits;
			}

		private:
			void next()
			{
				//move to next page 
				++current_page;
			}
		public:


			reference operator*()
			{
				//do some extra checks to make sure we are accessing a valid address
				assert(parent_ref.virtual_memory_lookup[axis_to_iterate_over].does_virtual_page_have_real_page(current_page));
				
				//setup real address
				real_address_and_count.page_start_address = parent_ref.virtual_memory_lookup[axis_to_iterate_over].resolve_page_number_to_real_address(current_page);
				
				real_address_and_count.items_in_page = (current_page == (first_empty_page)) ? number_of_items_in_last_page : page_size;

				return real_address_and_count;
			}

			y_real_page_address_iterator& operator++() {
				next();
				return *this;
			}

			y_real_page_address_iterator operator++(int) {
				y_real_page_address_iterator tmp = *this;
				next();
				return tmp;
			}

			
			friend bool operator==(const y_real_page_address_iterator& lhs, const y_real_page_address_iterator& rhs)
			{
				return lhs.current_page == rhs.current_page;
			}

			friend bool operator!=(const y_real_page_address_iterator& lhs, const y_real_page_address_iterator& rhs) {
				return !(lhs == rhs);
			}

		private:
		};

		
		class real_page_address_iterator
		{
		public:
			struct page_start_and_count
			{
				real_node_address_type page_start_address;
				y_axis_count_type items_in_page;
			};


		private:
			//int type used for page handle 
			using page_index_type = combined_address_virtual_memory_map_type::page_handle_type::data_type;
			using parent_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;
			using reference = page_start_and_count&;

			page_index_type current_page;
			y_axis_count_type items_remaining; 

			page_start_and_count real_address_and_count;

			const parent_type& parent_ref;

		public:
			real_page_address_iterator(const parent_type& parent, page_index_type start_page, y_axis_count_type item_count) :parent_ref(parent), current_page(start_page), items_remaining(item_count)
			{

			};

			real_page_address_iterator(const parent_type& parent, x_axis_count_type target_axis) :parent_ref(parent), current_page(target_axis* max_y_axis_pages), items_remaining(parent.y_axis_count[target_axis])
			{

			};

			static real_page_address_iterator get_end_value(const parent_type& parent, page_index_type start_page, y_axis_count_type item_count)
			{
				//page past end
				page_index_type end_page = start_page + (item_count >> y_axis_virtual_memory_map_type::local_address_bits) + 1;

				//setup current page to be one more than the last active page
				// but also the first page if there is nothing in it
				// this is so we can correctly compate begin iterator to end and detect when we have reached the last page 
				return real_page_address_iterator(parent, end_page, item_count);
			}

			static real_page_address_iterator get_end_value(const parent_type& parent, x_axis_count_type target_axis)
			{
				//page past end
				page_index_type end_page = (target_axis * max_y_axis_pages) + (parent.y_axis_count[target_axis] + (page_size -1) >> y_axis_virtual_memory_map_type::local_address_bits);

				//setup current page to be one more than the last active page
				// but also the first page if there is nothing in it
				// this is so we can correctly compate begin iterator to end and detect when we have reached the last page 
				return real_page_address_iterator(parent, end_page, parent.y_axis_count[target_axis]);
			}

		private:
			void next()
			{
				//move to next page 
				++current_page;

				//decrement the number of items remaining 
				items_remaining -= page_size;
			}
		public:
			virtual_combined_node_adderss_type get_page_virtual_address() const
			{
				return virtual_combined_node_adderss_type(current_page * page_size);
			}

			virtual_combined_node_adderss_type get_page_virtual_address(y_axis_count_type offset_in_page) const
			{
				return virtual_combined_node_adderss_type((current_page * page_size) + offset_in_page);
			}


			reference operator*()
			{
				const combined_address_virtual_memory_map_type& combined_memory_map = parent_ref.get_all_axis_memory_map();

				//do some extra checks to make sure we are accessing a valid address
				assert(combined_memory_map.does_virtual_page_have_real_page(current_page));

				//setup real address
				real_address_and_count.page_start_address = combined_memory_map.resolve_page_number_to_real_address(current_page);
				real_address_and_count.items_in_page = std::min(decltype(items_remaining)( page_size), items_remaining);

				return real_address_and_count;
			}

			real_page_address_iterator& operator++() {
				next();
				return *this;
			}

			real_page_address_iterator operator++(int) {
				real_page_address_iterator tmp = *this;
				next();
				return tmp;
			}



			friend bool operator==(const real_page_address_iterator& lhs, const real_page_address_iterator& rhs)
			{
				return lhs.current_page == rhs.current_page;
			}

			friend bool operator!=(const real_page_address_iterator& lhs, const real_page_address_iterator& rhs) {
				return !(lhs == rhs);
			}


		public:

			

			
		};
		
		class real_and_virtual_page_address_iterator
		{
		public:
			struct page_start_and_count
			{
				real_node_address_type page_start_address;
				virtual_combined_node_adderss_type virtual_page_start_address;
				y_axis_count_type items_in_page;
			};


		private:
			//int type used for page handle 
			using page_index_type = combined_address_virtual_memory_map_type::page_handle_type::data_type;
			using parent_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;
			using reference = page_start_and_count&;

			page_index_type current_page;
			y_axis_count_type items_remaining;

			page_start_and_count page_address_and_count;

			const parent_type& parent_ref;

		public:
			real_and_virtual_page_address_iterator(const parent_type& parent, page_index_type start_page, y_axis_count_type item_count) :parent_ref(parent), current_page(start_page), items_remaining(item_count)
			{

			};

			real_and_virtual_page_address_iterator(const parent_type& parent, x_axis_count_type target_axis) :parent_ref(parent), current_page(target_axis* max_y_axis_pages), items_remaining(parent.y_axis_count[target_axis])
			{

			};

			static real_and_virtual_page_address_iterator get_end_value(const parent_type& parent, page_index_type start_page, y_axis_count_type item_count)
			{
				//page past end
				page_index_type end_page = start_page + (item_count >> y_axis_virtual_memory_map_type::local_address_bits) + 1;

				//setup current page to be one more than the last active page
				// but also the first page if there is nothing in it
				// this is so we can correctly compate begin iterator to end and detect when we have reached the last page 
				return real_and_virtual_page_address_iterator(parent, end_page, item_count);
			}

			static real_and_virtual_page_address_iterator get_end_value(const parent_type& parent, x_axis_count_type target_axis)
			{
				//page past end
				page_index_type end_page = (target_axis * max_y_axis_pages) + (parent.y_axis_count[target_axis] + (page_size - 1) >> y_axis_virtual_memory_map_type::local_address_bits);

				//setup current page to be one more than the last active page
				// but also the first page if there is nothing in it
				// this is so we can correctly compate begin iterator to end and detect when we have reached the last page 
				return real_and_virtual_page_address_iterator(parent, end_page, parent.y_axis_count[target_axis]);
			}

		private:
			void next()
			{
				//move to next page 
				++current_page;

				//decrement the number of items remaining 
				items_remaining -= page_size;
			}
		public:
			virtual_combined_node_adderss_type get_page_virtual_address() const
			{
				return virtual_combined_node_adderss_type(current_page * page_size);
			}

			virtual_combined_node_adderss_type get_page_virtual_address(y_axis_count_type offset_in_page) const
			{
				return virtual_combined_node_adderss_type((current_page * page_size) + offset_in_page);
			}


			reference operator*()
			{
				const combined_address_virtual_memory_map_type& combined_memory_map = parent_ref.get_all_axis_memory_map();

				//do some extra checks to make sure we are accessing a valid address
				assert(combined_memory_map.does_virtual_page_have_real_page(current_page));

				if (current_page >= (max_y_axis_pages * Inumber_of_x_axis_items))
				{
					//have gone off end of array
					assert(false);
				}

				//setup real address
				page_address_and_count.page_start_address = combined_memory_map.resolve_page_number_to_real_address(current_page);
				page_address_and_count.virtual_page_start_address = virtual_combined_node_adderss_type(current_page << y_axis_virtual_memory_map_type::local_address_bits);
				page_address_and_count.items_in_page = std::min(decltype(items_remaining)(page_size), items_remaining);

				//doing this because visual studio cant assert :(
				if (page_address_and_count.page_start_address.address >= max_total_entries ||
					page_address_and_count.items_in_page >= Ipage_size ||
					page_address_and_count.virtual_page_start_address.address >= combined_address_virtual_memory_map_type::max_virtual_address
					)
				{
					//have gone off end of array
					assert(false);
				}

				return page_address_and_count;
			}

			real_and_virtual_page_address_iterator& operator++() {
				next();
				return *this;
			}

			real_and_virtual_page_address_iterator operator++(int) {
				real_and_virtual_page_address_iterator tmp = *this;
				next();
				return tmp;
			}



			friend bool operator==(const real_and_virtual_page_address_iterator& lhs, const real_and_virtual_page_address_iterator& rhs)
			{
				return lhs.current_page == rhs.current_page;
			}

			friend bool operator!=(const real_and_virtual_page_address_iterator& lhs, const real_and_virtual_page_address_iterator& rhs) {
				return !(lhs == rhs);
			}


		public:

		};



		class all_real_address_iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = real_node_address_type;
			using difference_type = std::ptrdiff_t;
			using reference = real_node_address_type&;
			using parent_type = paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>;

			x_axis_count_type axis_to_iterate_over;
			virtual_y_axis_node_adderss_type virtual_index;
			real_node_address_type real_index;
			const parent_type& parent_ref;

			//an array of all the real addresses in the 

			explicit all_real_address_iterator(const parent_type& parent, x_axis_count_type x_axis = 0) : parent_ref(parent), axis_to_iterate_over(x_axis), virtual_index(0)
			{

			}

			void setup_first_value()
			{

				for (; axis_to_iterate_over < parent_ref.y_axis_count.size(); ++axis_to_iterate_over)
				{
					if (parent_ref.y_axis_count[axis_to_iterate_over] > 0)
					{
						//get real address
						real_index = parent_ref.find_address(axis_to_iterate_over, virtual_y_axis_node_adderss_type(0));
						return;
					}
				}
			}

		private:

			void next()
			{
				//go to the next index
				++virtual_index;

				while (virtual_index.address >= parent_ref.y_axis_count[axis_to_iterate_over])
				{
					virtual_index = virtual_y_axis_node_adderss_type(0);

					++axis_to_iterate_over;

					//check if off end of array
					if (axis_to_iterate_over >= parent_ref.y_axis_count.size())
					{
						//at end of array
						return;
					}
				}

				//check if we have moved to the next page
				if ( y_axis_virtual_memory_map_type::is_first_item_in_real_page(virtual_index))
				{
					//get new real address
					real_index = parent_ref.find_address(axis_to_iterate_over, virtual_index);
				}
				else
				{
					//we can safely go to the next address
					++real_index;
				}
			}
		public:


			real_node_address_type& operator*()
			{
				//do some extra checks to make sure we are accessing a valid address
				assert(parent_ref.is_address_valid(axis_to_iterate_over, virtual_index));

				return real_index;
			}

			all_real_address_iterator& operator++() {
				next();
				return *this;
			}

			all_real_address_iterator operator++(int) {
				all_real_address_iterator tmp = *this;
				next();
				return tmp;
			}

			all_real_address_iterator operator+(uint32_t n) const
			{
				return all_real_address_iterator(parent_ref, axis_to_iterate_over, virtual_index + n);
			}

			all_real_address_iterator& operator+=(uint32_t n)
			{
				//offset the address
				virtual_index += n;

				//get new real address
				real_index = parent_ref.find_address(axis_to_iterate_over, virtual_index);

				return *this;
			}

			friend bool operator==(const all_real_address_iterator& lhs, const all_real_address_iterator& rhs)
			{
				return (lhs.axis_to_iterate_over == rhs.axis_to_iterate_over);
			}

			friend bool operator!=(const all_real_address_iterator& lhs, const all_real_address_iterator& rhs) {
				return !(lhs == rhs);
			}

		private:
		};

		y_real_address_iterator begin(x_axis_count_type x_index) const;
		y_real_address_iterator end(x_axis_count_type x_index) const;

		real_page_address_iterator real_only_page_begin(x_axis_count_type x_index) const;
		real_page_address_iterator real_only_page_end(x_axis_count_type x_index) const;

		real_and_virtual_page_address_iterator page_begin(x_axis_count_type x_index) const;
		real_and_virtual_page_address_iterator page_end(x_axis_count_type x_index) const;

		all_real_address_iterator begin() const;
		all_real_address_iterator end() const;

#pragma endregion 

	};

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::virtual_y_axis_node_adderss_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_virtual_end_address_for_x_axis(x_axis_count_type x_axis_index)
	{
		return typename y_axis_virtual_memory_map_type::virtual_address_type{ y_axis_count[x_axis_index] };
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_node_address_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::allocate_page_for_address_and_return_real_address(virtual_combined_node_adderss_type virtual_address)
	{
		//check we have not exceeded the y axis count limit
		assert(virtual_address.address < combined_address_virtual_memory_map_type::max_virtual_address);

		//get the page for this
		auto virtual_page_number = combined_address_virtual_memory_map_type::extract_page_number_from_virtual_address(virtual_address);

		//get ref to combined memory map
		auto& virtual_mem_map = get_all_axis_memory_map_internal();

		//check if adding this item needs a new memory page
		const bool is_page_alocated = virtual_mem_map.does_virtual_page_have_real_page(virtual_page_number);

		//allocate a new page if needed
		auto new_page = paged_memory_tracker.branchless_allocate(!is_page_alocated);

		//optionally apply the new page value and convert the address over to a "physical address"
		virtual_mem_map.non_branching_add_page(virtual_page_number, new_page, !is_page_alocated);

		//apply the virtual page to the virtual address
		auto address_to_write_to = virtual_mem_map.resolve_address_using_virtual_page_offset(virtual_address, virtual_page_number);

		//return the address that can be safely written to
		return address_to_write_to;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_node_address_type
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::push_back(x_axis_count_type x_axis_index)
	{
		// indexing outside x array bounds 
		assert(x_axis_index < Inumber_of_x_axis_items);

		//create virtual address
		auto new_y_address = typename y_axis_virtual_memory_map_type::virtual_address_type{ y_axis_count[x_axis_index]};

		//increment the axis count
		y_axis_count[x_axis_index]++;

		//check we have not exceeded the y axis count limit
		assert(new_y_address.address < Imax_y_items);

		//get the page for this
		auto virtual_page_number = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(new_y_address);

		//check if adding this item needs a new memory page
		const bool is_page_alocated = virtual_memory_lookup[x_axis_index].does_virtual_page_have_real_page(virtual_page_number);

		//allocate a new page if needed
		auto new_page = paged_memory_tracker.branchless_allocate(!is_page_alocated);

		//optionally apply the new page value and convert the address over to a "physical address"
		virtual_memory_lookup[x_axis_index].non_branching_add_page(virtual_page_number, new_page,!is_page_alocated);

		//apply the virtual page to the virtual address
		auto address_to_write_to = virtual_memory_lookup[x_axis_index].resolve_address_using_virtual_page_offset(new_y_address, virtual_page_number);

		//return the address that can be safely written to
		return address_to_write_to;
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::address_return_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::push_back_and_return_address(x_axis_count_type x_axis_index)
	{
		//get the y axis address at the end of the range
		auto y_address_to_push_to = get_virtual_end_address_for_x_axis(x_axis_index);

		//increment the address count
		++y_axis_count[x_axis_index];

		//convert to combined 
		auto combined_address_to_push_to = convert_from_y_axis_to_combined_virtual_address(x_axis_index, y_address_to_push_to);
		
		//resolve the virtual address to a real one 
		auto real_address = allocate_page_for_address_and_return_real_address(combined_address_to_push_to);

		//return struct with all elements. I hope this gets inlined and optimized out by the compiler
		return address_return_type{ real_address ,combined_address_to_push_to,y_address_to_push_to};
	}
	;

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline void paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::pop_back(x_axis_count_type x_axis_index)
	{
		//get the y axis virtual address as well as decrement the number of items in this y axis array
		auto address_to_pop = typename y_axis_virtual_memory_map_type::virtual_address_type { --y_axis_count[x_axis_index] };

		//check if we need to free the underlying memory page
		bool is_only_item_in_page = y_axis_virtual_memory_map_type::is_first_item_in_real_page(address_to_pop);

		auto virtual_page_index = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(address_to_pop);

		//clear the memory page but only if it is the last item in the memory pool
		paged_memory_tracker.branchless_free(virtual_memory_lookup[x_axis_index].get_page_handle_to_return(virtual_page_index), is_only_item_in_page);
		
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::address_return_type
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::pop_back_and_return_address(x_axis_count_type x_axis_index)
	{
		//get the y axis virtual address as well as decrement the number of items in this y axis array
		auto y_virtual_address_to_pop = typename y_axis_virtual_memory_map_type::virtual_address_type{ --y_axis_count[x_axis_index] };

		//check if we need to free the underlying memory page
		bool is_only_item_in_page = y_axis_virtual_memory_map_type::is_first_item_in_real_page(y_virtual_address_to_pop);

		//get the page this virtual address maps to 
		auto virtual_page_index = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(y_virtual_address_to_pop);

		//convert from y axis to combined 
		auto combined_virtual_address = convert_from_y_axis_to_combined_virtual_address(x_axis_index, y_virtual_address_to_pop);

		//we need the address so we can move data around it arrays 
		auto real_address = virtual_memory_lookup[x_axis_index].resolve_address_using_virtual_page_offset(y_virtual_address_to_pop, virtual_page_index);
		
		//clear the memory page but only if it is the last item in the memory pool
		paged_memory_tracker.branchless_free(virtual_memory_lookup[x_axis_index].get_page_handle_to_return(virtual_page_index), is_only_item_in_page);

		//return the address of the last item that was just poped 
		return address_return_type{ real_address ,combined_virtual_address,y_virtual_address_to_pop };
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline void paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::expand(x_axis_count_type x_axis_index, y_axis_count_type new_y_axis_count)
	{
		//check we are not trying to contract 
		assert(y_axis_count[x_axis_index] <= new_y_axis_count);

		//the number of virtual pages to the start of the axis 
		typename combined_address_virtual_memory_map_type::page_handle_type::data_type axis_page_offset = x_axis_index * max_y_axis_pages;

		//convert both to page indexes
		typename combined_address_virtual_memory_map_type::page_handle_type::data_type current_page_count = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(y_axis_virtual_memory_map_type::virtual_address_type(y_axis_count[x_axis_index] + (page_size - 1)));
		typename combined_address_virtual_memory_map_type::page_handle_type::data_type new_page_count = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(y_axis_virtual_memory_map_type::virtual_address_type(new_y_axis_count + (page_size - 1)));

		//apply offset for start of axis
		current_page_count += axis_page_offset;
		new_page_count += axis_page_offset;

		//get pages from the page handler and add them to the table
		for (uint32_t ipage_index = current_page_count; ipage_index < new_page_count; ++ipage_index)
		{
			//get a page
			typename paged_memory_header<max_pages>::page_handle_type new_page = paged_memory_tracker.allocate();

			//add it to the list of pages assigned to this memory tracker 
			get_all_axis_memory_map_internal().add_page(ipage_index, new_page);
		}

		//can now safely set the new axis size 
		y_axis_count[x_axis_index] = new_y_axis_count;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_node_address_type
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::find_address(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type virtual_address) const
	{
		assert(virtual_address.address < y_axis_count[x_axis_index]);
		return get_y_axis_memory_map()[x_axis_index].resolve_address(virtual_address);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_node_address_type
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::find_address(virtual_combined_node_adderss_type virtual_address) const
	{
		return get_all_axis_memory_map().resolve_address(virtual_address);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_node_address_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::find_address(real_node_address_type virtual_address) const
	{
		//check that the address is at least in memory 
		assert(virtual_address.address < max_total_entries);
		return virtual_address;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline void paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::clear_axis(typename x_axis_count_type axis_index)
	{
		virtual_y_axis_node_adderss_type last_possible_entry(y_axis_count[axis_index] + y_axis_virtual_memory_map_type::number_of_items_per_page - 1);

		//get the last active page
		auto last_page = y_axis_virtual_memory_map_type::extract_page_number_from_virtual_address(last_possible_entry);
		
		for (uint32_t i = 0; i < last_page; i++)
		{
			//get the page to return
			auto& handle_to_return =  virtual_memory_lookup[axis_index].get_page_handle_to_return(i);
			
			//reutrn the page 
			paged_memory_tracker.free(handle_to_return);
		
		}

		y_axis_count[axis_index] = 0;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline void paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::clear_all_axis()
	{
		for (x_axis_count_type ix = 0; ix < y_axis_count.size(); ++ix)
		{
			clear_axis(ix);
		}
	}

	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline const std::array<typename paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_virtual_memory_map_type, Inumber_of_x_axis_items>& 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_y_axis_memory_map() const
	{
		//return a ref to virtual mem lookup array
		return virtual_memory_lookup;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline typename paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::combined_address_virtual_memory_map_type& 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_all_axis_memory_map_internal() 
	{
		//i don't like this code but have not found a clean way to treat an array of arrays the same as a flat array 

		auto reinterpreted_virtual_memory_map = reinterpret_cast<combined_address_virtual_memory_map_type*>(virtual_memory_lookup.data());

		static_assert(std::is_same< decltype(reinterpreted_virtual_memory_map), combined_address_virtual_memory_map_type*>::value);

		return *std::launder(reinterpreted_virtual_memory_map);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline  paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_axis_count_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::num() const
	{
		return std::accumulate(y_axis_count.begin(), y_axis_count.end(), 0);
	}
	
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline const typename paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::combined_address_virtual_memory_map_type& 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::get_all_axis_memory_map() const
	{
		//todo use correct start life as here

		auto reinterpreted_virtual_memory_map = reinterpret_cast< const combined_address_virtual_memory_map_type*>(virtual_memory_lookup.data());

		static_assert(std::is_same< decltype(reinterpreted_virtual_memory_map), const combined_address_virtual_memory_map_type*>::value);

		return *std::launder(reinterpreted_virtual_memory_map);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline constexpr paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::virtual_combined_node_adderss_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::convert_from_y_axis_to_combined_virtual_address(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type y_axis_virtual_address)
	{
		return virtual_combined_node_adderss_type{ (y_axis_virtual_address.address + (x_axis_index * (max_y_axis_pages << y_axis_virtual_memory_map_type::local_address_bits))) };
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline constexpr paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::x_axis_count_type 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::convert_from_combined_virtual_address_to_x(virtual_combined_node_adderss_type combined_address)
	{
		//convert from combined to pages then from pages to x index 
		//this would be a lot faster if max_y_axis_pages was a power of 2 
		return (combined_address.address >> combined_address_virtual_memory_map_type::local_address_bits) / max_y_axis_pages;
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline bool paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::is_address_valid(virtual_combined_node_adderss_type address) const
	{
		//get axis this address belongs to 
		auto x_axis_index = convert_from_combined_virtual_address_to_x(address);


		return address < (y_axis_count[x_axis_index] + (x_axis_index * y_axis_virtual_memory_map_type::max_virtual_address));
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline bool paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::is_address_valid(x_axis_count_type x_axis_index, virtual_y_axis_node_adderss_type y_axis_virtual_address) const
	{
		return y_axis_virtual_address < virtual_y_axis_node_adderss_type((y_axis_count[x_axis_index]));
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_real_address_iterator 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::end(x_axis_count_type x_index) const
	{
		return y_real_address_iterator(*this, x_index, virtual_y_axis_node_adderss_type(y_axis_count[x_index]));
	}
	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::y_real_address_iterator 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::begin(x_axis_count_type x_index) const
	{
		auto iterator = y_real_address_iterator(*this, x_index);
		return iterator;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_page_address_iterator 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_only_page_begin(x_axis_count_type x_index) const
	{
		auto iterator = real_page_address_iterator(*this, x_index);
		return iterator;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_page_address_iterator 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_only_page_end(x_axis_count_type x_index) const
	{
		return real_page_address_iterator::get_end_value(*this, x_index);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_and_virtual_page_address_iterator
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::page_begin(x_axis_count_type x_index) const
	{
		auto iterator = real_and_virtual_page_address_iterator(*this, x_index);
		return iterator;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::real_and_virtual_page_address_iterator
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::page_end(x_axis_count_type x_index) const
	{
		return real_and_virtual_page_address_iterator::get_end_value(*this, x_index);
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::all_real_address_iterator
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::begin() const
	{
		auto iterator = all_real_address_iterator(*this);
		iterator.setup_first_value();
		return iterator;
	}

	template<size_t Inumber_of_x_axis_items, size_t Imax_y_items, size_t Imax_total_y_items, size_t Ipage_size>
	inline  paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::all_real_address_iterator 
		paged_2d_array_header<Inumber_of_x_axis_items, Imax_y_items, Imax_total_y_items, Ipage_size>::end() const
	{
		return all_real_address_iterator(*this,y_axis_count.size());
	}
}

