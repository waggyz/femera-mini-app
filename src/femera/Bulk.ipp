#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP
#include <stddef.h>
#include <stdint.h>
 #include <nmmintrin.h>

namespace femera { namespace data {
  template <fmr::Align_int A>
  template <typename T> inline constexpr
  fmr::Align_int Bulk<A>::offset (const std::uintptr_t address)
  noexcept {
    return ((address % std::max (A, alignof(T))) == 0) ? 0// range: [0, A-1]
      : (std::max (A, alignof(T)) - (address % std::max (A, alignof(T))));
  }
  template <fmr::Align_int A>
  template <typename T> inline
  std::size_t Bulk<A>::mem_size ()
  noexcept {
    return this->bulk.size ();
  }
  template <fmr::Align_int A>
  template <typename T> inline
  std::size_t Bulk<A>::get_size ()
  noexcept {
    return this->size;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  fmr::Align_int Bulk<A>::get_sizeof ()
  noexcept {
    return this->size_of;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  bool Bulk<A>::has_sign ()
  noexcept {
    return this->is_signed;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  Bulk_t& Bulk<A>::take_bulk ()
  noexcept {
    this->size = 0;
    return this->bulk;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T* Bulk<A>::raw (const std::size_t nT)
  noexcept {
    if (nT <= 0) {
      this->bulk      ={};// new empty vec or clear existing
      this->size      = 0;
      this->size_of   = sizeof (T);
      this->is_signed = std::is_signed <T>::value;
      return reinterpret_cast<T*> (this->bulk.data());
    }
    // over-allocate for aligned access
    const auto sz = (nT * sizeof (T) + 2 * (std::max (A, alignof(T)) - 1))
      / sizeof (fmr::Bulk_int);
    // sz is also large enough for aligned SIMD access at the tail
    this->bulk.reserve (sz);// uninitialized, bulk.size()==0
    this->size      = nT;
    this->size_of   = sizeof (T);
    this->is_signed = std::is_signed <T>::value;
    auto ptr        = this->bulk.data ();
#if 0
    this->bulk [0] = fmr::Bulk_int (0);// undefined behavior
    // Accessesing a vector past size() is undefined behavior...
#endif
    // ...but accessing the reserved space in the underlying array should be OK.
    ptr [0] = fmr::Bulk_int (0);// first touch
    return reinterpret_cast<T*> (ptr);
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T* Bulk<A>::set (const std::size_t nT, const T init_val)
  noexcept {
    this->raw<T> (nT);
    if (nT <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = nT * sizeof (T) / sizeof (fmr::Bulk_int);
    const auto lpad = this->offset<T> (ptr);// max left  padding is A-1
    const auto rpad = this->offset<T> (sz );// max right padding is A-1
    // Zero is the same bits for all numeric types (data/Vals.gtst.cpp).
    if (init_val <= T(0) && init_val >= T(0)) {// == 0.0, no float warning
      this->bulk.resize (lpad + sz + rpad);// <= capacity (); ptr still valid
      return reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    }
    // nonzero init
#if 0
    // Initialize from a temporary vector of type T.
    // This will not work with padding.
    const auto tmp = std::vector<T> (nT, init_val);
    const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
    this->bulk.assign (ptr, ptr + sz);
#endif
#if 0
    // Initialize without making a tmp vector. 10x slower than 0init.
    if (lpad > 0) {
      for (size_t i=0; i<lpad; i++) {this->bulk.push_back (fmr::Bulk_int(0));}
    }
    const auto bytes = reinterpret_cast <const fmr::Bulk_int*> (& init_val);
    const auto mod   = fmr::Local_int (sizeof (T) / sizeof (fmr::Bulk_int));
    for (size_t i=0; i<sz; i++ ) {this->bulk.push_back (bytes [i % mod]);}
#endif
    auto v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    this->bulk.resize (lpad + sz + rpad);// <= capacity (); ptr still valid
    FMR_PRAGMA_OMP_SIMD
    for (size_t i=0; i<nT; i++) {v [i] = init_val;}// 10% slower than 0init
    return v;
  }
  template <fmr::Align_int A>
  template <typename T>
  T* Bulk<A>::set (const std::size_t nT, const T* init_vals)
  noexcept {
    this->raw<T> (nT);
    if (nT <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = (nT * sizeof (T)) / sizeof (fmr::Bulk_int);
    const auto lpad = this->offset<T> (ptr);// max padding is A-1
    const auto rpad = this->offset<T> (sz );// max padding is A-1
    auto v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    if (lpad == 0) {// bulk.assign(..) if already aligned when allocated
      const auto from = reinterpret_cast<const fmr::Bulk_int*> (init_vals);
      this->bulk.assign (from, from + sz);
      if (rpad > 0) {
        for (size_t i=0; i<rpad; i++) {
          this->bulk.push_back (fmr::Bulk_int (0));
    } } } else {    // zero-initialize then copy elements
      this->bulk.resize (lpad + sz + rpad);// <= capacity(); v, ptr still valid
      FMR_PRAGMA_OMP_SIMD
      for (std::size_t i=0; i<nT; i++) {v [i] = init_vals [i];}
    }
    return v;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T* Bulk<A>::get_fast (std::size_t start)
  noexcept {
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
    return & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [start];
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T* Bulk<A>::get_safe (std::size_t start)
  noexcept {
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
    if ((start + 1) * sizeof (T) <= lpad + this->bulk.size ()) {
      return & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [start];
    }
    return nullptr;
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T Bulk<A>::make_hash (T in,
    typename std::enable_if <sizeof(T) == 4>::type*)// CRC32
  noexcept {
    uint64_t crc = ~in;
    auto sz = this->size * this->size_of;
    auto buf = reinterpret_cast<const unsigned char*> (& this->bulk);
    while (sz--) {
#if 1//def FMR_HAS_64BITCRC32
      crc = _mm_crc32_u64 (crc, *buf++);
#endif
#ifdef FMR_HAS_32BITCRC32
      crc = _mm_crc32_u32 (crc, *buf++);
#endif
#if 0
      crc ^= *buf++;
      for (int i = 0; i < 8; i++) {
#if 0
        crc = crc & 1 ? (crc >> 1) ^ FMR_CRC32_POLY : crc >> 1;
#endif
        crc = (crc >> 1) ^ (FMR_CRC32_POLY & (0 - (crc & 1)));
      }
#endif
    }
    return ~T (crc);
  }
  template <fmr::Align_int A>
  template <typename T> inline
  T Bulk<A>::make_hash (T crc,
    typename std::enable_if <sizeof(T) == 8>::type*)// CRC64
  noexcept {// https://stackoverflow.com/questions/27939882/fast-crc-algorithm
    crc = ~crc;
    std::size_t sz = this->size * this->size_of;
    auto buf = reinterpret_cast<const unsigned char*> (& this->bulk);
    while (sz--) {
      crc ^= *buf++;
      for (int i = 0; i < 8; i++) {
        crc = (crc >> 1) ^ (FMR_CRC64_POLY & (0 - (crc & 1)));
    } }
    return ~crc;
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
