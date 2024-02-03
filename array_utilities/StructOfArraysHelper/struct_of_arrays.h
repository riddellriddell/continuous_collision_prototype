#pragma once

#include <iostream>
#include <tuple>
#include <array>

//this class serves as utility for creating and managing structures that contain arrays where a single index in all the arrays represents a single object 

namespace ArrayUtilities
{
    template< typename Tderived_type>
    struct base_ref_type 
    {
       // do a check to make sure any type that inherits from this has a function called get tuple that 
        // returns an object that is the same size as the struct 
        //this it to make sure you never add a type without adding it to the tuple 
    };

	struct example_soa
	{
		int* x;
		int* y;
		bool* valid;

        auto get_as_tuple()
        {
            return std::tie(x, y, valid);
        }
	};


    //take a tuple and use a function to process each element and return the result as a new tuple
    struct tuple_converter
    {
        template<typename Tconversion_type>
        struct type_conversion_object
        {       
            using type = Tconversion_type;

            type default_type;
        };

        template<std::size_t Iarray_size>
        struct convert_to_array
        {
            template<typename Ttype_to_convert>
            auto operator()(Ttype_to_convert& type_to_convert) const
            {
                //catch the case of us trying to make an array of references
                return std::array<Ttype_to_convert, Iarray_size>{};
            }
        };

        struct convert_from_container_to_ref
        {
            uint32_t index_to_reference;

            template<typename Ttype_to_convert>
            auto operator()(Ttype_to_convert& type_to_convert) const
            {
                //get iterator from container 
                auto iterator = type_to_convert.begin();

                //move to index we want
                iterator += index_to_reference;

                auto& ref_to_value = *iterator;

                //catch the case of us trying to make an array of references
                return std::ref(ref_to_value);
            }
        };


        struct convert_from_container_to_value
        {
            uint32_t index_to_reference;

            template<typename Ttype_to_convert>
            auto operator()(Ttype_to_convert& type_to_convert) const
            {
                //get iterator from container 
                auto iterator = type_to_convert.begin();

                //move to index we want
                iterator += index_to_reference;

                return *iterator;
            }
        };
        
        template < typename Ttuple_to_convert, typename Tconverstion_object,std::size_t... Indices>
        static auto convert_internal(Ttuple_to_convert& target_to_convert, Tconverstion_object&& conversion_function, std::index_sequence<Indices...>)
        {
            return std::make_tuple(conversion_function(std::get<Indices>(target_to_convert))...);
        }

        template<typename Ttuple_to_convert, typename Tconverstion_func>
        static auto convert(Ttuple_to_convert& target_to_convert, Tconverstion_func&& conversion_function )
        {
            return convert_internal(target_to_convert, std::forward<Tconverstion_func&>(conversion_function), std::make_index_sequence<std::tuple_size_v<typename std::remove_reference<Ttuple_to_convert>::type>>());
        }
    };

    struct tuple_transferer
    {
    private:
        template <typename... Args, std::size_t... Is>
        static constexpr void transfer_internal(const auto& source,auto& destination, std::index_sequence<Is...>) 
        {
            // Use fold expression to transfer references
            ((std::get<Is>(destination) = std::get<Is>(source)), ...);
        }

    public:

        template <typename... Args>
        static constexpr void transfer(const auto& source, auto& destination)
        {
            constexpr std::size_t Size = sizeof...(Args);
            transferReferencesImpl(source, destination, std::make_index_sequence<Size>());
        }
    };

    template<typename Treference_struct>
    struct struct_of_arrays_helper
    {
    private:
        template <typename TupleType, std::size_t N>
        struct ConvertTupleToArrayOfArrays
        {
            template <std::size_t... Indices>
            static auto convert(std::index_sequence<Indices...>)
            {
                return std::make_tuple(std::array<
                    typename std::remove_pointer<
                    typename std::remove_reference<
                    std::tuple_element_t<Indices, TupleType>>::type>::type
                    , N>{}...);
            }

            static auto convert()
            {
                return convert(std::make_index_sequence<std::tuple_size_v<TupleType>>());
            }
        };

    public:
        //the underlying tuple type
        using ref_tuple_type = decltype(std::declval<Treference_struct>().get_as_tuple());

        //the array of tuples type
        template<size_t Iarray_size> 
        using tuple_of_arrays_type = decltype(ConvertTupleToArrayOfArrays< ref_tuple_type, Iarray_size>::convert());

    private:
        // Helper function to set pointers in the tuple
        static void set_pointer_to_index(auto& pointer, auto& array, auto index)
        {
            //using iterators so this will work with datatypes other than arrays or std arrays
            pointer = &*(array.begin() + index);
        }

        template <class Ttuple1, class Ttuple2, std::size_t... I>
        static void point_pointer_struct_to_index_impl(Ttuple1&& ref_tuple, Ttuple2&& tuple_of_arrays, auto index, std::index_sequence<I...>)
        {
            //do a fold over all items and transfer pointer values
             (
                 set_pointer_to_index(
                     std::get<I>(std::forward<Ttuple1>(ref_tuple)),
                     std::get<I>(std::forward<Ttuple2>(tuple_of_arrays)),
                     index),
                 ...
             );
        }


    public:
        //point an instance of the reference struct to all the array values of the tuple of arrays 
        template<typename TtupleOfArrays, typename Tindex_type>
        static void point_pointer_struct_to_index(Treference_struct& reference_struct, TtupleOfArrays& tuple_of_arrays, Tindex_type index)
        {
          //get tuple that points to all the elements in the reference struct
          auto ref_tuple = reference_struct.get_as_tuple();
          
          using ref_tuple_type = decltype(ref_tuple);

          static constexpr size_t tuple_size = std::tuple_size<ref_tuple_type>::value;

          point_pointer_struct_to_index_impl(
              std::forward<ref_tuple_type&>(ref_tuple),
              std::forward<TtupleOfArrays&>(tuple_of_arrays),
              index,
              std::make_index_sequence<tuple_size>());
        }
   
    private:
        // Helper function template to create an instance of MyStruct from a tuple
        template <typename... Args>
        static Treference_struct create_ref_struct_from_tuple(const std::tuple<Args...>& tuple)
        {
            return { std::get<Args>(tuple)... };
        }

        //template<typename TtupleOfArrays, typename Tindex_type, std::size_t... I>
        //static Treference_struct create_reference_struct_to_index_impl(TtupleOfArrays& tuple_of_arrays, Tindex_type index,std::index_sequence<I...>)
        //{
        //    return create_ref_struct_from_tuple (tuple_converter::convert(tuple_of_arrays, tuple_converter::convert_from_container_to_ref{ index }));
        //}

    public:

        template<typename TtupleOfArrays, typename Tindex_type>
        static Treference_struct create_reference_struct_to_index(TtupleOfArrays& tuple_of_arrays, Tindex_type index)
        {
            //create a tuple of references to the index in the tuple of arrays we want to return
            auto ref_tuple = tuple_converter::convert(tuple_of_arrays, tuple_converter::convert_from_container_to_ref{index});

            //convert the tuple to a reference struct by converting it to a value pack and passing it to the constructor
            return create_ref_struct_from_tuple(ref_tuple);
        }

    private:
        template<std::size_t... I>
        static void transfer_values_internal(ref_tuple_type& from, ref_tuple_type& to, auto index, std::index_sequence<I...>)
        {
            //do a fold over all items and transfer pointer values
            (((*std::get<I>(to)) = (*std::get<I>(from))), ...);
        }
    public:
        static void transfer_values(ref_tuple_type& from, ref_tuple_type& to)
        {
            static constexpr size_t tuple_size = std::tuple_size<ref_tuple_type>::value;


            auto tuple_index_pack = std::make_index_sequence<tuple_size>();

            transfer_values_internal(std::forward<ref_tuple_type&>(from),
                std::forward<ref_tuple_type&>(to),
                tuple_index_pack);
                
        }

        //transfers values from one reference struct to another
        static void transfer_values(Treference_struct& from, Treference_struct& to)
        {
            //get the tuple for both typs
            auto from_tuple = from.get_as_tuple();
            auto to_tuple = to.get_as_tuple();

            //transfer the values over using the tuples 
            transfer_values(from_tuple, to_tuple);
        }
    };

    template<typename Ttuple_of_iterable_objects>
	struct struct_of_arrays
	{
        //a tuple containting a bunch of array like data items that can be iterated over
        Ttuple_of_iterable_objects tuple_of_arrays;

        //a wrapper for a tuple that allows assignment opperators to work correctly
        template <typename... Ttypes>
        struct transferable_tuple: public std::tuple<Ttypes...>
        {
            //constructor
            transferable_tuple(const std::tuple<Ttypes...>& tpl) : std::tuple<Ttypes...>(tpl) {}

            //override the = operator to do a per element assignment 
            void operator = (const auto& assign_from)
            {
                tuple_transferer(this, assign_from);
            }
        };

        
        //iterator to manipulate data
       struct random_iterator
       {
           //standard iterator implementation
           using iterator_category = std::random_access_iterator_tag;
           using difference_type = std::ptrdiff_t;
           
           //data to iterate over
           Ttuple_of_iterable_objects& tuple_to_iterate;

           //index of the current item
           uint32_t index;

           random_iterator(Ttuple_of_iterable_objects& tuple_to_iterate, uint32_t index) : tuple_to_iterate{ tuple_to_iterate }, index{ index } {};

           //standard iterator implementation
           random_iterator operator++() { ++index; return *this; }

           random_iterator operator++(int) { random_iterator retval = *this; ++(*this); return retval; }

           bool operator==(random_iterator other) const { return index == other.index; }

           bool operator!=(random_iterator other) const { return !(*this == other); }

           auto operator*() const
           {
               //create a tuple of values at the index we want to return
               //wrap the results in a transferable tuple so you do *iterator = *iterator
               // return Ttuple_of_iterable_objects(tuple_converter::convert(tuple_to_iterate, tuple_converter::convert_from_container_to_ref{ index }));
               tuple_converter::convert_from_container_to_ref converter{ index };

               return tuple_converter::convert(tuple_to_iterate, converter);
           }

           random_iterator operator+=(difference_type offset) { index += offset; return *this; }

           random_iterator operator+(difference_type offset) const { random_iterator retval = *this; return retval += offset; }

           friend random_iterator operator+(difference_type offset, random_iterator other) { return other + offset; }

           random_iterator& operator-=(difference_type offset) { return *this += -offset; }

           random_iterator operator-(difference_type offset) const { return *this + -offset; }

           difference_type operator-(random_iterator other) const { return index - other.index; }

           auto operator[](difference_type offset) const { return *(*this + offset); }

           //same as above code but using the spaceship operator
           auto operator<=>(random_iterator other) const { return index <=> other.index; }
       };
       
	
       uint32_t size()
       {
           return std::get<0>(tuple_of_arrays).size();
       }

       random_iterator begin()
       {
           return random_iterator(tuple_of_arrays, 0);
       }
       random_iterator end()
       {
           return random_iterator(tuple_of_arrays, size());
       }
   
    };


    template<typename Treference_struct, std::size_t Iarray_size>
    struct struct_of_arrays_with_ref_struct
    {
        //length of the arrays 
        static constexpr uint32_t num() { return Iarray_size; };

        using ref_tuple_type = struct_of_arrays_helper<Treference_struct>::ref_tuple_type;

        using tuple_array_type = struct_of_arrays_helper<Treference_struct>::tuple_of_arrays_type<Iarray_size>;

        tuple_array_type tuple_of_arrays;
    };



}