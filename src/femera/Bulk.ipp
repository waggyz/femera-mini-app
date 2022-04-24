#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    if (n <= 0) {
      this->name_ints[id].bulk ={};// inserts new empty vec or clears existing
      const auto v = & this->name_ints [id];
      v->size    = 0;
      v->size_of = sizeof (T);
      return reinterpret_cast<T*> ((v->bulk.data()));
    }
    // over-allocate & find first aligned byte
    const auto sz = FMR_ALIGN_INTS - 1 + n * sizeof(T) / sizeof(fmr::Bulk_int);
    this->name_ints[id].bulk.reserve (sz);// uninitialized, bulk.size()==0
    const auto v = & this->name_ints [id];
    v->size    = n;
    v->size_of = sizeof (T);
    auto ptr   = v->bulk.data ();
#if 0
    this->name_ints[id].bulk [0] = fmr::Bulk_int(0);// undefined behavior
    // Accessesing a vector past size() is undefined behavior...
#else
    // ...but accessing the underlying array should be OK.
    ptr[0] = fmr::Bulk_int(0);  // first touch
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
      v->size    = 0;
      v->size_of = sizeof (T);
      return reinterpret_cast<T*> (v->bulk.data());
    }
    // over-allocate & find first aligned byte
    const auto sz = FMR_ALIGN_VALS - 1 + n * sizeof(T) / sizeof(fmr::Bulk_int);
    this->name_vals[id].bulk.reserve (sz);// uninitialized, size()==0
    const auto v = & this->name_vals [id];
    v->size    = n;
    v->size_of = sizeof (T);
    auto ptr   = v->bulk.data ();
    ptr[0] = fmr::Bulk_int(0);  // first touch
    return reinterpret_cast<T*> (ptr);
  }
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, const T init_val)
  noexcept {
    this->add<T> (id, n);
    auto vec = (std::is_floating_point <T>::value)
      ? & name_vals[id].bulk : & name_ints[id].bulk;
    if (n>0) {
      const uintptr_t a = (std::is_floating_point <T>::value)
        ? FMR_ALIGN_VALS : FMR_ALIGN_INTS;
      const auto ptr = uintptr_t (vec->data ());
      const auto pad = a - (ptr % a);
      uintptr_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
      sz += ((sz % a) == 0 ) ? 0 : a - (sz % a);// Pad end to alignment size.
      if (init_val <= T(0) && init_val >= T(0)) {// ==0, no float warning
        //NOTE Zero is the same bits for all numeric types (data/Vals.gtst.cpp).
        vec->resize (sz + a - 1);// Also pad the front. 
      } else {
#if 0
        // Initialize from a temporary vector of type T.
        const auto tmp = std::vector<T> (n, init_val);
        const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
        vec->assign (ptr, ptr + sz);
#else
        // Initialize without making a temporary vector.
        if (pad>0) {for (size_t i=0; i<pad; i++) {
          vec->push_back (fmr::Bulk_int(0));
        } }
        const auto bytes = reinterpret_cast <const fmr::Bulk_int*> (&init_val);
        const auto mod   = fmr::Local_int (sizeof (T) / sizeof (fmr::Bulk_int));
        for (size_t i=0; i<sz; i++ ) { vec->push_back (bytes [i % mod]); }
#endif
      }
      return reinterpret_cast<T*> (& vec->data ()[pad]);
    }
    return reinterpret_cast<T*> (vec->data ());
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, size_t start, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    //TODO Check size & sign?
    auto ptr = uintptr_t (this->name_ints.at(id).bulk.data());
    return & reinterpret_cast<T*>
    (ptr + FMR_ALIGN_INTS - (ptr % FMR_ALIGN_INTS)) [start];
    //TODO catch std::out_of_range
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, size_t start, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
    //TODO Check size & sign?
    auto ptr = uintptr_t (this->name_vals.at(id).bulk.data());
    return & reinterpret_cast<T*>
      (ptr + FMR_ALIGN_VALS - (ptr % FMR_ALIGN_VALS)) [start];
    //TODO catch std::out_of_range
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
