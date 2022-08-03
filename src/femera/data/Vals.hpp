#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "Bulk.hpp"

#include <unordered_map>

namespace femera { namespace data {//TODO move to fmr::data:: namespace?
  class Vals {//No vtable for fast non-static procedural data generation methods
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
    template <typename V>
    V* get (const fmr::Vals_name_t& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
    //TODO get_fast (id), get_safe (id), get_cast (id), get_copy (id, T* copy)
    //     take (id, & dest), give (id, & src)
    template <typename T>
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
