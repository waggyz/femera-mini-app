#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>
#include <immintrin.h>       // __m256d

namespace femera { namespace data {
  //===========================================================================
  class Bulk {//TODO change to class Bank, Bulk, Menu ?
  private:
    template <fmr::Local_int N>
    class alignas (N) Bulk_align {//FIXME does not work; over-allocate and use
      //                                  std::align or pointer math
    private:
    public:
      std::vector <fmr::Bulk_int> bulk;
      std::size_t   size      = 0;// number of values
      fmr::Hash_int file_hash = 0;// CRC32 or CRC64 of stored data
      uint_fast16_t size_of   = 0;// size of each value in bytes
    };
#if 0
    template <typename A>
    struct alignas (A) Bulk_align {
      std::vector <fmr::Bulk_int> bulk;
    };
#endif
    using Ints_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_VALS>>;
    //TODO need 1 of each map/omp thrd?
  public:
    template <typename T>
    T* add (const Data_id& id, const size_t n, const T init_val)
    noexcept;
    template <typename I>
    I* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename I>
    I* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <I>::value>::type* = nullptr)
    noexcept;
  private:
    Ints_map_t    name_ints ={};// size_t alignment
    Vals_map_t    name_vals ={};// __m256d, SSE, or AVX512 alignment
  private:
    template <typename I>
    I* add (const Data_id& id, const size_t n=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename I>
    I* add (const Data_id& id, const size_t n=0, typename
      std::enable_if <std::is_floating_point <I>::value>::type* = nullptr)
    noexcept;
#if 0
    template <typename To, typename From>
    To* cast (const Data_id& id, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
#endif
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
