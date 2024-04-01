#pragma once


#include <array>
#include <assert.h>
#include "misc_utilities/int_type_selection.h"

namespace ArrayUtilities
{
    //a helper class that wraps basic arrays and allows us to not have to
    //manage our own std arrays 

    template <typename Tdata_type, std::size_t Isize>
    class fixed_size_vector_array {
    public:
        // Types
        using value_type = Tdata_type;
        using size_type = MiscUtilities::uint_s<Isize>::int_type_t;
        using iterator = typename std::array<Tdata_type, Isize>::iterator;
        using const_iterator = typename std::array<Tdata_type, Isize>::const_iterator;

        // Constructors
        fixed_size_vector_array() = default;
        fixed_size_vector_array(std::initializer_list<Tdata_type> initList) 
        {
            size_ = std::min(initList.size(), Isize);
            std::copy(initList.begin(), initList.begin() + size_, data_.begin());
        }

        // Member functions
        size_type size() const { return size_; }
        size_type max_size() const { return Isize; }

        // Element access
        Tdata_type& operator[](size_type index) { return data_[index]; }
        const Tdata_type& operator[](size_type index) const { return data_[index]; }

        // Iterators
        iterator begin() { return data_.begin(); }
        iterator end() { return data_.begin() + size_; }
        const_iterator begin() const { return data_.begin(); }
        const_iterator end() const { return data_.begin() + size_; }
        const_iterator cbegin() const { return data_.begin(); }
        const_iterator cend() const { return data_.begin() + size_; }

        // Modifiers
        void push_back(const Tdata_type& value) 
        {
            //check we are not about to go off the end of the array
            assert(size_ < Isize);

            data_[size_++] = value;
        }

        //this can help speed up writes by not breaking any branch prediction maybe 
        void branchless_push_back(const Tdata_type& value, const bool apply)
        {
            //check we are not about to go off the end of the array
            assert(size_ < Isize);

            data_[size_] = value;

            size_ += apply;
        }

        //this behaves the same as the above push back but only 
        //increments the number of items if the apply bool is true
        //this is useful if you want to write branchless code and only
        //optionally store a value
        //if you pass in false the next time an item is added it will write over
        //the value
        void push_back(const Tdata_type& value, bool apply_push_back)
        {
            //check we are not about to go off the end of the array
            assert(size_ < Isize);

            data_[size_] = value;

            size_ += apply_push_back;
        }

        void pop_back() 
        {
            size_ -= (size_ > 0);
        }

        void clear() 
        {
            size_ = 0;
        }


    private:
        std::array<Tdata_type, Isize> data_;
        size_type size_ = 0;
    };
}
