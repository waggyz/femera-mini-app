#ifndef FMR_HAS_FORM_HPP
#define FMR_HAS_FORM_HPP

#include "fmr.hpp"

#include <string>

namespace fmr { namespace form {

  template <typename V>
  std::string si_unit (const V value, std::string unit,
    const int min_digits = 2, const bool is_signed = false);
  template <typename V>
  std::string si_time (const V seconds,
    const int min_digits = 2, const bool is_signed = false);

  std::string utc_time ();

} }// end fmr::form:: namespace

namespace femera { namespace form {

  template<typename ...Args>
  std::string text_line
  (fmr::Line_size_int line_width, const std::string& format, Args...);

  template<typename ...Args>
  std::string text_line (FILE*,
    fmr::Line_size_int line_width, const std::string& format, Args...);

  template<typename ...Args>
  std::string name_line
  (fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& nameing, const std::string& format, Args...);

  template<typename ...Args>
  std::string name_line (FILE*,
    fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& nameing, const std::string& format, Args...);

  template<typename ...Args>
  std::string name_time
  (fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& nameing, const std::string& format, Args...);

  template<typename ...Args>
  std::string name_time (FILE*,
    fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& nameing, const std::string& format, Args...);

} }//end femera::form:: namespace

#include "form.ipp"

//end FMR_HAS_FORM_HPP
#endif
