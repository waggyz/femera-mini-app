#ifndef FMR_HAS_FMR_IPP
#define FMR_HAS_FMR_IPP

template <typename E> constexpr typename
std::underlying_type<E>::type fmr::enum2val (E e) {
 /* Cast enum to underlying type to get the last enum index number for
  * size, sync., etc., use, e.g.,
  * const auto data_type_n = fmr::enum2val (fmr::Data_type::end);
  */
  return static_cast <typename std::underlying_type<E>::type> (e);
}

//end FMR_HAS_FMR_IPP
#endif