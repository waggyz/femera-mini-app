#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  //===========================================================================
  class Bulk {//TODO change to class Bank, Bulk ?
    private:
      using Bulk_vec_t = std::vector <fmr::Bulk_int>;
  private:
    template <fmr::Align_int N>
    class alignas (N) Bulk_vals {//NOTE does not always work; over-allocate
      //                                bulk vector and align manually
    public:
      Bulk_vec_t    bulk;
      std::size_t   size      = 0;// # of values <= sizeof(T) * bulk.capacity()
      fmr::Hash_int file_hash = 0;// CRC32 or CRC64 of stored data
      uint_fast16_t size_of   = 0;// size of each value in bytes
      bool has_sign           = false;
    };
    using Ints_map_t = std::unordered_map <Data_id, Bulk_vals<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Data_id, Bulk_vals<FMR_ALIGN_VALS>>;
  private:
    Ints_map_t name_ints ={};// size_t alignment
    Vals_map_t name_vals ={};// SSE, __m256d, or AVX512 alignment
  public:
    //TODO handle SSE, AVX, AVX512 types
    static constexpr
    fmr::Align_int offset (uintptr_t, fmr::Align_int);
    template <typename T>
    T* add (const Data_id& id, const size_t n, const T init_val)
    noexcept;
#if 1
    template <typename T>
    T* add (const Data_id& id, const size_t n, const T* init_vals)
    noexcept;
#endif
    template <typename I>
    I* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_integral <I>::value>::type* = nullptr)
    noexcept;
    template <typename V>
    V* get (const Data_id& id, size_t start=0, typename
      std::enable_if <std::is_floating_point <V>::value>::type* = nullptr)
    noexcept;
    //TODO get_fast (id), get_safe (id), get_cast (id), get_copy (id, T* copy)
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
  public:
    Bulk ();
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
