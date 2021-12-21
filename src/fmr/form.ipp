#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

#include <cmath>    // pow(), log10()
#include <cstring>  // strlen()
#include <vector>

#define makestr(s) str(s)
#define str(s) #s
namespace fmr {

template<typename V>
std::string form::si_unit_string (const V val, std::string unit, const int sf) {
  int log1000 = 0;
  std::string sign = (val < V(0.0)) ? "-" :"";
  double v = std::abs(double(val));
  if (unit.size()>8) { unit = unit.substr (0,8); }
  const double threshold = 0.95*std::pow (10.0, double(sf - 1));
  if (v > 1.0e-24) {
    log1000 = int(std::log10 (v))/3 - (v<1 ? 1 : 0);
    v *= std::pow (10.0, -3.0 * double(log1000));
    if (v < threshold) { v *= 1000.0; log1000 -= 1; }
  }
  const int i = log1000 + 6;
  const char prefix[]="afpnum kMGTPE";
  std::string pre = "?";
  if (i>=0 && i<std::strlen(prefix)) { pre = prefix[i]; }
#ifdef FMR_MICRO_UCHAR
  if (pre=="u")  { pre = makestr(\FMR_MICRO_UCHAR) ; }
#endif
  std::vector<char> buf(16,0);
  std::snprintf (&buf[0],15,"%s%4.0f %s%s",
    sign.c_str(), v, pre.c_str(), unit.c_str());
  return std::string(&buf[0]);
}
}// end fmr:: namespace
#undef str
#undef makestr
#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif