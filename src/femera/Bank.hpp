#ifndef FEMERA_DATA_BANK_HPP
#define FEMERA_DATA_BANK_HPP

#include "Bulk.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  class Bank {//TODO Swap Bank with Vals?
  private:
#ifdef FMR_ALIGN_INTS
    using Vec_int_t = Bulk<FMR_ALIGN_INTS>;
#else
    using Vec_int_t = Bulk<alignof (size_t)>;
#endif
#ifdef FMR_ALIGN_INTS
    using Vec_val_t = Bulk<FMR_ALIGN_VALS>;
#else
    using Vec_val_t = Bulk<alignof (double)>;
#endif
  private:
    std::unordered_map<Data_id, Vec_int_t> name_ints ={};// size_t alignment
    std::unordered_map<Data_id, Vec_val_t> name_vals ={};// SSE,__m256d,...align
  public:
    //TODO handle SSE, AVX, AVX512 types
    template <typename T>
    T* set (const Data_id& id, const size_t n, const T init_val=T(0))
    noexcept;
    template <typename T>
    T* set (const Data_id& id, const size_t n, const T* init_vals)
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
#if 0
    static constexpr
    fmr::Align_int offset (uintptr_t, fmr::Align_int)//TODO Remove?
    noexcept;
#endif
  public:
    Bank ();
  };
} }//end femera::data:: namespace

#include "Bank.ipp"

//end FEMERA_DATA_BANK_HPP
#endif
