#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

namespace femera {
  inline
  data::Logs::Logs (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="logging";
    this->abrv ="logs";
    this->task_type = task_cast (Plug_type::Logs);
    this->info_d = 3;
  }
  template <typename ...Args> inline
  std::string data::Logs::data_line (Args... args) {
    return data_line_p (std::string(""), args...);
  }
  inline
  std::string data::Logs::data_line_p (std::string line, std::string last) {
    return line +"\""+last+"\"";
  }
  template <typename ...Rest> inline
  std::string data::Logs::data_line_p
  (std::string line, std::string first, Rest... rest) {
    return data_line_p (line +"\""+first+"\"" +",", rest...);
  }
  inline
  std::string data::Logs::data_line_p (std::string line, const char* last) {
    return line +"\""+last+"\"";
  }
  template <typename ...Rest> inline
  std::string data::Logs::data_line_p
  (std::string line, const char* first, Rest... rest) {
    return data_line_p (line + "\""+first+"\"" +",", rest...);
  }
  inline
  std::string data::Logs::data_line_p (std::string line) {
    return line;
  }
  template <typename O> inline
  std::string data::Logs::data_line_p (std::string line, O only) {
    return line + std::to_string (only);//TODO snprintf(..)
  }
  template <typename F, typename ...Rest> inline
  std::string data::Logs::data_line_p (std::string line, F first, Rest... rest) {
    return data_line_p (line + std::to_string (first) +",", rest...);
  }
}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
