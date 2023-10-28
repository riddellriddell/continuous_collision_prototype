#pragma once
#include <assert.h>
#include <array>
#include <limits>
#include "base_types_definition.h"
#include "int_type_selection.h"
#include "int_type_selection.h"

namespace ArrayUtilities
{
	template<size_t node_width, typename TLinkType, typename TDataType>
	struct wide_node
	{
		//the invalid data type, this gets used to detect if a node is empty and can be used 
		static constexpr TDataType invalid_data_value = std::numeric_limits<TDataType>::max();
	
		//the number of padding bytes needed to pur the parent node value at the end of the union
		static constexpr size_t free_node_data_padding = sizeof(std::array<TDataType, node_width>) - sizeof(TLinkType);

		struct active_node
		{
			std::array<TDataType, node_width> data;
			TLinkType child_node;
		};
	
		struct free_node
		{
			union
			{
				TDataType free_node_indicator_value; //this get set to invalid value and is an indicator that this node is free
				std::array<uint8, free_node_data_padding> data_padding; //need this value to keep the structure the same alignment 
			};
	
			//structure breaks if the data * width is smaller than the link type
			static_assert(sizeof(std::array<TDataType, node_width>) > sizeof(TLinkType), "size of parent node larger than data size, will cause issues with node sizes");
	
			TLinkType parent_node;
			TLinkType child_node;
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
			return free_node_data.free_node_indicator_value == invalid_data_value;
		};

		//sets the first value in the wide node to invalid / max value
		//indicating there are no active entries in this wide node and that 
		//it can be used by root nodes to store more values
		void mark_as_free()
		{
			free_node_data.free_node_indicator_value = invalid_data_value;
		}
	
	};

	template<typename TLinkType, uint32 Imax_children>
	struct root_node
	{
		TLinkType last_node; //the last node that is not 100 percent filled 
		MiscUtilities::uint_s<Imax_children> count; //how many items are attached to this root node
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
	
		//function to add a value to a root node
		//the root node is the node to add to
		//data is the values to copy
		//node bias is which node to put the data into 
		void add(uint32 root_node_index, TDataType& data);
	
	
		void remove(uint32 root_node_index, TDataType& data);
	
	private:
		TLinkType get_free_node();
	
		//adds node back on the free list 
		void return_node(TLinkType node_index);
	
	};
	
	template<size_t root_node_count, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void ArrayUtilities::wide_node_linked_list<root_node_count, TLinkType, node_count, node_width, TDataType>::add(uint32 root_node_index, TDataType& data)
	{
		//check root node is in expected range
		assert(root_node_index < root_node_count, "trying to add to a root node that does not exist / is out of bounds");
	
		//get the node the root points to 
		root_node& root_node_data = root_node_ptrs[root_node_index];
	
		typedef wide_node<node_width, TLinkType, TDataType> node_type;
	
		//using the root node count get the index in a wide node 
		uint32 index_in_node = root_node_data.count % node_width;

		//check this item is the first element in a new wide node 
		//which means an empty free node needs to be pulled from the free list
		if (!index_in_node)
		{
			//need to get node off the free list 
			auto node_index = get_free_node();

			assert(nodes[node_index].is_free(), "node pulled from free list is not initializesd to a free state");

			//point this new node to the previous active head node
			nodes[node_index].active_node_data.child_node = root_node_data.last_node;

			//update the node the root is pointing too
			root_node_data.last_node = node_index;
		}

		//copy across the data
		nodes[root_node_data.last_node].active_node_data.data[index_in_node] = data;

		//update the number of elements
		root_node_data.count++;
	}
	
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::remove(uint32 root_node_index, TDataType& data)
	{
		//get the node the root points to 
		root_node& root_node_data = root_node_ptrs[root_node_index];

		//get the number of nodes in the first half full node
		uint32 first_node_entry_index = (root_node_data.count -1) % node_width;

		uint32 write_node_index = root_node_data.last_node;

		uint32 active_node = root_node_data.last_node;

		uint32 active_node_read_index = first_node_entry_index;

		for (int i = 0; i < node_count; ++i)
		{
			//update the half fill node
			for (;active_node_read_index >= 0; --active_node_read_index)
			{
				//does node match
				if (nodes[active_node].active_node_data.data[active_node_read_index] == data)
				{
					//coppy across lead value
					nodes[active_node].active_node_data.data[active_node_read_index] = nodes[write_node_index].active_node_data.data[first_node_entry_index];

					//check if write node is empty 
					if (!first_node_entry_index)
					{
						//clean up and update the point 
						root_node_data.last_node = root_node_data.last_node.active_node_data.child_node;
					}

					root_node_data.count--;

					return;
				}
			}

			//reset the wide node read index
			active_node_read_index = (node_width - 1);

			//change read node to child of current node 
			active_node = nodes[active_node].active_node_data.child_node;
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
		free_list_start = nodes[free_node].free_node_data.child;
	
		//return the index of the free node
		return free_node;
	}
	
	template<size_t root_nodes, typename TLinkType, size_t node_count, size_t node_width, typename TDataType>
	inline void wide_node_linked_list<root_nodes, TLinkType, node_count, node_width, TDataType>::return_node(TLinkType node_index)
	{
		//make sure there is nothing in this node 
		assert(nodes[node_index].is_free() == true);
	
		//get the current free node head and point it to this new node
		nodes[free_list_start].free_node_data.parent_node = node_index;
	
		nodes[node_index].free_node_data.child_node = free_list_start;
	
		free_list_start = node_index;
	}

}

