#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  //===========================================================================
  class Bulk {//TODO change to class Bank, Bulk, Menu ?
  private:
    template <fmr::Local_int N>
    class alignas (N) Bulk_align {//NOTE does not always work; over-allocate
      //                                 bulk and align manually
    public:
      std::vector <fmr::Bulk_int> bulk;
      std::size_t   size      = 0;// # of values <= sizeof(T) * bulk.capacity()
      fmr::Hash_int file_hash = 0;// CRC32 or CRC64 of stored data
      uint_fast16_t size_of   = 0;// size of each value in bytes
    };
    using Ints_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Data_id, Bulk_align<FMR_ALIGN_VALS>>;
  private:
    Ints_map_t name_ints ={};// size_t alignment
    Vals_map_t name_vals ={};// __m256d, SSE, or AVX512 alignment
  public:
    static constexpr
    uintptr_t offset (uintptr_t, uintptr_t);
    template <typename T>
    T* add (const Data_id& id, const size_t n, const T init_val)
    noexcept;
#if 1
    template <typename T>
    T* add (const Data_id& id, const size_t n, const T* init_vals)
    noexcept;
#endif
    template <typename I>
    //TODO get_safe (..) Check sizeof & sign? get_fast (..) ?
    I* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename V>
    V* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
    template <typename T>
    void del (const Data_id& id)
    noexcept;
  private:
    template <typename I>
    I* add (const Data_id& id, const size_t n=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename V>
    V* add (const Data_id& id, const size_t n=0, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
#if 0
    template <typename To, typename From>
    To* cast (const Data_id& id, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
#endif
  public:
    Bulk () {
      this->name_ints.reserve (1024);
      this->name_vals.reserve (1024);
    }
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
