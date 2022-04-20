#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <typename I> inline
  I* Bulk::add (const Data_id& id, const size_t n,
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    if (n == 0) {
      this->name_ints[id].bulk ={};// inserts new or clears existing empty vec
      return reinterpret_cast<I*> (this->name_ints[id].bulk.data());
    } else {
      //TODO over-allocate & find first aligned byte?
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      this->name_ints[id].bulk.reserve (sz);// uninitialized, size()==0
#if 0
      this->name_ints[id].bulk [sz - 1] = fmr::Bulk_int(0);// undefined behavior
      //printf ("uninit size: %lu\n", name_ints[id].bulk.size());
      // Accessesing past size() is undefined behavior...
#else
      // ...but accessing the underlying array should be OK.
      auto       bulk_vec = & name_ints[id].bulk;
      auto ptr = bulk_vec->data ();
      ptr [0] = fmr::Bulk_int (0);     // first-touch allocate, then set size
      bulk_vec->assign (ptr, ptr + sz);// Self-assign can be optimized away.
#endif
      auto vals = reinterpret_cast<I*> (bulk_vec->data());
      return vals;
  } }
  template <typename I> inline
  I* Bulk::add (const Data_id& id, const size_t n, const I& init_val,
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    auto vals = reinterpret_cast<I*> (this->add<I> (id, n));
    if (n > 0) {
      for (size_t i=0; i<n; i++) { vals [i] = init_val; }
#if 0
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      const auto bulk_ptr = reinterpret_cast<fmr::Bulk_int*> (vals);
      auto       bulk_vec = & name_ints[id].bulk;
      for (size_t i=0; i<sz; i++) { bulk_vec->push_back (bulk_ptr[i]); }
#endif
    }
    return vals;
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
