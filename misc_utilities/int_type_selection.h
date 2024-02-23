#pragma once
#include <stdint.h>

namespace MiscUtilities
{
    template<unsigned long long Max>
    struct RequiredBits
    {
        enum {
            value =
            Max <= 0xff ? 8 :
            Max <= 0xffff ? 16 :
            Max <= 0xffffffff ? 32 :
            64
        };
    };

    template<int bits> struct SelectInteger_ {};
    template<> struct SelectInteger_<0> { typedef uint8_t  int_type_t; };
    template<> struct SelectInteger_<8>  { typedef uint8_t  int_type_t; };
    template<> struct SelectInteger_<16> { typedef uint16_t int_type_t; };
    template<> struct SelectInteger_<32> { typedef uint32_t int_type_t; };
    template<> struct SelectInteger_<64> { typedef uint64_t int_type_t; };


    template<uint64_t Max>
    struct uint_s 
    {
        typedef typename SelectInteger_<static_cast<int>(RequiredBits<Max>::value)>::int_type_t int_type_t;
    };

}