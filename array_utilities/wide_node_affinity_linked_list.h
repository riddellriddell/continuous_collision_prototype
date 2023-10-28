#pragma once
#include <assert.h>
#include <array>
#include <limits>
#include "base_types_definition.h"
#include "int_type_selection.h"

namespace ArrayUtilities
{
	template<size_t node_width, typename TLinkType, typename TDataType>
	struct affinity_wide_node
	{
		typedef uint8 usage_flag_type;

		struct active_node
		{
			std::array<TDataType, node_width> data;
			TLinkType child_node;
			usage_flag_type usage_flags;
		};

		struct free_node
		{
			union
			{
				std::array<TDataType, node_width> data; //need this value to keep the structure the same alignment 
				
				TLinkType parent_node;
			};

			//structure breaks if the data is less than the size of the next node pointer
			static_assert(sizeof(data) > sizeof(parent_node), "size of parent node larger than data size, will cause issues with node sizes");

			TLinkType child_node;
			usage_flag_type usage_flags;
		};

		union
		{
			active_node active_node_data;
			free_node free_node_data;
		};
		
	};

	//the idea of this class is that most entries often are inside a size 
	//pathological logical case where a single 
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType >
	class wide_node_affinity_linked_list
	{
		static constexpr TLinkType invalid_node_index = std::numeric_limits<TLinkType>::max();

		//this link list can hold multiple start nodes 
		std::array<TLinkType, root_nodes> root_node_ptrs = {};

		//the start of any unused nodes 
		TLinkType free_list_start;

		//all the nodes 
		std::array<affinity_wide_node<node_width, TLinkType, TDataType>, node_count > nodes;

		//function to add a value to a root node
		//the root node is the node to add to
		//data is the values to copy
		//node bias is which node to put the data into 
		void add(uint32 root_node, TDataType& data, uint32 node_index_affinity);


		void remove(uint32 root_node, TDataType& data, uint32 node_index_affinity);

	private:
		TLinkType get_free_node();

		//adds node back on the free list 
		void return_node(TLinkType node_index);

	};

	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void ArrayUtilities::wide_node_affinity_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::add(uint32 root_node, TDataType& data, uint32 node_index_affinity)
	{
		//check root node is in expected range
		assert(root_node < root_nodes);

		//check that affinity is in expected range
		assert(node_index_affinity < node_width);

		//get the node the root points to 
		TLinkType node_index = root_node_ptrs[root_node];

		typedef affinity_wide_node<node_width, TLinkType, TDataType> node_type;

		//calculate the bitflag to check for occupation in a list 
		uint8 usage_flag = 1 << node_index_affinity;

		//check if node exists 
		if (node_index == invalid_node_index)
		{
			//need to get node off the free list 
			node_index = get_free_node();

			//since this is the fist usage of this node we dont have to worry about overwriting anything
			nodes[node_index].active_node_data.data[node_index_affinity] = data;
			nodes[node_index].active_node_data.child_node = invalid_node_index;
			nodes[node_index].usage_flags = usage_flag;

			root_node_ptrs[root_node] = node_index;

			return;
		}

		while (true)
		{
			//check usage 
			if (nodes[node_index].usage_flags & usage_flag)
			{
				//check if has valid child 
				if (nodes[node_index].active_node_data.child_nod == invalid_node_index)
				{
					auto new_node_index = get_free_node();
					//need to get node off the free list 
					nodes[node_index].active_node_data.child_node = new_node_index;

					//since this is the fist usage of this node we dont have to worry about overwriting anything
					nodes[new_node_index].active_node_data.data[node_index_affinity] = data;
					nodes[new_node_index].active_node_data.child_node = invalid_node_index;
					nodes[new_node_index].usage_flags = usage_flag;

					return;
				}

				//point to next node
				node_index = nodes[node_index].active_node_data.child_node;

			}
			else
			{
				nodes[node_index].active_node_data.data[node_index_affinity] = data;
				nodes[node_index].usage_flags |= usage_flag;

				return;
			}
		}

	}

	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_affinity_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::remove(uint32 root_node, TDataType& data, uint32 node_index_affinity)
	{
		//get the node the root points to 
		TLinkType node_containing_target = root_node_ptrs[root_node];
		TLinkType* parent_node = &root_node_ptrs[root_node];
		
		//find the node with the index
		while (nodes[node_containing_target].active_node_data.data[node_index_affinity] != data)
		{
			parent_node = &nodes[node_containing_target].child_node;
			node_containing_target = nodes[node_containing_target].child_node;
		}

		//loop through child nodes and find last node with data in index
		TLinkType* node_last_at_index = parent_node;

		//calculate the bitflag to check for occupation in a list 
		uint8 usage_flag = 1 << node_index_affinity;

		//get the last node with data at index
		while (
			nodes[*node_last_at_index].active_node_data.child_node != invalid_node_index &&
			(nodes[nodes[*node_last_at_index].child_node].active_node_data.usage_flags & usage_flag))
		{
			node_last_at_index = &nodes[*node_containing_target].child_node;
		}

		//copy the data from the last index
		nodes[*parent_node] = nodes[*node_last_at_index];

		//check if last node has anything left in it
		if (!nodes[*node_last_at_index].active_node_data.usage_flags)
		{
			return_node(*node_last_at_index);
		}
	}

	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline TLinkType wide_node_affinity_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::get_free_node()
	{
		//check that there are free nodes 
		assert(free_list_start != invalid_node_index);
		
		//store the free node
		TLinkType free_node = free_list_start;
		
		//update the free list start
		free_list_start = nodes[free_node].free_node_data.child;

		//return the index of the free node
		return free_node;
	}

	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_affinity_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::return_node(TLinkType node_index)
	{
		//make sure there is nothing in this node 
		assert(nodes[node_index].active_node_data.usage_flags == 0);

		//get the current free node head and point it to this new node
		nodes[free_list_start].free_node_data.parent_node = node_index;

		nodes[node_index].free_node_data.child_node = free_list_start;

		free_list_start = node_index;
	}

}

