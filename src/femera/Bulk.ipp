#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    this->size    = n;
    this->size_of = sizeof (T);
    if (n <= 0) {
      this->name_ints[id].bulk ={};// inserts new empty vec or clears existing
      return reinterpret_cast<T*> (this->name_ints[id].bulk.data());
    } else {
      //TODO over-allocate & find first aligned byte?
      const auto sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
      this->name_ints[id].bulk.reserve (sz);// uninitialized, size()==0
      auto ptr = name_ints[id].bulk.data ();
#if 0
      this->name_ints[id].bulk [0] = fmr::Bulk_int(0);// undefined behavior
      // Accessesing a vector past size() is undefined behavior...
#else
      // ...but accessing the underlying array should be OK.
      ptr[0] = fmr::Bulk_int(0);  // first touch
#endif
      return reinterpret_cast<T*> (ptr);
  } }
  template <typename T> inline
  T* Bulk::add (const Data_id& id, const size_t n, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
    this->size    = n;
    this->size_of = sizeof (T);
    if (n <= 0) {
      this->name_vals[id].bulk ={};// inserts new empty vec or clears existing
      return reinterpret_cast<T*> (this->name_vals[id].bulk.data());
    }
    //TODO over-allocate & find first aligned byte?
    const auto sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
    this->name_vals[id].bulk.reserve (sz);// uninitialized, size()==0
    auto ptr = name_vals[id].bulk.data ();
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
      const auto a = (std::is_floating_point <T>::value)
        ? sizeof (FMR_ALIGN_VALS) : sizeof (FMR_ALIGN_INTS);
      size_t sz = n * sizeof (T) / sizeof (fmr::Bulk_int);
      sz += ((sz % a) == 0 ) ? 0 : a - (sz % a);// Pad end to alignment size.
      if (init_val <= T(0) && init_val >= T(0)) {// ==0, no float warning
        //NOTE              Zero is the same bits for all numeric types.
        vec->resize (sz);// This is tested in data/Vals.gtst.cpp.
      } else {
#if 0
        // Initialize from a temporary vector of type T.
        const auto tmp = std::vector<T> (n, init_val);
        const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
        vec->assign (ptr, ptr + sz);
#else
        // Initialize without making a temporary vector.
        const auto bytes = reinterpret_cast<const fmr::Bulk_int*> (&init_val);
        const auto mod   = fmr::Local_int (sizeof (T) / sizeof (fmr::Bulk_int));
        for (size_t i=0; i<sz; i++ ) { vec->push_back (bytes [i % mod]); }
#endif
    } }
    return reinterpret_cast<T*> (vec->data ());
  }
  template <typename T> inline
  T* Bulk::get (const Data_id& id, size_t start)
  noexcept {
    if (sizeof (T) == this->size_of) {//TODO Check if signed?
      return (std::is_floating_point <T>::value
        ? reinterpret_cast<T*> (& this->name_vals.at(id).bulk.data()
          [start * sizeof (T) / sizeof (fmr::Bulk_int)])
        : reinterpret_cast<T*> (& this->name_ints.at(id).bulk.data()
          [start * sizeof (T) / sizeof (fmr::Bulk_int)]));
      //TODO catch std::out_of_range
    }
    return nullptr;//TODO convert to requested type T
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
