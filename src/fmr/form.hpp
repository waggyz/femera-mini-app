#ifndef FMR_HAS_FORM_HPP
#define FMR_HAS_FORM_HPP

#include "fmr.hpp"

#include <string>

namespace fmr { namespace form {

template <typename V>
std::string si_unit_string (const V value, std::string unit,
  const int min_digits = 2, const bool is_signed = false);

} }// end fmr::form:: namespace

namespace femera { namespace form {

  template<typename ...Args>
  std::string head_line
  (fmr::Line_size_int head_width, fmr::Line_size_int line_width,
    const std::string& head, const std::string& form, Args...);

  template<typename ...Args>
  std::string head_line (FILE*,
    fmr::Line_size_int head_width, fmr::Line_size_int line_width,
    const std::string& head, const std::string& form, Args...);

} }//end femera::form:: namespace

#include "form.ipp"

//end FMR_HAS_FORM_HPP
#endif
