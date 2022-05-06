#ifndef FEMERA_DATA_BANK_IPP
#define FEMERA_DATA_BANK_IPP

namespace femera { namespace data {
  inline
  Bank::Bank () {
    this->name_vals.reserve (this->map_init_size);
    this->name_ints.reserve (this->map_init_size);
  }
  inline
  Bank::Bank (const fmr::Local_int n) : map_init_size (n) {
    this->name_vals.reserve (this->map_init_size);
    this->name_ints.reserve (this->map_init_size);
  }
  inline
  fmr::Local_int Bank::del_all ()
  noexcept {
    const fmr::Local_int n = fmr::Local_int
      (this->name_vals.size () + this->name_ints.size ());
    this->name_vals.clear ();// may release reserve; standard does not specify
    this->name_vals.reserve (this->map_init_size);
    this->name_ints.clear ();
    this->name_ints.reserve (this->map_init_size);
    return n;
  }
  template <typename T> inline
  T* Bank::set (const Data_id& id, const std::size_t n, const T init_val)
  noexcept {
    return std::is_floating_point <T>::value
      ? this->name_vals[id].set (n, init_val)
      : this->name_ints[id].set (n, init_val);
  }
  template <typename T>
  T* Bank::set (const Data_id& id, const std::size_t n, const T* init_vals)
  noexcept {
    return std::is_floating_point <T>::value
      ? this->name_vals[id].set (n, init_vals)
      : this->name_ints[id].set (n, init_vals);
  }
  template <typename T> inline
  void Bank::del (const Data_id& id)
  noexcept {std::is_floating_point <T>::value
      ? this->name_vals.erase (id)
      : this->name_ints.erase (id);
  }
  template <typename T> inline
  T* Bank::get (const Data_id& id, const std::size_t start, typename
    std::enable_if <std::is_floating_point <T>::value>::type*)
  noexcept {
    Bulk_vals* vals = nullptr;
    try {vals =& this->name_vals.at (id);}
    catch (std::out_of_range & e) {
      printf ("%s: floating point name not found\n", id.c_str());
      return nullptr;
    }
    const auto ptr = vals->get_fast<T>();
    if (vals->get_sizeof<T>() == sizeof (T)) {
      return & ptr [start];
    }
    // convert vals in memory to requested type T
    const auto n    = vals->get_size  <T>();
    const auto each = vals->get_sizeof<T>();
#ifdef FMR_DEBUG
    printf ("%s: padded size before move %lu bytes\n",
      id.c_str(), vals->mem_size<T>());
#endif
    const auto keep = std::move (vals->take_bulk());    // keep ptr valid
#ifdef FMR_DEBUG
    printf ("%s: padded size after  move %lu bytes\n",
      id.c_str(), vals->mem_size<T>());
#endif
    FMR_ALIGN_PTR dest = this->set<T> (id, n, T(0.0));           // zero-initialize
    if (n<=0) {return dest;}
    switch (each) {
      case sizeof (float): {                            // cast from float to T
        const FMR_ALIGN_PTR v = reinterpret_cast<float*> (ptr);
        FMR_PRAGMA_OMP_SIMD
        for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
      break;}
      case sizeof (double): {                           // cast from double to T
        const FMR_ALIGN_PTR v = reinterpret_cast<double*> (ptr);
        FMR_PRAGMA_OMP_SIMD
        for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
      break;}
    }
    return & dest [start];
  }
  template <typename T> inline
  T* Bank::get (const Data_id& id, const std::size_t start, typename
    std::enable_if <std::is_integral <T>::value>::type*)
  noexcept {
    Bulk_ints* vals = nullptr;
    try {vals =& this->name_ints.at(id);}
    catch (std::out_of_range & e) {
      printf ("%s: integer name not found\n", id.c_str());
      return nullptr;
    }
    const auto ptr = vals->get_fast<T>();
    if (vals->get_sizeof <T>() == sizeof (T)
      && vals->has_sign <T>() == std::is_signed <T>::value) {
      return & ptr [start];
    }
    // convert stored to requested type
#ifdef FMR_DEBUG
    printf ("converting %s from %sint%lu_t to %sint%lu_t...\n", id.c_str(),
      vals->has_sign () ? "":"u", vals->get_sizeof (),
      std::is_signed <T>::value ? "":"u", sizeof(T) )
#endif
    const auto n    = vals->get_size  <T>();
    const auto each = vals->get_sizeof<T>();
    const auto sign = vals->has_sign  <T>();
    const auto keep = std::move (vals->take_bulk());        // keep ptr valid
    FMR_ARRAY_PTR dest = this->set<T> (id, n, T(0));        // zero-initialize
    if (n<=0) {return dest;}
    if (sign) {                                             // signed ints
      switch (each) {
        case sizeof (int8_t): {
          const FMR_ARRAY_PTR v = reinterpret_cast<int8_t*> (ptr);
          FMR_PRAGMA_OMP_SIMD
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (int16_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<int16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (int32_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<int32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (int64_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<int64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
      } } else {                                            // unsigned ints
      switch (each) {
        case sizeof (uint8_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<uint8_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (uint16_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<uint16_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (uint32_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<uint32_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
        case sizeof (uint64_t): {
          FMR_PRAGMA_OMP_SIMD
          const FMR_ARRAY_PTR v = reinterpret_cast<uint64_t*> (ptr);
          for (std::size_t i=0; i<n; i++) {dest [i] = T (v [i]);}
        break;}
    } }
    return & dest [start];
  }
} }//end femera::data:: namespace

//end FEMERA_DATA_BANK_IPP
#endif
