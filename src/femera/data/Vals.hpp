#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "Bulk.hpp"

#include <unordered_map>

namespace femera { namespace data {
  class Vals {//No vtable for fast non-static procedural data generation methods
   /*The Vals class is a data container for storing and managing values of
  different types. It is designed for fast non-static procedural data generation
  methods. Here's a summary of what each method does:

  set(id, size_t, T init_val = T(0)): Sets the value of a given identifier (id)
  to a specified initial value (init_val).
  
  set(id, size_t, const T* init_vals): Sets the value of a given identifier (id)
  to values specified in an array (init_vals).
  
  get(id, size_t, typename std::enable_if <std::is_integral ::value>::type* =
  nullptr): Retrieves the value of a given identifier (id) as an integer type
  (I).
  
  get(id, size_t, typename std::enable_if <std::is_floating_point
  ::value>::type* = nullptr): Retrieves the value of a given identifier (id) as
  a floating-point type (F).
  
  del(id): Deletes the value of a given identifier (id).
  
  del_all(): Clears all values stored in the name_vals and name_ints maps and
  returns the number of deleted values.
  
  Vals(): Default constructor.
  
  Vals(fmr::Local_int init_size): Constructor that initializes the object with a
  given initial size (init_size).

  The class uses std::unordered_map to store values, with different types of
  Bulk objects (Bulk_vals and Bulk_ints) depending on alignment settings.
  */
  private:
#ifdef FMR_ALIGN_VALS
    using Bulk_vals = Bulk <FMR_ALIGN_VALS>;
#else
    using Bulk_vals = Bulk <alignof (double)>;
#endif
#ifdef FMR_ALIGN_INTS
    using Bulk_ints = Bulk <FMR_ALIGN_INTS>;
#else
    using Bulk_ints = Bulk <alignof (std::size_t)>;
#endif
    using map_vals_t = std::unordered_map <fmr::Vals_name_t, Bulk_vals>;
    using map_ints_t = std::unordered_map <fmr::Vals_name_t, Bulk_ints>;
  private:
    map_vals_t name_vals = {};// SSE, __m256d,... alignment
    map_ints_t name_ints = {};// default is size_t alignment
    fmr::Local_int map_init_size = 1024;
  public:
    //TODO handle SSE, AVX, AVX512 types
    template <typename T>
    T* set (const fmr::Vals_name_t& id, size_t, T init_val = T(0))
    noexcept;
    template <typename T>// T& is ambiguous as an argument here
    T* set (const fmr::Vals_name_t& id, size_t, const T* init_vals)
    noexcept;
    template <typename I>
    I* get (const fmr::Vals_name_t& id, size_t start=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename F>
    F* get (const fmr::Vals_name_t& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <F>::value>::type* = nullptr)
    noexcept;
    //TODO get_fast (id), get_safe (id), get_cast (id), get_copy (id, T* copy)
    //     take (id, & dest), give (id, & src)
    template <typename T>//TODO Check if template needed.
    void del (const fmr::Vals_name_t& id)
    noexcept;
    fmr::Local_int del_all ()// clears int and float maps; returns # deleted
    noexcept;
  public:
    Vals () noexcept;
    Vals (fmr::Local_int init_size) noexcept;
  };
} }//end femera::data:: namespace

#include "Vals.ipp"

//end FEMERA_DATA_VALS_HPP
#endif
