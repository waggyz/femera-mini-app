#ifndef FMR_HAS_FORM_HPP
#define FMR_HAS_FORM_HPP

#include "fmr.hpp"

// below used in form.ipp
#include <cctype>
#include <cmath>      // std::abs()
#include <cstdio>     // std::snprintf
#include <valarray>  // needed by form.ipp

namespace fmr { namespace form {

  template <typename V> static
  std::string si_unit (const V value, std::string unit,
    const int min_digits = 2, const bool is_signed = false);
  template <typename V> static
  std::string si_time (const V seconds,
    const int min_digits = 2, const bool is_signed = false);
  //
  std::string utc_time ();
  //
  static bool ends_with (const std::string& str, const std::string& end);
  //
  bool are_both_spaces (char lhs, char rhs);
  std::string delete_extra_spaces ( std::string );
} }// end fmr::form:: namespace

namespace fmr {
  using Line_size_int = uint_fast16_t;
}// end fmr:: namespace

namespace femera { namespace form {

  std::string si_unit (const double val, std::string unit,
    const int min_digits, const std::string signchar);// defined in form.cpp

  template <typename ...Args> static
  std::string text_line
  (fmr::Line_size_int line_width, const std::string& format, Args...);

  template <typename V, typename ...Args> static
  std::string perf_line (V count, V secs, const std::string& unit="B");

  template <typename ...Args> static
  std::string text_line (FILE*,
    fmr::Line_size_int line_width, const std::string& format, Args...);

  template <typename ...Args> static
  std::string name_line
  (fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& name, const std::string& format, Args...);

  template <typename ...Args> static
  std::string name_line (FILE*,
    fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& name, const std::string& format, Args...);

  template <typename ...Args> static
  std::string time_line
  (fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& name, const std::string& format, Args...);

  template <typename ...Args> static
  std::string time_line (FILE*,
    fmr::Line_size_int name_width, fmr::Line_size_int line_width,
    const std::string& name, const std::string& format, Args...);

#if 0
    ss wrap_line (..);
    ss data_line (..);
    ss perf_line (..);
    ss perf_line (..);
#endif
} }//end femera::form:: namespace

#include "form.ipp"

//end FMR_HAS_FORM_HPP
#endif
