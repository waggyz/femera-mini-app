#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

namespace fmr { namespace detail { namespace form {

  std::string si_unit (const double val, std::string unit,
    const int min_digits, const std::string signchar);// defined in form.cpp

} } }// end fmr::detail::form:: namespace

namespace fmr {
  template <typename V> static
  std::string form::si_unit
 (const V val, std::string unit, const int md, const bool is_signed) {
    const std::string sign
      = (val < V(0)) ? "-" : (is_signed ? ((val > V(0)) ? "+" : " ") : "");
    return detail::form::si_unit (std::abs (double (val)), unit, md, sign);
  }
  template <typename V> static
  std::string form::si_time
  (V val, const int md, const bool is_signed) {
    std::string unit = "s";
    if (val >= V(600)) {val /= V(60); unit="m";}
    if (val >= V(600)) {val /= V(60); unit="h";}
    const std::string sign
      = (val < V(0)) ? "-" : (is_signed ? ((val > V(0)) ? "+" : " ") : "");
    return detail::form::si_unit (std::abs (double (val)), unit, md, sign);
  }
  inline
  bool form::ends_with (const std::string& str, const std::string& end) {
    return str.size() >= end.size() &&
      str.compare(str.size() - end.size(), end.size(), end) == 0;
  }
  //
}// end fmr:: namespace

namespace femera {
  template <typename ...Args> static
  std::string form::text_line
  (fmr::Line_size_int line_width, const std::string& format, Args...args) {
    std::valarray<char> buf (line_width + 1);
    std::snprintf (&buf[0], buf.size(), format.c_str(), args...);
    return std::string (&buf[0]);
  }
  template <typename ...Args> static
  std::string form::text_line
  (FILE* f, fmr::Line_size_int l, const std::string& format, Args...args) {
    const auto line = form::text_line (l, format, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }
  template <typename V, typename ...Args> static
  std::string form::perf_line (const V count, const V secs,
    const std::string& unit) {
    const auto count_str = fmr::form::si_unit (count, unit);
    const auto  time_str = fmr::form::si_time (secs);
    const auto speed_str
      = fmr::form::si_unit (double (count)/double (secs), unit) + "/s";
    return text_line (250, "%s /%s, %s",
      count_str.c_str(), time_str.c_str(), speed_str.c_str());
  }
  template <typename ...Args> static
  std::string form::name_line
  (const fmr::Line_size_int name_width, const fmr::Line_size_int line_width,
    const std::string& name, const std::string& form, Args...args) {
    const auto format = "%*s "+ form;
    std::valarray<char> buf (line_width + 1);
    const auto nc = std::snprintf (&buf[0], buf.size(), format.c_str(),
      int (name_width), name.c_str(), args...);
    if (nc > line_width) {
#ifdef FMR_MICRO_UCHAR
      // Count unicode multibyte chars, e.g.. "\u00b5" or "\u03bc"
      // then reformat with longer length if present.
      fmr::Line_size_int c =0;
      const auto n = fmr::Line_size_int (buf.size());
      for (fmr::Line_size_int i=0; i < n; ++i) {
#if 1
        c += (buf[i]=='\xb5') ? 1u : 0u;
        c += (buf[i]=='\xbc') ? 1u : 0u;
#endif
#if 0
        c += (buf[i] > '\x7f') ? 1u : 0u;      // does not work; buf is signed
        c += (buf[i] < 0) ? 1u : 0u;           // finds too many characters
        c += (std::isprint (buf[i])) ? 0u : 1u;//does not work
#endif
      }
      if (c > 0) {
        buf.resize (line_width + 1 + c);
        std::snprintf (&buf[0], buf.size(), format.c_str(),
          int (name_width), name.c_str(), args...);
      }
#endif
    }
    return std::string (&buf[0]);
  }
  template <typename ...Args> static
  std::string form::name_line (FILE* f,
  const fmr::Line_size_int h, const fmr::Line_size_int l,
  const std::string& name, const std::string& form, Args...args) {
    const auto line = form::name_line (h, l, name, form, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }
  template <typename ...Args> static
  std::string form::time_line
  (const fmr::Line_size_int name_width, const fmr::Line_size_int line_width,
  const std::string& name, const std::string& form, Args...args) {
    //
    std::string timestr = "UTC "+fmr::form::utc_time();
    //"UTC 2022-02-02T14:29+00:00.000";  //TODO
    //
    const auto time_width = timestr.length () + 1;
    auto line = form::name_line (name_width, line_width, name, form, args...);
    const auto text_width = line_width - time_width;
    if (line.length () > text_width) {line = line.substr (0, text_width);}
    std::valarray<char> buf (line_width + 2);
    std::snprintf (&buf[0], buf.size(), "%-*s%*s",
      int (text_width), line.c_str(), int (time_width), timestr.c_str());
    return std::string(&buf[0]);
  }
  template <typename ...Args> static
  std::string form::time_line (FILE* f,
  const fmr::Line_size_int h, const fmr::Line_size_int l,
  const std::string& name, const std::string& form, Args...args) {
    const auto line = form::time_line (h, l, name, form, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }
}//end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif
