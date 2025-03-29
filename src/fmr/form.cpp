#include "form.hpp"

#include <cmath>    // pow(), log10()
#include <cstring>  // strlen()
#include <cfloat>   // DBL_EPSILON
#include <chrono>
#include <array>
#include <codecvt>  //  deprecated in C++17?
#include <locale>

namespace femera {

  size_t form::utf8_visible_strlen(const std::string& str) {//TODO test this
    //NOTE AI generated, needs tests; consider moving to .ipp
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string utf32 = converter.from_bytes(str);
    size_t count = 0;
    for (char32_t ch : utf32) {
      // Skip zero-width characters and combining marks.
      if (ch != 0x200B && ch != 0x200C && ch != 0x200D &&// Zero-width characters
        (ch < 0x0300 || ch > 0x036F) &&  // Combining Diacritical Marks (CDM)
        (ch < 0x1AB0 || ch > 0x1AFF) &&  // CDM Extended
        (ch < 0x1DC0 || ch > 0x1DFF) &&  // CDM Supplement
        (ch < 0x20D0 || ch > 0x20FF) &&  // CDM for Symbols
        (ch < 0xFE20 || ch > 0xFE2F)) {  // Combining Half Marks
        ++count;
    } }
    return count;
    /* WAS:
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(str);
    return wide.length();
    */
  }
  std::string form::si_unit (// implemented here, not inline
  double v, std::string unit, const int min_digits, const std::string sign) {
    if (unit.size()>8) { unit = unit.substr (0,8); }
    const double threshold
      = (0.95 - DBL_EPSILON) * std::pow (10.0, double(min_digits - 1));
    int log1000 = 0;
    if (v > double(DBL_EPSILON) * 0.5) {//1.0e-24) {// was FLT_EPSILON
      log1000 = int(std::log10 (v)) / 3 - ((v < 1.0) ? 1 : 0);
      v *= std::pow (10.0, -3.0 * double(log1000));
      if (v < threshold) {v *= 1000.0; log1000--;}//return std::to_string(log1000);
    }// else {return std::string("???");}
    const int i = log1000 + 6;
    const char prefix[]="afpnum kMGTPE";
    std::string pre = "?";
    if (i >= 0 && i < int (std::strlen (prefix))) { pre = prefix[i]; }
#ifdef FMR_MICRO_UCHAR
    if (pre == "u")  {
      const std::string mu( MAKESTR(\FMR_MICRO_UCHAR) );
      pre = (mu =="\\u") ? "u" : mu ;
    }
#endif
    std::array <char,16> buf;
    std::snprintf (buf.data(), buf.size(), "%s%4.0f %s%s",
      sign.c_str(), v, pre.c_str(), unit.c_str());
    return std::string (buf.data());
  }//
  //
}// end femera:: namespace
namespace fmr {

  std::string form::utc_time () {
    std::string timestr="";
    const auto now_time = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast <std::chrono::milliseconds>
      (now_time.time_since_epoch()).count();
    const auto as_time_t = std::chrono::system_clock::to_time_t (now_time);
    struct tm tmbuf;
    std::array <char,32> buf;
    if (::gmtime_r (& as_time_t, & tmbuf)) {
      if (std::strftime (buf.data(), buf.size(),"%Y-%m-%d %H:%M:%S",& tmbuf)) {
        timestr = std::string (buf.data());
    } }
    std::snprintf (buf.data(), buf.size(), "%s.%03i",
      timestr.c_str(), int(ms % 1000));
    return std::string (buf.data());
  }
  /* The next two functions are from:
     https://stackoverflow.com/questions/8362094/replace-multiple-spaces-
     with-one-space-in-a-string
   */
  bool form::are_both_spaces (const char lhs, const char rhs) {
    // checks if two characters are the same
    return (lhs == rhs) && (lhs == ' ');
  }
  std::string form::delete_extra_spaces  ( std::string str ) {
    // removes extra spaces
    std::string::iterator new_end = std::unique(str.begin(),
      str.end(), form::are_both_spaces);
    str.erase (new_end, str.end());
    return str;
  }//
  //
}// end fmr:: namespace

#undef FMR_DEBUG
