#ifndef FEMERA_DATA_BULK_IPP
#define FEMERA_DATA_BULK_IPP

namespace femera { namespace data {
  template <typename I> inline
  I* Bulk::add (const Data_id& id, const size_t n, const I& init_val,
    typename std::enable_if<std::is_integral<I>::value>::type*)
  noexcept {
    if (n == 0) {
      this->name_ints[id].bulk ={};// empty, inserts new or clears existing
      return reinterpret_cast<I*> (this->name_ints[id].bulk.data());
    } else {
      //TODO over-allocate & find first aligned byte?
      const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
      this->name_ints[id].bulk.reserve (sz);// uninitialized
      this->name_ints[id].bulk [sz-1] = fmr::Bulk_int(0);// first touch
      auto vals = reinterpret_cast<I*> (this->name_ints[id].bulk.data());
      if (&init_val != nullptr) {// initialize
        for (size_t i=0; i<n; i++) { vals[i]=init_val; }
      }
      return vals;
  } }
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
