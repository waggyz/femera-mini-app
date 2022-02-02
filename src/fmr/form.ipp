#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

#include <cmath>    // std::abs()
#include <cstdio>     // std::snprintf
#include <vector>

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



namespace femera {

  template<typename ...Args>
  std::string form::head_line
  (const fmr::Line_size_int head_width, const fmr::Line_size_int line_width,
    const std::string head, const std::string form, Args ...args)
    {
    std::vector<char> buf (line_width + 1, 0);
    const auto format = "%" + std::to_string(head_width) +"s "+ form;
    std::snprintf (&buf[0], line_width, format.c_str(), head.c_str(), args...);
    return std::string(&buf[0]);
  }

}//end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif
