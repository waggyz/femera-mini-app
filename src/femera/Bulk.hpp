#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>
#include <immintrin.h>       // __m256d

namespace femera { namespace data {
  //===========================================================================
  class Bulk {//TODO change to Bank.hpp; or class Bank, Bulk, Menu ?
  private:
    template <typename A>
    //TODO std::shared_ptr<std::vector<fmr::Bulk_int>> ? needed if std::move ?
    struct alignas (A) Bulk_align { std::vector<fmr::Bulk_int> bulk; };
    //TODO need 1 of each map/omp thrd?
    using Ints_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_VALS>>;
  private:
    Ints_map_t name_ints ={};// size_t alignment
    Vals_map_t name_vals ={};//__m256d, SSE, or AVX512 alignment
  public:
    template <typename I>
    I* add (const Data_id& id, const size_t n, const I& init_val,
      typename std::enable_if<std::is_integral<I>::value>::type* = nullptr)
    noexcept;
    template <typename I>
    I* add (const Data_id& id, const size_t n=0,
      typename std::enable_if<std::is_integral<I>::value>::type* = nullptr)
    noexcept;
#if 1
    template <typename I>
    I* get (const Data_id& id,// includes char type
      typename std::enable_if<std::is_integral<I>::value>::type* = nullptr)
    noexcept;
    template <typename V>
    V* get (const Data_id& id,
      typename std::enable_if<std::is_floating_point<V>::value>::type* = nullptr)
    noexcept;
#endif
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
