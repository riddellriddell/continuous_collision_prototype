#pragma once
#include <cstdint>
#include <algorithm>
#include <array>
#include <limits>
#include <bit>
#include "array_utilities/SectorPackedArray/paged_memory_header.h"
#include "array_utilities/SectorPackedArray/virtual_memory_map.h"

namespace ArrayUtilities
{
	//function for calculating the number of memory pages needed for the given number of root nodes
	static constexpr size_t calculate_memory_pages_for_wide_linked_list(size_t root_node_count, size_t node_width, size_t max_entries_per_root, size_t max_entries_per_root_group, size_t max_global_entries, size_t page_size, size_t root_node_group_size)
	{
		
		//step 1 put an item in every node to get as many entries as possible to try and fill a memory page
		size_t one_entry_root_nodes_in_group = std::min(std::min(std::min(root_node_group_size, max_entries_per_root_group), max_global_entries), page_size);

		//get the number of entries still available after adding single items to every root
		size_t nodes_left_in_page = page_size - one_entry_root_nodes_in_group;

		size_t nodes_needed_to_force_a_new_node_on_next_page = node_width - 1;

		//lets ignore the max peer root and assume we can spread the elements around the root group to
		//get enough entries to fill the page 
		size_t min_entries_to_fill_page_to_overflow = one_entry_root_nodes_in_group + (nodes_left_in_page * node_width) + nodes_needed_to_force_a_new_node_on_next_page;

		size_t number_of_root_node_groups = root_node_count / root_node_group_size;

		//if every root group had one entry
		size_t pages_taken_up_by_single_entry = std::min(number_of_root_node_groups, max_global_entries);


		size_t entries_remaining = max_global_entries - pages_taken_up_by_single_entry;

		//with the remaining entries work out how many pages you could fill enought to require another page
		size_t filled_to_overflow_pages = entries_remaining / min_entries_to_fill_page_to_overflow;

		//subtract from remaining elements
		entries_remaining -= filled_to_overflow_pages * min_entries_to_fill_page_to_overflow;

		//number of entries needed to for a single root to fill  a page
		size_t entries_per_root_per_page = page_size * node_width;

		//fully fill pages
		size_t fully_filled_pages = entries_remaining / entries_per_root_per_page;

		//the total number of pages needed 
		size_t total_pages_needed = pages_taken_up_by_single_entry + filled_to_overflow_pages + fully_filled_pages;

		return total_pages_needed;
	}

	static constexpr size_t calculate_max_pages_per_root_group(size_t node_width, size_t max_entries_per_root, size_t max_entries_per_root_group, size_t max_global_entries, size_t page_size, size_t root_node_group_size)
	{
		//maximum number of sub nodes if all root nodes are filled to capacity
		//this does not take into account partial fill of nodes to maximize entities per root group 
		//TODO::Calculate this correctly
		size_t max_possible_entities_per_root_group = std::min(std::min(root_node_group_size * node_width * max_entries_per_root, max_entries_per_root_group), max_global_entries);

		return (((max_possible_entities_per_root_group + (node_width - 1)) / node_width ) + (page_size - 1)) / page_size;
	}


	template<
		typename Tdatatype, 
		size_t Iroot_node_count, 
		size_t Inode_width, 
		size_t Imax_entries_per_root, 
		size_t Imax_entries_per_root_group, 
		size_t Imax_global_entries,
		size_t Ipage_size,
		size_t Iroot_node_group_size>
	struct paged_wide_node_linked_list
	{
	private:

		//items in the root and in pages need to be a power of 2 because we use bitshift and masks to 
		//remove the offeset component of an address leaving only the page or root group
		
		//make sure root node count is a power of 2 
		static_assert((Imax_entries_per_root_group& (Imax_entries_per_root_group - 1)) == 0);
		
		//make sure items in page are a power of 2 
		static_assert((Ipage_size& (Ipage_size - 1)) == 0);

		//the maximum number of pages this system will ever need
		static constexpr size_t total_pages = calculate_memory_pages_for_wide_linked_list(Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size);

		static constexpr size_t max_pages_per_root_group = calculate_max_pages_per_root_group(Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size);

		static constexpr size_t total_number_of_nodes = total_pages * Inode_width;

		


		//the address type used
		using address_type = uint32_t;
		using root_node_address_type = uint32_t;
		using real_address_type = uint32_t;
		using page_handle_type = page_handle<uint32_t>;


		//type used to track nodes, must be one more than the max to allow for an invalid value
		using node_link_type = MiscUtilities::uint_s<total_number_of_nodes + 1>::int_type_t;

		//value for an ivalid node
		static constexpr node_link_type invalid_node_address = std::numeric_limits<node_link_type>::max();

		//the bit shift to convert from node address to the page the node is in 
		static constexpr node_link_type node_page_bitshift = std::bit_width(Ipage_size - 1);

		//number type to use for node groups 
		using root_node_group_address_type = MiscUtilities::uint_s<(Iroot_node_count / Iroot_node_group_size) + total_pages>::int_type_t;

		using root_node_virtual_map_type = virtual_memory_map<Iroot_node_group_size, Iroot_node_count / Iroot_node_group_size, total_pages>;

		//the virtual address to use when accessing addresses for root nodex 
		using root_node_virtual_address_type = root_node_virtual_map_type::virtual_address_type;

		static root_node_address_type invalid_root_node_value = std::numeric_limits< root_node_address_type>::max();

		//bitshift needed to convert from root node address to root group
		static constexpr  root_node_group_address_type  extract_root_group_bit_shift = std::bit_width(Iroot_node_group_size - 1);

		static constexpr root_node_group_address_type number_of_root_groups = Iroot_node_count / Iroot_node_group_size;

#pragma region SubStructDefinition

		struct wide_node
		{
			//the invalid data type, this gets used to detect if a node is empty and can be used 
			static constexpr node_link_type invalid_link_value = std::numeric_limits<node_link_type>::max();

			struct active_node
			{
				std::array<Tdatatype, node_width> data;
				node_link_type child_node;
			};

			struct free_node
			{
				struct free_node_links
				{
					node_link_type parent_node;
					node_link_type child_node;
				};

				union
				{
					free_node_links node_links;
					std::array<Tdatatype, node_width> data;
				};

				//structure breaks if the data * width is smaller than the link type
				static_assert(sizeof(std::array<TDataType, node_width>) > sizeof(free_node_links), "size of node links larger than data size, will cause issues with node sizes");

				//this is set to a specific value to indicate that this node is free / not in use
				node_link_type free_node_flag;
			};

			static_assert(sizeof(active_node) == sizeof(free_node), "active node and free nodes should occupy the same space / be the same size");

			union
			{
				active_node active_node_data;
				free_node free_node_data;
			};

			//returns true if there are no entries in this node 
			bool is_free() const
			{
				//if the fist node in the array is invlid then the entire node is empty 
				//which makes it free
				return free_node_data.free_node_flag == invalid_link_value;
			};

			//sets the first value in the wide node to invalid / max value
			//indicating there are no active entries in this wide node and that 
			//it can be used by root nodes to store more values
			void mark_as_free()
			{
				free_node_data.free_node_flag = invalid_link_value;
			}

		};

		struct root_node
		{
			node_link_type active_node; //the last node that is not 100 percent filled that we are adding new items to and we start our read from
			MiscUtilities::uint_s<Inode_width>::int_type_t write_index_in_active_node; //index in the partially filled node to write to
		};


		//each page acts like its own linked list
		//when returning a node it gets reatached to the free list for the page
		//pages also need to 

		//these nodes can also act as sentinals 
		struct page_link_info
		{
			using free_node_count_type = MiscUtilities::uint_s<Ipage_size>::int_type_t;
		 

			struct free_node_address_data
			{
				real_address_type free_node_address;

				free_node_count_type free_node_count;
			};

			struct sentinal_partial_page_data
			{
				//page holding the free nodes
				page_handle_type last_page;
				page_handle_type next_page;
			};

			struct sentinal_full_page_data
			{
				//page holding the free nodes
				page_handle_type last_page;
				page_handle_type next_page;
			};

			sentinal_partial_page_data partial_page_link_info;

			union
			{
				sentinal_full_page_data full_page_link_info;
				free_node_address_data free_node_address_info;
			};

			//is there nothing in any of the nodes in this page 
			void set_as_full_page();

			void set_as_full_page_branchless(bool apply);

			void is_full_page();

			bool has_no_nodes_in_use() const;

			bool has_no_free_nodes() const;

			bool has_valid_partial_page() const;

			bool has_valid_full_page() const;
		};

		struct root_group_info
		{
			//address of active free node page
			page_handle_type partial_page_start;

			//address of the first page attached to this root node group
			page_handle_type full_page_start;

		};

#pragma endregion

		//linked list meta data for all the pages plus data for the sentinal objects 
		std::array<page_link_info, total_pages + number_of_root_groups> page_meta_linked_list;

		//page header to track what memory pages are free
		paged_memory_header<total_pages> page_header;

		//virtual memory map that maps all the sectors to their first page
		root_node_virtual_map_type root_virtual_address_map;

		//array of all the root headers
		//std::array< root_group_info, number_of_root_groups> root_group_info;

		//array of all the page headers
		//std::array< page_link_info, total_pages> per_page_link_list_data;

		//array of pointers to the start of a per page free list
		std::array<root_node, Iroot_node_count > root_node_ptrs;

		//nodes that hold all the data 
		std::array< wide_node, total_number_of_nodes> nodes;
	public:
		//add an item to index x
		address_type add_to_index(root_node_address_type root_node_index, Tdatatype& data_to_add);

		//remove a given value
		void remove_value(root_node_address_type root_node_index, Tdatatype& data_to_remove);

		//return an itterator for looping over all the agents attached to a root index
		real_address_type get_free_node(root_node_virtual_address_type root_node_index);

		void return_node(root_node_virtual_address_type root_node_index, real_address_type address_of_node_to_return);

		static constexpr page_handle_type convert_root_node_to_root_page_handle(root_node_address_type root_node_index);

	};

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::address_type 
		paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::add_to_index(root_node_address_type root_node_index, Tdatatype& data_to_add)
	{
		//check root node is in expected range
		assert(root_node_index < Iroot_node_count, "trying to add to a root node that does not exist / is out of bounds");

		//get the node the root points to 
		root_node& root_node_data = root_node_ptrs[root_node_index];

		//using the root node count get the index to write to
		uint32 index_in_node = root_node_data.write_index + 1;

		//check this item is the first element in a new wide node 
		//which means an empty free node needs to be pulled from the free list
		if (index_in_node == node_width)
		{
			//need to get node off the free list 
			auto node_index = get_free_node();

			assert(nodes[node_index].is_free(), "node pulled from free list is not initializesd to a free state");

			//point this new node to the previous active head node
			nodes[node_index].active_node_data.child_node = root_node_data.write_node;

			//update the node the root is pointing too
			root_node_data.write_node = node_index;

			index_in_node = 0;
		}

		//copy across the data
		nodes[root_node_data.write_node].active_node_data.data[index_in_node] = data;

		//update the write index
		root_node_data.write_index = index_in_node;
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline void paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::remove_value(root_node_address_type root_node_index, Tdatatype& data_to_remove)
	{
		//check root node is in expected range
		assert(root_node_index < Iroot_node_count, "trying to add to a root node that does not exist / is out of bounds");

		//get the node the root points to 
		root_node<TLinkType, node_count>& root_node_data = root_node_ptrs[root_node_index];

		//get the number of nodes in the first half full node
		uint32 first_node_entry_index = root_node_data.write_index;

		uint32 write_node_index = root_node_data.write_node;

		uint32 active_node = write_node_index;

		uint32 active_node_read_index = first_node_entry_index;

		for (; active_node != invalid_node_index;)
		{
			//does node match
			if (nodes[active_node].active_node_data.data[active_node_read_index] == data)
			{
				//copy across lead value
				nodes[active_node].active_node_data.data[active_node_read_index] = nodes[write_node_index].active_node_data.data[first_node_entry_index];

				//check if write node is empty 
				if (!first_node_entry_index)
				{
					//clean up and update the point 
					root_node_data.write_node = nodes[write_node_index].active_node_data.child_node;

					//update the new read index
					//this will be decremented into the actual read index later
					root_node_data.write_index = node_width;

					//return last node to free list 
					return_node(write_node_index);
				}

				root_node_data.write_index--;

				return;
			}

			bool is_last_item_in_node = static_cast<bool>(!active_node_read_index);

			//reset the wide node read index
			active_node_read_index += (node_width * is_last_item_in_node);

			//change read node to child of current node 
			active_node = (active_node * !is_last_item_in_node) + (nodes[active_node].active_node_data.child_node * is_last_item_in_node);

			//decrement index 
			--active_node_read_index;
		}

		assert(false, "only here if no matching node found");
	}



	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::real_address_type
		paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::get_free_node(root_node_virtual_address_type root_node_index)
	{
		//check root node is in expected range
		assert(root_node_index < Iroot_node_count, "trying to add to a root node that does not exist / is out of bounds");

		//get the memory page this node is mapped to 
		page_handle_type root_address_group_handle = convert_root_node_to_root_page_handle(root_node_index);

		//get pointer to active page link list 
		auto& root_group_sentinal = page_meta_linked_list[root_address_group_handle.get_page()];

		page_handle_type free_page_start = root_group_sentinal.partial_page_link_info.next_page;

		//if invalid page link then get new page 
		bool needs_new_free_page = !root_group_sentinal.has_valid_partial_page(); 

		constexpr bool use_branchless_page_update = false;

		if constexpr (use_branchless_page_update)
		{
			free_page_start.branchless_set_handle(page_header.branchless_allocate(needs_new_free_page), needs_new_free_page);

			root_group_sentinal.partial_page_link_info.next_page = needs_new_free_page ? free_page_start : root_group_sentinal.partial_page_link_info.next_page;
			root_group_sentinal.partial_page_link_info.last_page = needs_new_free_page ? free_page_start : root_group_sentinal.partial_page_link_info.last_page;

			auto& new_page_info = page_meta_linked_list[free_page_start.get_page()];

			new_page_info.partial_page_link_info.next_page = needs_new_free_page ? root_address_group_handle : new_page_info.partial_page_link_info.next_page;
			new_page_info.partial_page_link_info.last_page = needs_new_free_page ? root_address_group_handle : new_page_info.partial_page_link_info.last_page;
		}
		else
		{
			if ([[unlikely]] needs_new_free_page)
			{
				free_page_start = page_header.allocate();

				root_group_sentinal.partial_page_link_info.next_page = new_page;
				root_group_sentinal.partial_page_link_info.last_page = new_page;

				auto& new_page_info = page_meta_linked_list[free_page_start.get_page()];

				new_page_info.partial_page_link_info.next_page = root_address_group_handle;
				new_page_info.partial_page_link_info.last_page = root_address_group_handle;
			}
		}

		page_link_info& free_node_source_page = page_meta_linked_list[free_page_start.get_page()];

		//get pointer to first node
		auto page_first_free_node_address = free_node_source_page.free_node_address_info.free_node_address;

		//get the node to return
		wide_node& node_to_return = nodes[page_first_free_node_address];

		//update pointer to first node to point to child
		free_node_source_page.free_node_address_info.free_node_address = node_to_return.free_node_data.child_node;

		//we should have more than 0 nodes at this point 
		//if we dont we will have a wrap arround error
		assert(age_link_info.free_node_count > 0);

		//update count for page 
		--free_node_source_page.free_node_address_info.free_node_count;

		//if page is out of nodes update the root node pointer to point to the next page
		bool page_has_empty_nodes = !free_node_source_page.has_no_free_nodes();

		if constexpr (use_branchless_page_update)
		{
			//get next page or your own page if invalid
			page_handle_type next_partial_page_address = page_has_empty_nodes ? free_page_start : free_node_source_page.partial_page_link_info.next_page;
			page_handle_type last_partial_page_address = page_has_empty_nodes ? free_page_start : free_node_source_page.partial_page_link_info.last_page;

			//un link the next page from this one so that if it ever removes itself it does not break anything
			page_meta_linked_list[next_partial_page_address.get_page()].partial_page_link_info.last_page = free_node_source_page.partial_page_link_info.last_page;
			page_meta_linked_list[last_partial_page_address.get_page()].partial_page_link_info.next_page = free_node_source_page.partial_page_link_info.next_page;

			//update the parent node links
			page_handle_type old_full_next_handle = root_group_sentinal.full_page_link_info.next_page;
			page_link_info& current_next_full_page = page_meta_linked_list[old_full_next_handle.get_page()];

			page_handle_type next_partial_page_address = page_has_empty_nodes ? old_full_next_handle : free_page_start;
			page_handle_type last_partial_page_address = page_has_empty_nodes ? root_address_group_handle : free_page_start;


			page_meta_linked_list[next_partial_page_address.get_page()].full_page_link_info.last_page = root_address_group_handle;
			page_meta_linked_list[last_partial_page_address.get_page()].full_page_link_info.next_page = old_full_next_handle;

			root_group_sentinal.full_page_link_info.next_page    = page_has_empty_nodes? root_group_sentinal.full_page_link_info.next_page		:	free_page_start;
			current_next_full_page.full_page_link_info.last_page = page_has_empty_nodes? current_next_full_page.full_page_link_info.last_page	:	free_page_start;


			//flag this node as now being in the full list so we can correctly move it back to the empty list when a page is returned 
			free_node_source_page.set_as_full_page_branchless(!page_has_empty_nodes);

		}
		else
		{
			//use a branch and pay for a prediction miss
			if ([[unlikely]] page_has_empty_nodes == false)
			{
				//get next page or your own page if invalid
				page_handle_type next_partial_page_address = free_node_source_page.partial_page_link_info.next_page;
				page_handle_type last_partial_page_address = free_node_source_page.partial_page_link_info.last_page;

				//un link the next page from this one so that if it ever removes itself it does not break anything
				page_meta_linked_list[next_partial_page_address.get_page()].partial_page_link_info.last_page = last_partial_page_address;
				page_meta_linked_list[last_partial_page_address.get_page()].partial_page_link_info.next_page = next_partial_page_address;

				//update the parent node links
				page_handle_type old_full_next_handle = root_group_sentinal.full_page_link_info.next_page;
				page_link_info& current_next_full_page = page_meta_linked_list[old_full_next_handle.get_page()];
				
				free_node_source_page.full_page_link_info.last_page = root_address_group_handle;
				free_node_source_page.full_page_link_info.next_page = old_full_next_handle;

				root_group_sentinal.full_page_link_info.next_page = free_page_start;
				current_next_full_page.full_page_link_info.last_page = free_page_start;

				//flag this node as now being in the full list so we can correctly move it back to the empty list when a page is returned 
				free_node_source_page.set_as_full_page();
			}
		}

		//return the address of the new free node 
		return page_first_free_node_address;

	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline void paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::return_node(root_node_virtual_address_type root_node_index, real_address_type address_of_node_to_return)
	{
		//check that root node is in expected range
		assert(root_node < Iroot_node_count);

		//check that node to return is in expected range
		assert(address_of_node_to_return < total_number_of_nodes);

		//get the page the node is in
		node_link_type return_node_page = address_of_node_to_return >> node_page_bitshift;

		//get the info for the page 
		page_link_info& page_info = per_page_link_list_data[return_node_page];

		//check if the page was full
		page_info.

		//add node to free node list 
		wide_node& node_to_return = nodes[address_of_node_to_return];

		//set the child node
		node_to_return.child_node = page_info.free .free_node_address;

		//update the freed address
		page_info.free_node_address = address_of_node_to_return;

		//check if this page was full
		bool was_full_page = page_info.has_no_free_nodes();

		//update the freed node count
		++page_info.free_node_count;

		//check if page had been fully freed and can be returned to the page pool 
		bool all_nodes_returned = page_info.has_no_nodes_in_use();

		if ([[unlikely]] was_full_page)
		{
			//get the root node group
			auto root_address_group = convert_from_root_node_to_root_node_group_address(root_node_index);

			auto& group_info = root_group_info[root_address_group];

			page_handle& last_page = page_info.last_page.is_valid() ? page_info.last_page : group_info.full_page_start;

			//page_handle& next_page = page_info.next_page.is_valid() ? page_info.next_page : group_info.full_page_start;

			//either this is the first page in the group in which case  group_info.full_page_start should be pointing at it 
			//or it is attached to the first page in the full group in which case it should have a valid last page 
			assert(last_page.is_valid());

			//cut out of full group list
			page_link_info& last_page = page_info

				page_info.last_page.get_page();



			group_info.partial_page_start

			//extrack from neighbours and add to free list
			

		}

		if ([[unlikely]] all_nodes_returned)
		{
			//we need to put this page back into the page pool

			//step 1 
		}

		//if returning this node will not make the page empty and this wont be the first node in the free pool for the node 

		//make sure we are getting a valid node index that is in this page
		assert(page_info. != invalid_node_address);
		assert((page_info.free_node_push_address >> node_page_bitshift) == return_node_page);

		//get the last node in the chain
		wide_node& last_free_node_added = nodes[page_info.free_node_push_address];

		//get this node 
		wide_node& node_to_reutrn = nodex[address_of_node_to_return];

		//splice in return node to the free chain
		node_to_reutrn.parent_node = last_free_node_added.parent_node;

		last_free_node_added.parent_node = address_of_node_to_return;

		node_to_reutrn.child_node = page_info.free_node_push_address;

		//update the pointer to oldest node 
		page_info.free_node_push_address = address_of_node_to_return;

		//update the number of free nodes in cell
		page_info.free_node_count++;

		//get the parent root page for the node 
		auto root_page = root_virtual_address_map.resolve_virtual_address_to_page_handle(root_node_index);

		//t
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline constexpr  paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_handle_type 
		paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::convert_root_node_to_root_page_handle(root_node_address_type root_node_index)
	{
		assert(std::numeric_limits<page_handle_type::data_type>::max() < ((root_node_index >> extract_root_group_bit_shift) + total_pages));
		//create page handle type 
		return page_handle_type((root_node_index >> extract_root_group_bit_shift) + total_pages);
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline void paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::set_as_full_page()
	{
		//when full page the partial page values don't get used and when its a partial page
		//this value should never be null as our page link list is circular and should never be null
		partial_page_link_info.last_page = page_handle_type::invalid_page_value();
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline void paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::set_as_full_page_branchless(bool apply)
	{
		//when full page the partial page values don't get used and when its a partial page
		//this value should never be null as our page link list is circular and should never be null
		partial_page_link_info.last_page == apply?  page_handle_type::invalid_page_value(): partial_page_link_info.last_page;
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline void paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::is_full_page()
	{
		return partial_page_link_info.last_page == page_handle_type::invalid_page_value();
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline bool paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::has_no_nodes_in_use() const
	{
		//should auto cast the number to a bool
		return free_node_address_info.free_node_count;
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline bool paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::has_no_free_nodes() const
	{
		//make sure nothing crazy has happened
		assert(free_node_address_info.free_node_count <= Ipage_size);

		return free_node_address_info.free_node_count == Ipage_size;
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline bool paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::has_valid_partial_page() const
	{
		//if this node is pointing to itself then there are no partial pages with free nodes that can be used that are attached to this root
		return partial_page_link_info.next_page != partial_page_link_info.last_page;
	}

	template<typename Tdatatype, size_t Iroot_node_count, size_t Inode_width, size_t Imax_entries_per_root, size_t Imax_entries_per_root_group, size_t Imax_global_entries, size_t Ipage_size, size_t Iroot_node_group_size>
	inline bool paged_wide_node_linked_list<Tdatatype, Iroot_node_count, Inode_width, Imax_entries_per_root, Imax_entries_per_root_group, Imax_global_entries, Ipage_size, Iroot_node_group_size>::page_link_info::has_valid_full_page() const
	{
		return full_page_link_info.next_page != full_page_link_info.last_page;
	}

}