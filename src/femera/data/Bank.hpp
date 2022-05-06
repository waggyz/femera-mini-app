#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "Bulk.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  class Bank {//TODO Swap Bank with Vals?
  private:
#ifdef FMR_ALIGN_VALS
    using Bulk_vals = Bulk <FMR_ALIGN_VALS>;
#else
    using Bulk_vals = Bulk <alignof (double)>;
#endif
#ifdef FMR_ALIGN_INTS
    using Bulk_ints = Bulk <FMR_ALIGN_INTS>;
#else
    using Bulk_ints = Bulk <alignof (size_t)>;
#endif
  private:
    std::unordered_map<Data_id, Bulk_vals> name_vals ={};// SSE,__m256d,...align
    std::unordered_map<Data_id, Bulk_ints> name_ints ={};// size_t alignment
    fmr::Local_int map_init_size = 1024;
  public:
    //TODO handle SSE, AVX, AVX512 types
    template <typename T>
    T* set (const Data_id& id, size_t n, T init_val=T(0))
    noexcept;
    template <typename T>
    T* set (const Data_id& id, size_t n, const T* init_vals)// T& is ambiguous
    noexcept;
    template <typename I>
    I* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename V>
    V* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
    //TODO get_fast (id), get_safe (id), get_cast (id), get_copy (id, T* copy)
    //     take (id, & dest), give (id, & src)
    template <typename T>
    void del (const Data_id& id)
    noexcept;
    fmr::Local_int del_all ()// clears int and float maps; returns # deleted
    noexcept;
  public:
    Bank ();
    Bank (const fmr::Local_int init_size);
  };
} }//end femera::data:: namespace

#include "Bank.ipp"

//end FEMERA_DATA_BANK_HPP
#endif