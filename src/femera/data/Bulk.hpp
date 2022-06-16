#ifndef FEMERA_DATA_BULK_HPP
#define FEMERA_DATA_BULK_HPP

#include "../../fmr/fmr.hpp"
#include "../../fmr/math.hpp"

#include <vector>
#include <unordered_map>

namespace femera { namespace data {
  namespace zyc = ::zyclops;
  //
  using Bulk_t = std::vector <fmr::Bulk_int>;
  //
  template <fmr::Align_int A>
  class alignas (A) Bulk {//NOTE alignas does not always work, so...
  private:                //
    Bulk_t         bulk;  // ...bulk is over-allocated and aligned manually.
    std::size_t    size      = 0;// # values <= bulk.capacity() / sizeof(T)
    // Defaults below are for storing Bulk_int values.
    zyc::Zomplex   zomplex   = std::is_signed <fmr::Bulk_int>::value
      ? zyc::Integer : zyc::Natural;
    fmr::Hash_int  file_hash = 0;// CRC32 or CRC64 of data read from file
    fmr::Align_int size_of   = sizeof (fmr::Bulk_int);// native value in bytes
    zyc::Stored    zlayout   = zyc::Stored::Native;
    bool           is_signed = std::is_signed <fmr::Bulk_int>::value;
  public:
    template <typename T> inline
    fmr::Align_int get_sizeof ()// sizeof currently stored native (C++) type
    noexcept;
    template <typename T> inline
    std::size_t get_size ()// number of currently stored native (C++) values
    noexcept;
    template <typename T> inline
    std::size_t zyc_size ()// number of hypercomplex values
    noexcept;
    template <typename T> inline
    std::size_t mem_byte ()// size including padding (bytes)
    noexcept;
    template <typename T> inline
    bool has_sign ()// sign of currently stored integer values
    noexcept;
    template <typename T> inline
    T* set (std::size_t nvals, T init_val)// sets real part only
    noexcept;
    template <typename T>
    T* set (std::size_t nvals, const T* init_vals)
    noexcept;
    template <typename T> inline
    T* get_fast (std::size_t start = 0)
    noexcept;
    template <typename T> inline
    T* get_safe (std::size_t start = 0)
    noexcept;
    Bulk_t& take_bulk ()// use to move or swap bulk member variable
    noexcept;
    template <typename H> inline
    H make_hash (H init_hash = 0,
      typename std::enable_if <sizeof (H) == 4>::type* = nullptr)
    noexcept;
    template <typename H> inline
    H make_hash (H init_hash = 0,
      typename std::enable_if <sizeof (H) == 8>::type* = nullptr)
    noexcept;
    //
    Bulk () noexcept;// Constructor
  private:
    template <typename T> inline static constexpr
    fmr::Align_int offset (uintptr_t)
    noexcept;
    template <typename T>
    T* raw (size_t nT=0)
    noexcept;
  };
} }//end femera::data:: namespace

#include "Bulk.ipp"

//end FEMERA_DATA_BULK_HPP
#endif
