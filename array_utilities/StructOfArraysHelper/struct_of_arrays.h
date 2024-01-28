#pragma once

#include <iostream>
#include <tuple>
#include <array>

//this class serves as utility for creating and managing structures that contain arrays where a single index in all the arrays represents a single object 

namespace ArrayUtilities
{
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

        template < typename Ttuple_to_convert, typename Tconverstion_object,std::size_t... Indices>
        static auto convert_internal(Ttuple_to_convert& target_to_convert, Tconverstion_object& conversion_function, std::index_sequence<Indices...>)
        {
            return std::make_tuple(conversion_function(std::get<Indices>(target_to_convert))...);
        }

        template<typename Ttuple_to_convert, typename Tconverstion_func>
        static auto convert(Ttuple_to_convert& target_to_convert, Tconverstion_func& conversion_function )
        {
            return convert_internal(target_to_convert, conversion_function, std::make_index_sequence<std::tuple_size_v<std::remove_reference<Ttuple_to_convert>::type>>());
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
                    std::remove_pointer<
                    typename std::remove_reference<
                    std::tuple_element_t<Indices, TupleType>>::type>::type
                    , N>{}...);
            }

            static auto convert()
            {
                return convert(std::make_index_sequence<std::tuple_size_v<TupleType>>());
            }
        };

        inline static Treference_struct ref_struct{};
    public:
        //the underlying tuple type
        using ref_tuple_type = decltype(ref_struct.get_as_tuple());

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
        static void point_reference_struct_to_index_impl(Ttuple1&& ref_tuple, Ttuple2&& tuple_of_arrays, auto index, std::index_sequence<I...>)
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
        static void point_reference_struct_to_index(Treference_struct& reference_struct, TtupleOfArrays& tuple_of_arrays, Tindex_type index)
        {
          //get tuple that points to all the elements in the reference struct
          auto ref_tuple = reference_struct.get_as_tuple();
          
          using ref_tuple_type = decltype(ref_tuple);

          static constexpr size_t tuple_size = std::tuple_size<ref_tuple_type>::value;

          point_reference_struct_to_index_impl(
              std::forward<ref_tuple_type&>(ref_tuple),
              std::forward<TtupleOfArrays&>(tuple_of_arrays),
              index,
              std::make_index_sequence<tuple_size>());
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

    template<typename Treference_struct, std::size_t Iarray_size>
	struct struct_of_arrays
	{
        //length of the arrays 
        static constexpr uint32_t num() { return Iarray_size; };

        using ref_tuple_type = struct_of_arrays_helper<Treference_struct>::ref_tuple_type;

        using tuple_array_type = struct_of_arrays_helper<Treference_struct>::tuple_of_arrays_type<Iarray_size>;

        tuple_array_type tuple_of_arrays;
	};

}