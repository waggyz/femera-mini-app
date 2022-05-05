#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "../Data.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  using Bulk_t = std::vector <fmr::Bulk_int>;
  template <fmr::Align_int A>
  class alignas (A) Bulk {
  //NOTE alignas does not always work; over-allocate bulk and align manually
  private:
    Bulk_t         bulk;
    std::size_t    size      = 0;// # values <= sizeof(T) * bulk.capacity()
    fmr::Hash_int  file_hash = 0;// CRC32 or CRC64 of data stored in file
    fmr::Align_int size_of   = sizeof (fmr::Bulk_int);// each value in bytes
    bool           is_signed = std::is_signed <fmr::Bulk_int>::value;
  public:
    template <typename T> inline static constexpr
    fmr::Align_int offset (uintptr_t)
    noexcept;
    template <typename T> inline
    std::size_t get_size ()// size of data
    noexcept;
    template <typename T> inline
    std::size_t mem_size ()// size including padding
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
    Bulk_t& take_bulk ()// use to move or swap Bulk_t
    noexcept;
    template <typename T> inline
    T make_hash (T init_hash=T(0),
      typename std::enable_if <sizeof(T) == 4>::type* = nullptr)
    noexcept;
    template <typename T> inline
    T make_hash (T init_hash=T(0),
      typename std::enable_if <sizeof(T) == 8>::type* = nullptr)
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
