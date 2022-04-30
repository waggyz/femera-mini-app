#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  //===========================================================================
  template <fmr::Align_int N> inline constexpr
  fmr::Align_int Bulk_vals<N>::offset (const std::uintptr_t address)
  noexcept {
    return ((address % N) == 0 ) ? 0 : (N - (address % N));// range: [0, N-1]
  }
  template <fmr::Align_int N>
  template <typename T> inline
  std::size_t Bulk_vals<N>::get_size ()
  noexcept {
    return this->size;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  fmr::Align_int Bulk_vals<N>::get_sizeof ()
  noexcept {
    return this->size_of;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  bool Bulk_vals<N>::has_sign ()
  noexcept {
    return this->is_signed;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  Bulk_vec_t& Bulk_vals<N>::take_bulk ()
  noexcept {
    this->size = 0;
    return this->bulk;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk_vals<N>::raw (const std::size_t n)
  noexcept {
    if (n <= 0) {
      this->bulk      ={};// new empty vec or clear existing
      this->size      = 0;
      this->size_of   = sizeof (T);
      this->is_signed = std::is_signed<T>::value;
      return reinterpret_cast<T*> (this->bulk.data());
    }
    // over-allocate for aligned access
    const auto sz = (n * sizeof (T) + 2 * (N - 1)) / sizeof (fmr::Bulk_int);
    //NOTE sz is large enough for aligned SIMD access at the tail
    this->bulk.reserve (sz);// uninitialized, bulk.size()==0
    this->size      = n;
    this->size_of   = sizeof (T);
    this->is_signed = std::is_signed<T>::value;
    auto ptr        = this->bulk.data ();
#if 0
    this->bulk [0] = fmr::Bulk_int(0);// undefined behavior
    // Accessesing a vector past size() is undefined behavior...
#else
    // ...but accessing the reserved space in the underlying array should be OK.
    ptr [0] = fmr::Bulk_int(0);  // first touch
#endif
    return reinterpret_cast<T*> (ptr);
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk_vals<N>::set (const std::size_t n, const T init_val)
  noexcept {
    this->raw<T> (n);
    if (n <= 0) {return reinterpret_cast<T*> (this->bulk.data ());}
    const auto ptr = std::uintptr_t (this->bulk.data ());
    const std::uintptr_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
    const auto lpad = this->offset (ptr);// max left  padding is N-1
    const auto rpad = this->offset (sz );// max right padding is N-1
    //NOTE Zero is the same bits for all numeric types (data/Vals.gtst.cpp).
    if (init_val <= T(0) && init_val >= T(0)) {// == 0.0, no float warning
      this->bulk.resize (sz + lpad + rpad);// <= capacity (); ptr still valid
    } else {                                              // nonzero
#if 0
      // Initialize from a temporary vector of type T.
      // NOTE This will not work with padding.
      const auto tmp = std::vector<T> (n, init_val);
      const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
      this->bulk.assign (ptr, ptr + sz);
#else
      // Initialize without making a tmp vector. NOTE 10x slower than 0init.
#if 0
      if (lpad > 0) {
        for (size_t i=0; i<lpad; i++) {this->bulk.push_back (fmr::Bulk_int(0));}
      }
      const auto bytes = reinterpret_cast <const fmr::Bulk_int*> (& init_val);
      const auto mod   = fmr::Local_int (sizeof (T) / sizeof (fmr::Bulk_int));
      for (size_t i=0; i<sz; i++ ) { this->bulk.push_back (bytes [i % mod]); }
#endif
      auto v = reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
      this->bulk.resize (sz + lpad + rpad);// <= capacity (); ptr still valid
      for (size_t i=0; i<n; i++ ) {v [i] = init_val;}// 35% slower than 0init
      return v;
#endif
    }
    return reinterpret_cast<T*> (& this->bulk.data ()[lpad]);
  }
  template <fmr::Align_int N>
  template <typename T>
  T* Bulk_vals<N>::set (const std::size_t n, const T* init_vals)
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
      if (rpad>0) {
        for (size_t i=0; i<rpad; i++) {this->bulk.push_back(fmr::Bulk_int(0));
    } } } else {    // zero-initialize then copy elements
      this->bulk.resize (sz + lpad + rpad);// <= capacity(); v,ptr still valid
      for (std::size_t i=0; i<n; i++ ) {v [i] = init_vals [i];}
    }
    return v;
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk_vals<N>::get_fast (std::size_t start)
  noexcept {
    const auto lpad = this->offset (std::uintptr_t (this->bulk.data ()));
    return & reinterpret_cast<T*> (& this->bulk.data ()[lpad])[start];
  }
  template <fmr::Align_int N>
  template <typename T> inline
  T* Bulk_vals<N>::get_safe (std::size_t start)
  noexcept {
    const auto lpad = this->offset (std::uintptr_t (this->bulk.data ()));
    if ((start + 1) * sizeof (T) <= lpad + this->bulk.size()) {
      return & reinterpret_cast<T*> (& this->bulk.data ()[lpad])[start];
    }
    return nullptr;
  }
  //===========================================================================
  inline
  Bulk::Bulk () {
    this->name_ints.reserve (1024);
    this->name_vals.reserve (1024);
  }
#if 0
  inline constexpr
  fmr::Align_int Bulk::offset
  (const std::uintptr_t adr, const fmr::Align_int align)
  noexcept {
    return ((adr % align) == 0 ) ? 0 : (align - (adr % align));// [0, align-1]
  }
#endif
  template <typename T> inline
  T* Bulk::set (const Data_id& id, const std::size_t n, const T init_val)
  noexcept {
    return std::is_floating_point <T>::value
      ? this->name_vals[id].set (n, init_val)
      : this->name_ints[id].set (n, init_val);
  }
  template <typename T>
  T* Bulk::set (const Data_id& id, const std::size_t n, const T* init_vals)
  noexcept {
    return std::is_floating_point <T>::value
      ? this->name_vals[id].set (n, init_vals)
      : this->name_ints[id].set (n, init_vals);
  }
  template <typename T> inline
  void Bulk::del (const Data_id& id)
  noexcept {std::is_floating_point <T>::value
      ? this->name_vals.erase (id)
      : this->name_ints.erase (id);
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, std::size_t start, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
#ifdef FMR_ALIGN_INTS
    Bulk_vals<FMR_ALIGN_INTS>* vals = nullptr;
#else
    Bulk_vals<alignof(std::size_t)>* vals = nullptr;
#endif
    try {vals =& this->name_ints.at(id);}
    catch (std::out_of_range& e) {
      printf ("%s: integer name not found\n", id.c_str());
      return nullptr;
    }
    const auto ptr = vals->get_fast<T> ();
    if (vals->get_sizeof<T> () == sizeof(T)
      && vals->has_sign<T> () == std::is_signed<T>::value) {
      return & ptr [start];
    }
    // convert stored to requested type
#ifdef FMR_DEBUG
    printf ("converting %s from %sint%lu_t to %sint%lu_t...\n", id.c_str(),
      vals->has_sign () ? "":"u", vals->get_sizeof (),
      std::is_signed<T>::value ? "":"u", sizeof(T) )
#endif
    const auto n    = vals->get_size  <T> ();
    const auto bits = vals->get_sizeof<T> () * 8;
    const auto sign = vals->has_sign  <T> ();
    const auto keep = std::move (vals->take_bulk<T>());// keeps ptr valid
    auto dest = this->set<T> (id, n, T(0));                 // zero-initialize                                     // signed ints
    if (n<=0) {return dest;}
    if (sign) {                                             // signed ints
      switch (bits) {
        case  8: {
          const auto v = reinterpret_cast<int8_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 16: {
          const auto v = reinterpret_cast<int16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 32: {
          const auto v = reinterpret_cast<int32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 64: {
          const auto v = reinterpret_cast<int64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
      } } else {                                            // unsigned ints
      switch (bits) {
        case  8: {
          const auto v = reinterpret_cast<uint8_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 16: {
          const auto v = reinterpret_cast<uint16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 32: {
          const auto v = reinterpret_cast<uint32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
        case 64: {
          const auto v = reinterpret_cast<uint64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
        break;}
    } }
    return & dest [start];
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, std::size_t start, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
#ifdef FMR_ALIGN_VALS
    Bulk_vals<FMR_ALIGN_VALS>* vals = nullptr;
#else
    Bulk_vals<alignof(double)>* vals = nullptr;
#endif
    try {vals =& this->name_vals.at(id);}
    catch (std::out_of_range& e) {
      printf ("%s: floating point name not found\n", id.c_str());
      return nullptr;
    }
    const auto ptr = vals->get_fast<T> ();
    if (vals->get_sizeof<T> () == sizeof(T)) {
      return & ptr [start];
    }
    // convert vals in memory to requested type T
    const auto n    = vals->get_size  <T> ();
    const auto bits = vals->get_sizeof<T> () * 8;
    const auto keep = std::move (vals->take_bulk<T>()); // keeps ptr valid
    auto dest = this->set<T> (id, n, T(0.0));           // zero-initialize                                      // signed ints
    if (n<=0) {return dest;}
    switch (bits) {
      case 32: {                                        // cast from float to T
        const auto v = reinterpret_cast<float*> (ptr);
        for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
      break;}
      case 64: {                                        // cast from double to T
        const auto v = reinterpret_cast<double*> (ptr);
        for (std::size_t i=0; i<n; i++) {dest [i] = T(v [i]);}
      break;}
    }
    return & dest [start];
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
