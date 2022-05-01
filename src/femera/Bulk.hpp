#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  //===========================================================================
  using Bulk_vec_t = std::vector <fmr::Bulk_int>;//TODO rename to Bulk_t ?
  template <fmr::Align_int N>
  class alignas (N) Bulk_vals {//TODO move to Bulk_vals.hpp ===================
  //NOTE alignas does not always work; over-allocate bulk and align manually
  private:
    Bulk_vec_t     bulk;
    std::size_t    size      = 0;// # values <= sizeof(T) * bulk.capacity()
    fmr::Hash_int  file_hash = 0;// CRC32 or CRC64 of data stored in file
    fmr::Align_int size_of   = sizeof (fmr::Bulk_int);// each value in bytes
    bool           is_signed = std::is_signed <fmr::Bulk_int>::value;
  public:
    template <typename T> inline
    std::size_t get_size ()
    noexcept;
    template <typename T> inline
    std::size_t pad_size ()// size including padding
    noexcept;
    template <typename T> inline
    fmr::Align_int get_sizeof ()
    noexcept;
    template <typename T> inline
    bool has_sign ()
    noexcept;
    template <typename T> inline
    T* set (const std::size_t n, const T init_val)
    noexcept;
    static constexpr
    fmr::Align_int offset (uintptr_t)
    noexcept;
    template <typename T>
    T* set (const std::size_t n, const T* init_vals)
    noexcept;
    template <typename T> inline
    T* get_fast (std::size_t start=0)
    noexcept;
    template <typename T> inline
    T* get_safe (std::size_t start=0)
    noexcept;
    template <typename T> inline
    Bulk_vec_t& take_bulk ()// use to move or swap Bulk_vec_t
    noexcept;
  private:
    template <typename I>
    I* raw (const size_t n=0)
    noexcept;
  };//=========================================================================
  class Bulk {//TODO change to class Bank, Bulk ? Swap Bulk with Vals?
  private:
#ifdef FMR_ALIGN_INTS
    using Vec_int_t = Bulk_vals<FMR_ALIGN_INTS>;
#else
    using Vec_int_t = Bulk_vals<alignof (size_t)>;
#endif
#ifdef FMR_ALIGN_INTS
    using Vec_val_t = Bulk_vals<FMR_ALIGN_VALS>;
#else
    using Vec_val_t = Bulk_vals<alignof (double)>;
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
    Bulk ();
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
