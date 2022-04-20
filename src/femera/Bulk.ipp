#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <typename I> inline
  I* Bulk::add (const Data_id& id, const size_t n,
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    if (n <= 0) {
      this->name_ints[id].bulk ={};// inserts new empty vec or clears existing
      return reinterpret_cast<I*> (this->name_ints[id].bulk.data());
    } else {
      //TODO over-allocate & find first aligned byte?
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      this->name_ints[id].bulk.reserve (sz);// uninitialized, size()==0
      auto ptr = name_ints[id].bulk.data ();
#if 0
      this->name_ints[id].bulk [sz - 1] = fmr::Bulk_int(0);// undefined behavior
      //printf ("uninit size: %lu\n", name_ints[id].bulk.size());
      // Accessesing past size() is undefined behavior...
#else
      // ...but accessing the underlying array should be OK.
      ptr[0] = fmr::Bulk_int(0);  // first touch
#endif
# if 0
      auto tmp = std::vector<fmr::Bulk_int>();
      tmp.reserve (sz);
      auto       bulk_vec = & name_ints[id].bulk;
      bulk_vec->data ()[0] = fmr::Bulk_int (0);// first-touch allocate
      auto ptr = tmp.data ();
      ptr[0] = fmr::Bulk_int (0);// first-touch allocate
      bulk_vec->assign (ptr, ptr + sz);
#endif
      return reinterpret_cast<I*> (ptr);
  } }
  template <typename I> inline
  I* Bulk::add (const Data_id& id, const size_t n, const I& init_val,
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    this->add<I> (id, n);
    auto vec = & name_ints[id].bulk;
    if (n>0) {
      const auto tmp = std::vector<I>(n, init_val);
      const auto ptr = reinterpret_cast<const fmr::Bulk_int*> (tmp.data());
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      vec->assign (ptr, ptr + sz);
    }
    return reinterpret_cast<I*> (vec->data ());
#if 0
    auto vals = reinterpret_cast<I*> (this->add<I> (id, n));
    if (n > 0) {
      for (size_t i=0; i<n; i++) { vals [i] = init_val; }
#endif
#if 0
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      const auto bulk_ptr = reinterpret_cast<fmr::Bulk_int*> (vals);
      auto       bulk_vec = & name_ints[id].bulk;
      for (size_t i=0; i<sz; i++) { bulk_vec->push_back (bulk_ptr[i]); }
    }
    return vals;
#endif
  }
# if 1
  template <typename I> inline
  I* Bulk::get (const Data_id& id,// includes char type
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    return reinterpret_cast<I*> (this->name_ints.at(id).bulk.data());
    //TODO catch std::out_of_range
  }
    template <typename V> inline
    V* Bulk::get (const Data_id& id,
      typename std::enable_if<std::is_floating_point<V>::value>::type*)
    noexcept {
      return reinterpret_cast<V*> (this->name_vals.at(id).bulk.data());
      //TODO catch std::out_of_range
    }
#   endif
} }//end femera::data:: namespace

//end FEMERA_DATA_BULK_IPP
#endif
