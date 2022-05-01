#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  using Bulk_vec_t = std::vector <fmr::Bulk_int>;//TODO rename to Bank_t ?
  template <fmr::Align_int N>
  class alignas (N) Bulk {//TODO move to Bank.hpp
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
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
