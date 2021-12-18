#ifndef FMR_HAS_FORM_IPP
#define FMR_HAS_FORM_IPP

#include <cmath>    // pow(), log10()
#include <cstring>  // strlen()
#include <vector>

namespace fmr {

template<typename V>
std::string form::si_unit_string (const V val, std::string unit, const int sf) {
  int log1000 = 0;
  double v = double(val);
  if (unit.size()>8) { unit = unit.substr (0,8); }
  const double threshold = 0.95*std::pow (10.0, double(sf - 1));
  if (std::abs(v) > 1.0e-24) {
    log1000 = int(std::log10 (std::abs(v)))/3 - (std::abs(v)<1 ? 1 : 0);
    v = double(val) * std::pow (10.0, double(-3 * log1000));
    if (v < threshold) { v *= 1000.0; log1000 -= 1; }
  }
  const int i = log1000 + 6;
  const char prefix[]="afpnum kMGTPE";
  std::string pre = "?";
  if (i>=0 && i<std::strlen(prefix)) { pre = prefix[i]; }
#ifdef FMR_MICRO_PREFIX
  if (pre=="u") { pre=""FMR_MICRO_PREFIX; }
#endif
  std::vector<char> buf(16,0);
  std::snprintf (&buf[0],15,"%4.0f %s%s", v, pre.c_str(), unit.c_str());
  return std::string(&buf[0]);
}
}// end fmr:: namespace
#undef FMR_DEBUG
//end FMR_HAS_FORM_IPP
#endif
