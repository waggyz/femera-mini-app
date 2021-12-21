#ifndef FMR_HAS_FORM_HPP
#define FMR_HAS_FORM_HPP

#include "fmr.hpp"

#include <string>

namespace fmr { namespace form {

template<typename V>
std::string si_unit_string (const V value, std::string unit,
  const int min_digits = 2, const bool is_signed = false);

} }// end fmr::form:: namespace

#include "form.ipp"

//end FMR_HAS_FORM_HPP
#endif
