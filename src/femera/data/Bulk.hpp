#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/math.hpp"

#include <vector>
// below used in Bulk.ipp
#include <stddef.h>
#include <stdint.h>
#include <immintrin.h>       // _mm_crc32_u64

namespace femera { namespace data {
  namespace zyc = ::zyclops;
  //
  using Bulk_t = std::vector <fmr::Bulk_int>;
  //
  template <fmr::Align_int A>
  class alignas (A) Bulk {//NOTE alignas does not always work, so...
  private:                //
    Bulk_t         bulk;  // ...bulk is over-allocated and aligned manually.
    // Defaults are for Bulk_int values which can be converted to strings.
    std::size_t    cval_n       = 0;// # values <= bulk.capacity() / sizeof(T)
    zyc::Zomplex   zplx         = std::is_signed <fmr::Bulk_int>::value
      ?                           zyc::Integer : zyc::Natural;
    fmr::Hash_int  file_hash    = 0;// CRC32 or CRC64 of data read from file
    fmr::Align_int cval_szof    = sizeof (fmr::Bulk_int);// native value (bytes)
    bool           cval_sign_tf = std::is_signed <fmr::Bulk_int>::value;
  public:
    bool has_sign ()// sign of currently stored integer values
    noexcept;
    std::size_t get_byte ()// memory for storing numbers w/out padding (bytes)
    noexcept;
    std::size_t mem_byte ()// amount of memory used including padding (bytes)
    noexcept;
    //
    fmr::Align_int get_sizeof ()// sizeof each currently stored native (C++)
    noexcept;                   // numbers (int, float, etc.) in bytes
    std::size_t get_cval_n ()// count of currently stored native (C++) numbers
    noexcept;
    std::size_t get_size ()// count of currently stored (hypercomplex) numbers
    noexcept;              // same as get_cval_n () for order 0
    //
    template <typename T>
    T* set_real (std::size_t nvals, T init_val)// sets real part only
    noexcept;
    template <typename T>
    T* set_real (zyc::Layout, zyc::Algebra, zyc::Zorder_int,
      std::size_t hc_vals_n, T real_part)
    noexcept;
    template <typename T>
    T* set (zyc::Layout, zyc::Algebra, zyc::Zorder_int,
      std::size_t hc_vals_n, T real_part)
    noexcept;
    template <typename T>
    T* set (std::size_t nvals, T init_val)// sets real part only
    noexcept;
    template <typename T>
    T* set (std::size_t nvals, const T* init_vals)
    noexcept;
    template <typename T>
    T* get_fast (std::size_t start = 0)
    noexcept;
    template <typename T>
    T* get_safe (std::size_t start = 0)
    noexcept;
    Bulk_t& take_bulk ()// use to move or swap bulk member variable
    noexcept;
    //
    template <typename H>
    H make_hash (H init_hash = 0,
      typename std::enable_if <sizeof (H) == 4>::type* = nullptr)
    noexcept;
    template <typename H>
    H make_hash (H init_hash = 0,
      typename std::enable_if <sizeof (H) == 8>::type* = nullptr)
    noexcept;
  public:
    Bulk () noexcept;// constructor
  private:
    template <typename T> static constexpr
    T ce_max (T a, T b);
    template <typename T> static
    fmr::Align_int offset (uintptr_t)
    noexcept;
    template <typename T>
    std::size_t raw (std::size_t nT=0)// returns bytes allocated
    noexcept;
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
