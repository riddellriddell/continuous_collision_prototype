#pragma once
#include <assert.h>
#include <array>
#include <limits>
#include "misc_utilities/int_type_selection.h"
#include "../base_types_definition.h"
#include "vector_2d_math_utils/byte_vector_2d.h"



namespace ArrayUtilities
{
	template<size_t node_width, typename TLinkType, typename TDataType>
	struct wide_node
	{
		//the invalid data type, this gets used to detect if a node is empty and can be used 
		static constexpr TLinkType invalid_link_value = std::numeric_limits<TLinkType>::max();

		struct active_node
		{
			std::array<TDataType, node_width> data;
			TLinkType child_node;
		};
	
		struct free_node
		{
			struct free_node_links
			{
				TLinkType parent_node;
				TLinkType child_node;
			};

			union
			{
				free_node_links node_links;
				std::array<TDataType, node_width> data;
			};
	
			//structure breaks if the data * width is smaller than the link type
			static_assert(sizeof(std::array<TDataType, node_width>) > sizeof(free_node_links), "size of node links larger than data size, will cause issues with node sizes");

			//this is set to a specific value to indicate that this node is free / not in use
			TLinkType free_node_flag;
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

	template<typename TLinkType, uint32 Imax_children>
	struct root_node
	{
		TLinkType write_node; //the last node that is not 100 percent filled 
		MiscUtilities::uint_s<Imax_children>::int_type_t write_index; //how many items are attached to this root node
	};

	//the idea of this class is that most entries often are inside a size 
	//pathological logical case where a single 
	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType >
	class wide_node_linked_list
	{
		

	public:
		static constexpr TLinkType invalid_node_index = std::numeric_limits<TLinkType>::max();
	
		//this link list can hold multiple start nodes 
		std::array<root_node<TLinkType, node_count>, root_node_count> root_node_ptrs = {};
	
		//the start of any unused nodes 
		TLinkType free_list_start;
	
		//all the nodes 
		std::array<wide_node<node_width, TLinkType, TDataType>, node_count > nodes;
	
		//construcotr
		constexpr wide_node_linked_list();

		//reset the state to of the linked list
		void reset();

		//function to add a value to a root node
		//the root node is the node to add to
		//data is the values to copy
		void add(uint32 root_node_index, TDataType data);
	
		
		void remove(uint32 root_node_index, TDataType data);

		struct iterator
		{

			typedef wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType> parent_list_type;
			
			static constexpr TLinkType invalid_node_index = std::numeric_limits<TLinkType>::max();
			
			uint32 read_index;
			uint32 node_index;
			
			//pointer to wide node array 
			std::array<wide_node<node_width, TLinkType, TDataType>, node_count >* nodes;
			
			// Define the iterator dereference operator.
			TDataType& operator*() const
			{
				return (*nodes)[node_index].active_node_data.data[read_index];
			}
			
			void next()
			{
				//check if we are at the last in the node
				bool last_in_node = !read_index;
			
				//if we are the last in the node change the ndde we are pointing to 
				node_index = (node_index * !last_in_node) + ((*nodes)[node_index].active_node_data.child_node * last_in_node);
			
				//go to next index in node or if it is the last node reset read index to node width -1 
				read_index = (read_index + (node_width * last_in_node)) -1;
			}
			
			// Define the iterator pre-increment operator.
			iterator& operator++() {
				next();
				return *this;
			}
			
			// Define the iterator equality operators.
			bool operator==(const iterator & other) const {
				return (read_index == other.read_index) && (node_index == other.node_index);
			}
			
			bool operator!=(const iterator & other) const {
				return (read_index != other.read_index) || (node_index != other.node_index);
			}
			
			iterator(parent_list_type& parent_list, TLinkType root_node_index) :
				read_index((parent_list.root_node_ptrs[root_node_index].write_index)),
				node_index(parent_list.root_node_ptrs[root_node_index].write_node),
				nodes(&(parent_list.nodes))
			{
			}
			
			//end index itterator
			iterator() :read_index(node_width -1), node_index(invalid_node_index),nodes(nullptr) {}
		};
	
		//get an iterator to iterate through all the data items in a root node 
		iterator get_root_node_start(TLinkType root_node_index);

		//end itterator 
		iterator end();
	private:

		//reset all the pointers to root nodes 
		void reset_root_nodes();

		//correctly setup the free node data
		void setup_free_node_list();

		//get a node off the free node list
		TLinkType get_free_node();
	
		//adds node back on the free list 
		void return_node(TLinkType node_index);
	
	};


	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline constexpr wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::wide_node_linked_list()
	{
		reset();
	}

	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::reset()
	{
		setup_free_node_list();
		reset_root_nodes();
	}

	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::iterator wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::get_root_node_start(TLinkType root_node_index)
	{
		auto& itterator_target = *this;
		return iterator(itterator_target,root_node_index);
	}

	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::iterator wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::end()
	{
		return wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::iterator();
	}

	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::reset_root_nodes()
	{
		for (int i = 0; i < root_node_count; ++i)
		{
			root_node_ptrs[i].write_node = invalid_node_index;
			root_node_ptrs[i].write_index = node_width -1;
		}
	}

	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::setup_free_node_list()
	{
		//nodes are organized in a loop so removing a node is the same even if its the first or last node
		free_list_start = 0;

		//do the first node
		nodes[0].free_node_data.node_links.child_node = 1;
		nodes[0].free_node_data.node_links.parent_node = node_count - 1;
		nodes[0].mark_as_free();

		//do all the middle nodex
		for (int i = 1; i < (node_count -1); ++i)
		{
			nodes[i].free_node_data.node_links.child_node = i + 1;
			nodes[i].free_node_data.node_links.parent_node = i - 1;
			nodes[i].mark_as_free();
		}
		
		//do the last node
		nodes[node_count -1].free_node_data.node_links.child_node = 0;
		nodes[node_count -1].free_node_data.node_links.parent_node = node_count - 2;
		nodes[node_count -1].mark_as_free();
	}


	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void ArrayUtilities::wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::add(uint32 root_node_index, TDataType data)
	{
		//check root node is in expected range
		assert(root_node_index < root_node_count, "trying to add to a root node that does not exist / is out of bounds");
	
		//get the node the root points to 
		root_node<TLinkType, node_count>& root_node_data = root_node_ptrs[root_node_index];
	
		typedef wide_node<node_width, TLinkType, TDataType> node_type;
	
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
	
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::remove(uint32 root_node_index, TDataType data)
	{
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
			active_node = (active_node * !is_last_item_in_node) + ( nodes[active_node].active_node_data.child_node * is_last_item_in_node);

			//decrement index 
			--active_node_read_index;
		}

		assert(false, "only here if no matching node found");
	}
	
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline TLinkType wide_node_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::get_free_node()
	{
		//check that there are free nodes 
		assert(free_list_start != invalid_node_index);
	
		//store the free node
		TLinkType free_node = free_list_start;
	
		//update the free list start
		free_list_start = nodes[free_node].free_node_data.node_links.child_node;

		//correctly link up the next node 
		nodes[free_list_start].free_node_data.node_links.parent_node = nodes[free_node].free_node_data.node_links.parent_node;
		nodes[nodes[free_list_start].free_node_data.node_links.parent_node].free_node_data.node_links.child_node = free_list_start;
	
		//return the index of the free node
		return free_node;
	}
	
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::return_node(TLinkType node_index)
	{
	
		auto existing_parent = nodes[free_list_start].free_node_data.node_links.parent_node;

		//get the current free node head and point it to this new node
		nodes[free_list_start].free_node_data.node_links.parent_node = node_index;
	
		nodes[node_index].mark_as_free();

		nodes[node_index].free_node_data.node_links.child_node = free_list_start;

		//link the new node to the other end of the free node loop  
		//free nodes are linked in a loop so if a free node is detected at an index
		//it can be removed from the free list without breaking the free list chain.
		//this is node so an active node can just check if a free node exists at the next available index instead of getting a node from the start of the 
		//queue which might not be in contigious memory 
		nodes[node_index].free_node_data.node_links.parent_node = existing_parent;

		nodes[existing_parent].free_node_data.node_links.child_node = node_index;
		

		free_list_start = node_index;
	}

}

