#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

#include <cmath>    // std::abs()
#include <cstdio>     // std::snprintf
#include <vector>

namespace fmr { namespace detail { namespace form {

  std::string si_unit (const double val, std::string unit,
    const int min_digits, const std::string signchar);

} } }// end fmr::detail::form:: namespace

namespace fmr {

  template<typename V>
  std::string form::si_unit (const V val, std::string unit,
  const int md, const bool is_signed) {
    const std::string sign = (val < V(0)) ? "-" :
      (is_signed ? ((val > V(0)) ? "+" : " ") : "");
    return detail::form::si_unit (std::abs(double(val)), unit, md, sign);
  }
  template<typename V>
  std::string form::si_time (V val,
  const int md, const bool is_signed) {
    std::string unit = "s";
    if (val > V(599)) { val /= 60; unit="m"; }
    if (val > V(599)) { val /= 60; unit="h"; }
    const std::string sign = (val < V(0)) ? "-" :
      (is_signed ? ((val > V(0)) ? "+" : " ") : "");
    return detail::form::si_unit (std::abs(double(val)), unit, md, sign);
  }

}// end fmr:: namespace

namespace femera {

  template<typename ...Args>
  std::string form::text_line
  (fmr::Line_size_int line_width, const std::string& format, Args ...args) {
    std::vector<char> buf (line_width + 1, 0);
    std::snprintf (&buf[0], buf.size(), format.c_str(), args...);
    return std::string(&buf[0]);
  }
  template<typename ...Args>
  std::string form::text_line
  (FILE* f, fmr::Line_size_int l, const std::string& format, Args ...args) {
    const auto line = form::text_line (l, format, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }
  template<typename ...Args>
  std::string form::head_line
  (const fmr::Line_size_int head_width, const fmr::Line_size_int line_width,
  const std::string& head, const std::string& form, Args ...args) {
    const auto format = "%*s "+ form;
    std::vector<char> buf (line_width + 1, 0);
    std::snprintf (&buf[0], buf.size(), format.c_str(),
      int(head_width), head.c_str(), args...);
    return std::string(&buf[0]);
  }
  template<typename ...Args>
  std::string form::head_line (FILE* f,
  const fmr::Line_size_int h, const fmr::Line_size_int l,
  const std::string& head, const std::string& form, Args ...args) {
    const auto line = form::head_line (h, l, head, form, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }
  template<typename ...Args>
  std::string form::head_time
  (const fmr::Line_size_int head_width, const fmr::Line_size_int line_width,
  const std::string& head, const std::string& form, Args ...args) {
    //
    std::string timestr = "UTC "+fmr::form::utc_time();//"UTC 2022-02-02T14:29+00:00.000";  //TODO
    //
    const auto time_width = timestr.length () + 1;
    auto line = form::head_line (head_width, line_width, head, form, args...);
    const auto text_width = line_width - time_width;
    if (line.length () > text_width) { line = line.substr (0, text_width); }
    std::vector<char> buf (line_width + 2, 0);
    std::snprintf (&buf[0], buf.size(), "%-*s%*s",
      int(text_width), line.c_str(), int(time_width), timestr.c_str());
    return std::string(&buf[0]);
  }
  template<typename ...Args>
  std::string form::head_time (FILE* f,
  const fmr::Line_size_int h, const fmr::Line_size_int l,
  const std::string& head, const std::string& form, Args ...args) {
    const auto line = form::head_time (h, l, head, form, args...);
    fprintf (f,"%s\n", line.c_str());
    return line;
  }

}//end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif
