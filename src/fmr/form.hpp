#ifndef FMR_HAS_FORM_HPP
#define FMR_HAS_FORM_HPP

//#include "detail.hpp"
#include <string>

namespace fmr { namespace form {

template<typename V>
std::string si_unit_string (const V val, std::string unit, const int mindigits=2);

} }// end fmr::form:: namespace

#include "form.ipp"

#undef FMR_DEBUG
//end FMR_HAS_FORM_HPP
#endif
