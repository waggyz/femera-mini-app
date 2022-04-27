#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  inline constexpr
  uintptr_t Bulk::offset (uintptr_t address, uintptr_t align){
    return ((address % align) == 0 ) ? 0 : (align - (address % align));
  }
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    if (n <= 0) {
      this->name_ints[id].bulk ={};// inserts new empty vec or clears existing
      const auto v = & this->name_ints [id];
      v->size      = 0;
      v->size_of   = sizeof (T);
      v->has_sign  = std::is_signed<T>::value;
      return reinterpret_cast<T*> (v->bulk.data());
    }
#ifdef FMR_ALIGN_INTS
    // over-allocate for aligned access
    const auto sz = (n * sizeof(T) + 2 * (FMR_ALIGN_INTS - 1))
      / sizeof (fmr::Bulk_int);
    //NOTE sz is also large enough for full aligned access at the tail
#else
    const auto sz = n * sizeof(T) / sizeof(fmr::Bulk_int);
#endif
    this->name_ints[id].bulk.reserve (sz);// uninitialized, bulk.size()==0
    const auto v = & this->name_ints [id];
    v->size      = n;
    v->size_of   = sizeof (T);
    v->has_sign  = std::is_signed<T>::value;
    auto ptr     = v->bulk.data ();
#if 0
    this->name_ints[id].bulk [0] = fmr::Bulk_int(0);// undefined behavior
    // Accessesing a vector past size() is undefined behavior...
#else
    // ...but accessing the reserved space in the underlying array should be OK.
    ptr [0] = fmr::Bulk_int(0);  // first touch
#endif
    return reinterpret_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
    if (n <= 0) {
      this->name_vals[id].bulk ={};// inserts new empty vec or clears existing
      const auto v = & this->name_vals [id];
      v->size      = 0;
      v->size_of   = sizeof (T);
      v->has_sign  = std::is_signed<T>::value;
      return reinterpret_cast<T*> (v->bulk.data());
    }
#ifdef FMR_ALIGN_VALS
    // over-allocate for aligned access
    const auto sz = (n * sizeof(T) + 2 * (FMR_ALIGN_VALS - 1))
      / sizeof (fmr::Bulk_int);
#else
    const auto sz = n * sizeof(T) / sizeof(fmr::Bulk_int);
#endif
    this->name_vals[id].bulk.reserve (sz);// uninitialized, size()==0
    const auto v = & this->name_vals [id];
    v->size      = n;
    v->size_of   = sizeof (T);
    v->has_sign  = std::is_signed<T>::value;
    auto ptr     = v->bulk.data ();
    ptr [0]      = fmr::Bulk_int(0);  // first touch
    return reinterpret_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, const T init_val)
  noexcept {
    this->add<T> (id, n);
    auto vec = (std::is_floating_point <T>::value)
      ? & name_vals[id].bulk : & name_ints[id].bulk;
    if (n > 0) {
      const auto ptr = uintptr_t (vec->data ());
      const uintptr_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
#ifdef FMR_ALIGN_VALS
      const uintptr_t a = (std::is_floating_point <T>::value
        ? FMR_ALIGN_VALS : FMR_ALIGN_INTS) / sizeof (fmr::Bulk_int);
      const auto rpad = Bulk::offset (sz,  a);// max padding is a-1
      const auto lpad = Bulk::offset (ptr, a);// max padding is a-1
#else
      const uintptr_t lpad=0, rpad=0;
#endif
      vec->resize (sz + lpad + rpad);// <= capacity (); includes padding
      //NOTE Zero is the same bits for all numeric types (data/Vals.gtst.cpp).
      if (init_val < T(0) || init_val > T(0)) {// != 0.0, no float warning
#if 0
        // Initialize from a temporary vector of type T.
        // NOTE This will not work with padding.
        const auto tmp = std::vector<T> (n, init_val);
        const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
        vec->assign (ptr, ptr + sz);
#else
        // Initialize without making a tmp vector. NOTE 10x slower than below.
#if 0
        if (lpad > 0) {
          for (size_t i=0; i<lpad; i++) {vec->push_back (fmr::Bulk_int(0));}
        }
        const auto bytes = reinterpret_cast <const fmr::Bulk_int*> (& init_val);
        const auto mod   = fmr::Local_int (sizeof (T) / sizeof (fmr::Bulk_int));
        for (size_t i=0; i<sz; i++ ) { vec->push_back (bytes [i % mod]); }
#endif
        auto v = reinterpret_cast<T*> (& vec->data ()[lpad]);
        for (size_t i=0; i<n; i++ ) {v [i] = init_val;}// 10% slower than 0init
        return v;
#endif
      }
      return reinterpret_cast<T*> (& vec->data ()[lpad]);
    }
    return reinterpret_cast<T*> (vec->data ());
  }
#if 1
  template <typename T>
  T* Bulk::add (const Data_id& id, const size_t n, const T* init_vals)
  noexcept {
    this->add<T> (id, n);
    auto vec = (std::is_floating_point <T>::value)
      ? & name_vals[id].bulk : & name_ints[id].bulk;
    if (n > 0) {
      const auto ptr = uintptr_t (vec->data ());
      const uintptr_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
#ifdef FMR_ALIGN_VALS
      const uintptr_t a = (std::is_floating_point <T>::value
        ? FMR_ALIGN_VALS : FMR_ALIGN_INTS) / sizeof (fmr::Bulk_int);
      const auto lpad = Bulk::offset (ptr, a);// max padding is a-1
      const auto rpad = Bulk::offset (sz,  a);// max padding is a-1
#else
      const uintptr_t a=0, lpad=0;
#endif
#if 0
      if (lpad > 0) {
        for (size_t i=0; i<lpad; i++) {vec->push_back (fmr::Bulk_int(0));}
      }
      const auto init = reinterpret_cast<const fmr::Bulk_int*> (init_vals);
      //vec->assign (init, init + n);
      for (size_t i=0; i<sz; i++ ) { vec->push_back (init [i]); }
      return reinterpret_cast<T*> (& vec->data ()[lpad]);
#endif
      vec->resize (sz + lpad + rpad);// <= capacity (); includes front padding
      auto v = reinterpret_cast<T*> (& vec->data ()[lpad]);
      for (size_t i=0; i<n; i++ ) {v [i] = init_vals [i];}
      return v;
    }
    return reinterpret_cast<T*> (vec->data ());
  }
#endif
  template <typename T> inline
  T* Bulk::get (const Data_id& id, size_t start, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    Bulk_align<FMR_ALIGN_INTS>* ba = nullptr;
    try {ba =& this->name_ints.at(id);}
    catch (std::out_of_range& e) {
      printf ("%s: integer name not found\n", id.c_str());
      return nullptr;
    }
    auto ptr = uintptr_t (ba->bulk.data());
#ifdef FMR_ALIGN_INTS
    ptr += Bulk::offset (ptr, FMR_ALIGN_INTS);
#endif
    if (ba->size_of == sizeof(T) && ba->has_sign == std::is_signed<T>::value) {
      return & reinterpret_cast<T*> (ptr) [start];
    }
#ifdef FMR_DEBUG
    printf ("converting %s from %sint%lu_t to %sint%lu_t...\n", id.c_str(),
      ba->has_sign ? "":"u", ba->size_of,
      std::is_signed<T>::value ? "":"u", sizeof(T) );
#endif
    // convert stored to requested type
    const auto n    = ba->size;
    const auto bits = ba->size_of * 8;
    const auto sign = ba->has_sign;
    const auto src  = std::move (ba->bulk);// stash vector here; ptr still valid
    auto dest = this->add<T> (id, n, T(0));                 // zero-initialized
    if (sign) {                                             // signed ints
      switch (bits) {
        case  8: {
          const auto v = reinterpret_cast<int8_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 16: {
          const auto v = reinterpret_cast<int16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 32: {
          const auto v = reinterpret_cast<int32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 64: {
          const auto v = reinterpret_cast<int64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
      }
    } else {                                                // unsigned ints
      switch (bits) {
        case  8: {
          const auto v = reinterpret_cast<uint8_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 16: {
          const auto v = reinterpret_cast<uint16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 32: {
          const auto v = reinterpret_cast<uint32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
        case 64: {
          const auto v = reinterpret_cast<uint64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
        break;}
      }
    }
    return & reinterpret_cast<T*> (dest) [start];
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, size_t start, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
    Bulk_align<FMR_ALIGN_VALS>* ba = nullptr;
    try {ba =& this->name_vals.at(id);}
    catch (std::out_of_range& e) {
      printf ("%s: floating point name not found\n", id.c_str());
      return nullptr;
    }
    auto ptr = uintptr_t (ba->bulk.data());
#ifdef FMR_ALIGN_VALS
    ptr += Bulk::offset (ptr, FMR_ALIGN_VALS);
#endif
    if (ba->size_of == sizeof(T)) {
      return & reinterpret_cast<T*> (ptr) [start];
    }
    // convert stored to requested type
    const auto n    = ba->size;
    const auto bits = ba->size_of * 8;
    const auto src  = std::move (ba->bulk);// stash vector here; ptr still valid
    auto dest = this->add<T> (id, n, T(0.0));               // zero-initialized
    switch (bits) {
      case 32: {                                            // float
        const auto v = reinterpret_cast<float*> (ptr);
        for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
      break;}
      case 64: {                                            // double
        const auto v = reinterpret_cast<double*> (ptr);
        for (std::size_t i=0; i<n; i++) {dest [i] = T(v[i]);}
      break;}
      //TODO add SSE, AVX, AVX512 cases
    }
    return & reinterpret_cast<T*> (dest) [start];
  }
  template <typename T> inline
  void Bulk::del (const Data_id& id)
  noexcept {
    this->name_vals.erase (id);
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
