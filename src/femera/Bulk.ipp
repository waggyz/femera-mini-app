#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <fmr::Align_int N> inline constexpr
  fmr::Align_int Bulk<N>::offset (const std::uintptr_t address)
  noexcept {
    return ((address % N) == 0 ) ? 0 : (N - (address % N));// range: [0, N-1]
  }
  template <fmr::Align_int N>
  template <typename T> inline
  std::size_t Bulk<N>::pad_size ()
  noexcept {
    return this->bulk.size ();
  }
  template <fmr::Align_int N>
  template <typename T> inline
  std::size_t Bulk<N>::get_size ()
  noexcept {
    return this->size;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  fmr::Align_int Bulk<N>::get_sizeof ()
  noexcept {
    return this->size_of;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  bool Bulk<N>::has_sign ()
  noexcept {
    return this->is_signed;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  Bulk_vec_t& Bulk<N>::take_bulk ()
  noexcept {
    this->size = 0;
    return this->bulk;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk<N>::raw (const std::size_t n)
  noexcept {
    if (n <= 0) {
      this->bulk      ={};// new empty vec or clear existing
      this->size      = 0;
      this->size_of   = sizeof (T);
      this->is_signed = std::is_signed <T>::value;
      return reinterpret_cast<T*> (this->bulk.data());
    }
    // over-allocate for aligned access
    const auto sz = (n * sizeof (T) + 2 * (N - 1)) / sizeof (fmr::Bulk_int);
    // sz is also large enough for aligned SIMD access at the tail
    this->bulk.reserve (sz);// uninitialized, bulk.size()==0
    this->size      = n;
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
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk<N>::set (const std::size_t n, const T init_val)
  noexcept {
    this->raw<T> (n);
    if (n <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
    const auto lpad = this->offset (ptr);// max left  padding is N-1
    const auto rpad = this->offset (sz );// max right padding is N-1
    // Zero is the same bits for all numeric types (data/Vals.gtst.cpp).
    if (init_val <= T(0) && init_val >= T(0)) {// == 0.0, no float warning
      this->bulk.resize (lpad + sz + rpad);// <= capacity (); ptr still valid
      return reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    }
    // nonzero init
#if 0
    // Initialize from a temporary vector of type T.
    // This will not work with padding.
    const auto tmp = std::vector<T> (n, init_val);
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
    for (size_t i=0; i<n; i++) {v [i] = init_val;}// 10% slower than 0init
    return v;
  }
  template <fmr::Align_int N>
  template <typename T>
  T* Bulk<N>::set (const std::size_t n, const T* init_vals)
  noexcept {
    this->raw<T> (n);
    if (n <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = (n * sizeof (T)) / sizeof (fmr::Bulk_int);
    const auto lpad = this->offset (ptr);// max padding is N-1
    const auto rpad = this->offset (sz );// max padding is N-1
    auto v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    if (lpad == 0) {// bulk.assign(..) works if already aligned when allocated
      const auto from = reinterpret_cast<const fmr::Bulk_int*> (init_vals);
      this->bulk.assign (from, from + sz);
      if (rpad > 0) {
        for (size_t i=0; i<rpad; i++) {
          this->bulk.push_back (fmr::Bulk_int (0));
    } } } else {    // zero-initialize then copy elements
      this->bulk.resize (lpad + sz + rpad);// <= capacity(); v, ptr still valid
      for (std::size_t i=0; i<n; i++) {v [i] = init_vals [i];}
    }
    return v;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk<N>::get_fast (std::size_t start)
  noexcept {
    const auto lpad = this->offset (std::uintptr_t (this->bulk.data ()));
    return & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [start];
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk<N>::get_safe (std::size_t start)
  noexcept {
    const auto lpad = this->offset (std::uintptr_t (this->bulk.data ()));
    if ((start + 1) * sizeof (T) <= lpad + this->bulk.size()) {
      return & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [start];
    }
    return nullptr;
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
