#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

#include <cmath>    //TODO move to .cpp pow(), log10()
#include <cfloat>   // DBL_EPSILON

namespace femera {
  inline
  data::Logs::Logs (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="logging";
    this->abrv ="logs";
    this->task_type = task_cast (Plug_type::Logs);
    this->info_d = 3;
  }
  inline
  std::string data::Logs::to_string (float f) {
  std::vector<char> buf (15 + 1, 0);
  std::snprintf (&buf[0], buf.size(), "%1.7e", double(f));
  return std::string(&buf[0]);
  }
  inline
  std::string data::Logs::to_string (double f) {
  std::vector<char> buf (31 + 1, 0);
  std::snprintf (&buf[0], buf.size(), "%1.15E", f);
  return std::string(&buf[0]);
  }
  template <typename I> inline
  std::string data::Logs::to_string (I integer) {
    return std::to_string (integer);
  }
  template <typename ...Args> inline
  std::string data::Logs::data_line (Args... args) {
    return data_line_p (std::string(""), args...);
  }
  inline
  std::string data::Logs::data_line_p (std::string line) {
    return line;
  }
  inline
  std::string data::Logs::data_line_p (std::string line, std::string only) {
    return line +"\""+only+"\"";
  }
  template <typename ...Rest> inline
  std::string data::Logs::data_line_p
  (std::string line, std::string first, Rest... rest) {
    return data_line_p (line +"\""+first+"\"" +",", rest...);
  }
  inline
  std::string data::Logs::data_line_p (std::string line, const char* only) {
    return line +"\""+only+"\"";
  }
  template <typename ...Rest> inline
  std::string data::Logs::data_line_p
  (std::string line, const char* first, Rest... rest) {
    return data_line_p (line + "\""+first+"\"" +",", rest...);
  }
  template <typename O> inline
  std::string data::Logs::data_line_p (std::string line, O only) {
    return line + data::Logs::to_string (only);//TODO snprintf(..)
  }
  template <typename F, typename ...Rest> inline
  std::string data::Logs::data_line_p (std::string line, F first, Rest... rest){
    return data_line_p (line + data::Logs::to_string (first) +",", rest...);
  }
}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
