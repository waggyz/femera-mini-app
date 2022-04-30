#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  //===========================================================================
  using Bulk_vec_t = std::vector <fmr::Bulk_int>;//TODO rename to Bulk_t ?
  template <fmr::Align_int N>
  class alignas (N) Bulk_vals {//NOTE does not always work; over-allocate
  public://TODO make private   //bulk vector and align manually
    Bulk_vec_t    bulk;
    std::size_t   size         = 0;// # values <= sizeof(T) * bulk.capacity()
    fmr::Hash_int file_hash    = 0;// CRC32 or CRC64 of data stored in file
    std::uint_fast16_t size_of = 0;// size of each value in bytes
    bool has_sign              = false;
  public:
    template <typename T> inline
    T* set (const std::size_t n, const T init_val)
    noexcept;
    static constexpr
    fmr::Align_int offset (uintptr_t);
    template <typename T>
    T* set (const std::size_t n, const T* init_vals)
    noexcept;
  private:
    template <typename I>
    I* raw (const size_t n=0)
    noexcept;
#if 0
    template <typename T> inline
    T* Bulk_vals<N>::get (std::size_t start, typename
      std::enable_if <std::is_integral <T>::value>::type*)
    noexcept;
    template <typename T> inline
    T* Bulk_vals<N>::get (std::size_t start, typename
      std::enable_if <std::is_floating_point <T>::value>::type*)
    noexcept;
#endif
  };
  
  
  
  
  
  class Bulk {//TODO change to class Bank, Bulk ? Swap Bulk with Vals?
  private:
    using Ints_map_t = std::unordered_map <Data_id, Bulk_vals<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Data_id, Bulk_vals<FMR_ALIGN_VALS>>;
  private:
    Ints_map_t name_ints ={};// size_t alignment
    Vals_map_t name_vals ={};// SSE, __m256d, or AVX512 alignment
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
  private:
    static constexpr
    fmr::Align_int offset (uintptr_t, fmr::Align_int);
  public:
    Bulk ();
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
