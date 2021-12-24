#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

#include <cmath>    // std::abs()

namespace fmr { namespace detail { namespace form {

std::string si_unit_string (const double val, std::string unit,
  const int min_digits, const std::string signchar);

} } }// end fmr::detail::form:: namespace

namespace fmr {

template<typename V>
std::string form::si_unit_string (const V val, std::string unit,
const int md, const bool is_signed) {
  const std::string sign = (val < V(0)) ? "-" :
    (is_signed ? ((val > V(0)) ? "+" : " ") : "");
  return detail::form::si_unit_string (std::abs(double(val)), unit, md, sign);
}

}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif
