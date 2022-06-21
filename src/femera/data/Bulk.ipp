#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <fmr::Align_int A> inline
  Bulk<A>::Bulk ()
  noexcept {
  }
  template <fmr::Align_int A> template <typename T> inline constexpr
  fmr::Align_int Bulk<A>::offset (const std::uintptr_t address)
  noexcept {
    return ((address % std::max (A, alignof(T))) == 0)// range: [0, A-1]
      ? 0 : (std::max (A, alignof(T)) - (address % std::max (A, alignof(T))));
  }
  template <fmr::Align_int A> inline
  std::size_t Bulk<A>::get_byte ()
  noexcept {
    return this->cval_n * this->cval_szof;
  }
  template <fmr::Align_int A> inline
  std::size_t Bulk<A>::mem_byte ()
  noexcept {
    return this->bulk.capacity ();
  }
  template <fmr::Align_int A> inline
  std::size_t Bulk<A>::get_cval_n ()
  noexcept {
    return this->cval_n;
  }
  template <fmr::Align_int A> inline
  std::size_t Bulk<A>::get_size ()
  noexcept {
    return this->cval_n / this->zplx.hc_size ();
  }
  template <fmr::Align_int A> inline
  fmr::Align_int Bulk<A>::get_sizeof ()
  noexcept {
    return this->cval_szof;
  }
  template <fmr::Align_int A> inline
  bool Bulk<A>::has_sign ()
  noexcept {
    return this->cval_szof;
  }
  template <fmr::Align_int A> inline
  Bulk_t& Bulk<A>::take_bulk ()
  noexcept {
    this->cval_n = 0;
    return this->bulk;
  }
  template <fmr::Align_int A> template <typename T> inline
  std::size_t Bulk<A>::raw (const std::size_t nT)// uninit., w/space for nT vals
  noexcept {
    this->zplx = std::is_floating_point <T>::value ? ::zyclops::Real
      : (std::is_signed <T>::value ? ::zyclops::Integer : ::zyclops::Natural);
    this->cval_szof = sizeof (T);
    this->cval_sign_tf = std::is_signed <T>::value;
    if (nT <= 0) {
      this->bulk ={};// new empty vec or clear existing
      this->cval_n = 0;
#if 0
      return reinterpret_cast<T*> (this->bulk.data());
#endif
      return this->bulk.capacity ();
    }
    // over-allocate for aligned access
    const auto sz = (nT * sizeof (T) + 2 * (std::max (A, alignof(T)) - 1))
      / sizeof (fmr::Bulk_int);
    // sz is also large enough for aligned SIMD access at the tail
    this->bulk.reserve (sz);// uninitialized, bulk.size()==0
    this->cval_n = nT;
#if 0
    this->bulk [0] = fmr::Bulk_int (0);// undefined behavior
    // Accessesing a vector past size() is undefined behavior...
#endif
    // ...but accessing reserved space in the underlying array should be OK.
    this->bulk.data () [0] = fmr::Bulk_int (0);// first touch allocate
    return this->bulk.capacity ();
  }
  template <fmr::Align_int A> template <typename T> inline
  T* Bulk<A>::set (const std::size_t nvals, const T init_val)
  noexcept {
    return this->set_real (nvals, init_val);
  }
  template <fmr::Align_int A> template <typename T> inline
  T* Bulk<A>::set_real (const std::size_t nvals, const T init_val)
  noexcept {
    const auto zsz = zplx.hc_size ();
    this->raw<T> (nvals * zsz);
    if (nvals <= 0) { return reinterpret_cast<T*> (this->bulk.data ()); }
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = (nvals * sizeof(T) * zsz) / sizeof(fmr::Bulk_int);
    const auto lpad = this->offset<T> (ptr);// max left  padding is A-1
    const auto rpad = this->offset<T> (sz );// max right padding is A-1
    // nonzero init
#if 0
    // Initialize from a temporary vector of type T.
    // This will not work with padding.
    const auto tmp = std::vector<T> (nvals, init_val);
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
    this->bulk.resize (lpad + sz + rpad);// <= capacity (); ptr still valid
    /* Resize sets to zero, and zero is the same bits for all numeric types.
       This is checked in Vals.gtst.cpp. */
    if (init_val <= T(0) && init_val >= T(0)) {// == 0.0, no float warning
      return reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    }
    FMR_ALIGN_PTR v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    if ((zsz > 1) && (this->zplx.get_layout () == zyclops::Layout::Inset)) {
      const auto nz = nvals * zsz;
      FMR_PRAGMA_OMP_SIMD
      for (size_t i=0; i<nz; i+=zsz) { v [i] = init_val; }
      return v;
    }
    FMR_PRAGMA_OMP_SIMD
    for (size_t i=0; i<nvals; i++) { v [i] = init_val; }// 10% slower than 0init
    return v;
  }
  template <fmr::Align_int A> template <typename T> inline
    T* Bulk<A>::set
    (zyclops::Layout lay, zyclops::Algebra alg, zyclops::Zorder_int zord,
      std::size_t hc_vals_n, T real_part)
  noexcept {
    return this->set_real (lay, alg, zord, hc_vals_n, real_part);
  }
  template <fmr::Align_int A> template <typename T> inline
    T* Bulk<A>::set_real
    (zyclops::Layout lay, zyclops::Algebra alg, zyclops::Zorder_int o,
      std::size_t hc_vals_n, T real_part)
  noexcept {
    this->zplx.order  = o;
    this->zplx.family = alg;
    this->zplx.layout = lay;
    return this->set_real (hc_vals_n, real_part);
  }
  template <fmr::Align_int A> template <typename T> inline
  T* Bulk<A>::set (const std::size_t nvals, const T* init_vals)
  noexcept {
    const auto zsz = this->zplx.hc_size ();
    this->raw<T> (nvals * zsz);
    if (nvals <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = (nvals * sizeof(T) * zsz) / sizeof(fmr::Bulk_int);
    const auto lpad = this->offset<T> (ptr);// max padding is A-1
    const auto rpad = this->offset<T> (sz );// max padding is A-1
    FMR_ARRAY_PTR v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
    if (lpad == 0) {// bulk.assign(..) if already aligned when allocated
      const FMR_ARRAY_PTR from
        = reinterpret_cast<const fmr::Bulk_int*> (init_vals);
      this->bulk.assign (from, from + sz);
      if (rpad > 0) {
        for (size_t i=0; i<rpad; i++) {
          this->bulk.push_back (fmr::Bulk_int (0));
    } } } else {    // zero-initialize then copy elements
      this->bulk.resize (lpad + sz + rpad);// <= capacity(); v, ptr still valid
      const auto nz = nvals * zsz;
      FMR_PRAGMA_OMP_SIMD
      for (std::size_t i=0; i<nz; i++) { v [i] = init_vals [i]; }
    }
    return v;
  }
  template <fmr::Align_int A> template <typename T> inline
  T* Bulk<A>::get_fast (std::size_t start)
  noexcept {
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
    return & reinterpret_cast<T*>
      (& this->bulk.data ()[lpad]) [start * this->zplx.hc_size ()];
  }
  template <fmr::Align_int A> template <typename T> inline
  T* Bulk<A>::get_safe (std::size_t start)
  noexcept {
    const auto z0 = start * this->zplx.hc_size ();
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
#if 0
    if ((z0 + 1) * sizeof (T) <= lpad + this->bulk.size ()) {
      return & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [z0];
    }
    return nullptr;
#else
    return ((z0 + 1) * sizeof (T) > lpad + this->bulk.size ())
      ? nullptr : & reinterpret_cast<T*> (& this->bulk.data ()[lpad]) [z0];
#endif
  }
  template <fmr::Align_int A> template <typename T> inline
  T Bulk<A>::make_hash (T in,
    typename std::enable_if <sizeof(T) == 4>::type*)// CRC32
  noexcept {
    std::uint64_t crc = ~in;
    auto sz = this->cval_n * this->cval_szof * this->zplx.hc_size ();
    if (sz <= 0) { return 0; }
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
#if 1//def FMR_HAS_64BITCRC32
    FMR_CONST_PTR buf = reinterpret_cast<std::uint64_t*> (& this->bulk[lpad]);
    sz /= 8;
    while (sz--) {
      crc = _mm_crc32_u64 (crc, *buf++);
    }
#endif
#if 0 //def FMR_HAS_32BITCRC32
    FMR_CONST_PTR buf = reinterpret_cast<std::uint32_t*> (& this->bulk[lpad]);
    sz /= 4;
    while (sz--) {
      crc = _mm_crc32_u32 (crc, *buf++);
    }
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
    return T (~crc);
  }
  template <fmr::Align_int A> template <typename T> inline
  T Bulk<A>::make_hash (T crc,
    typename std::enable_if <sizeof(T) == 8>::type*)// CRC64
  noexcept {// https://stackoverflow.com/questions/27939882/fast-crc-algorithm
    crc = ~crc;
    std::size_t sz = this->cval_n * this->cval_szof * this->zplx.hc_size ();
    if (sz <= 0) { return 0; }
    const auto lpad = this->offset<T> (std::uintptr_t (this->bulk.data ()));
    FMR_CONST_PTR buf = reinterpret_cast<const unsigned char*>
      (& this->bulk [lpad]);
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
