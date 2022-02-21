#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  data::Logs::Logs (const femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="logging";
    this->abrv ="logs";
    this->task_type = task_cast (Plug_type::Logs);
    this->info_d = 3;
  }
  inline
  std::string data::Logs::csv_string (const std::string& str) {
    return +"\""+str+"\"";
  }
  inline
  std::string data::Logs::csv_string (const char* str) {
    return "\""+std::string(str)+"\"";
  }
  template <typename ...Args> inline
  std::string data::Logs::data_line (Args... args) {
    return data_line_p (std::string(""), args...);
  }
  inline
  std::string data::Logs::csv_string (const float f) {
    std::vector<char> buf (15 + 1, 0);
    std::snprintf (&buf[0], buf.size(), "%1.7e", double(f));
    return std::string(&buf[0]);
  }
  inline
  std::string data::Logs::csv_string (const double f) {
    std::vector<char> buf (31 + 1, 0);
    std::snprintf (&buf[0], buf.size(), "%1.15E", f);
    return std::string(&buf[0]);
  }
  template <typename I> inline
  std::string data::Logs::csv_string (const I integer) {
    return std::to_string (integer);
  }
  inline
  std::string data::Logs::data_line_p (const std::string line) {
    return line;
  }
  template <typename L> inline
  std::string data::Logs::data_line_p (const std::string line, const L last) {
    return line + data::Logs::csv_string (last);
  }
  template <typename F, typename ...Rest> inline
  std::string data::Logs::data_line_p
  (const std::string line, const F first, Rest... rest) {
    return data_line_p (line + data::Logs::csv_string (first) +",", rest...);
  }
}//end femera namespace

//end FEMERA_DATA_LOGS_IPP
#endif
