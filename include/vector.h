#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

namespace aico
{

constexpr unsigned int DYNAMIC = 0;
struct empty 
{
    template <typename...types>
    empty(types...){}
    
    template <typename...types>
    empty& operator=(types...){return *this;}
    
    template <typename T>
    operator T()const{return T(0);}
};

template <typename T, size_t dim = DYNAMIC, bool inlined = 0>
requires (!(inlined && dim == DYNAMIC))
class vector
{
    template<typename D, size_t size, bool inl>
    requires (!(inl && size == DYNAMIC))
    friend class vector;

    [[no_unique_address]] std::conditional_t<inlined, empty, bool> 
        ownsData = true;
    [[no_unique_address]] std::conditional_t<dim == DYNAMIC, size_t, empty> 
        dynamicSize;
    alignas(inlined?alignof(T):alignof(T*)) 
        std::conditional_t<inlined, char[sizeof(T)*dim], T*> data;
public:
    /*SIZE*/

    /// @return Number of elements in the vector evaluated at compile time.
    inline constexpr size_t size()const requires(dim != DYNAMIC){return dim;}
    /// @return Number of elements in the vector fetched at runtime. 
    inline size_t size()const requires(dim == DYNAMIC){return dynamicSize;}

    /*CONSTRUCTOR*/

    vector(){}

    /*INDEXING*/
    
        /*STATIC*/

    template<size_t idx> 
        inline T& at()
            requires(dim!=DYNAMIC)
    {
        static_assert(idx<dim, "out of bounds");
        return ((T*)this->data)[idx];
    }
    template<size_t idx>
        inline const T& at()
            const requires(dim!=DYNAMIC)
    {
        static_assert(idx<dim, "out of bounds");
        return ((T*)this->data)[idx];
    }

        /*DYNAMIC*/

    inline T& at(size_t idx)
    {
        assert(idx<this->size() && "out of bounds");
        return ((T*)this->data)[idx];
    }
    inline const T& at(size_t idx) const
    {
        assert(idx<this->size() && "out of bounds");
        return ((T*)this->data)[idx];
    }

        /*SUBSCRIPT*/

    inline const T& operator[](size_t idx) const {return at(idx);}
    inline       T& operator[](size_t idx)       {return at(idx);}
        
    /*ITERATOR*/

    inline const T* begin()const{return (T*)data;}
    inline       T* begin()     {return (T*)data;}
    inline const T* end()const{return this->begin()+this->size();} //T* arithmetic
    inline       T* end()     {return this->begin()+this->size();}

};

}; //namespace aico















