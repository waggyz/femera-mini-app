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
  inline
  std::string data::Logs::data_line (std::string line) {
    return line;
  }
  template <typename O> inline
  std::string data::Logs::data_line (std::string line, O only) {
    return line + std::to_string (only);//TODO snprintf(..)
  }
  template <typename F, typename ...Rest> inline
  std::string data::Logs::data_line (std::string line, F first, Rest... rest) {
    //TODO move CSV builder to Logs::data_line
    return data_line (line + std::to_string (first) +",", rest...);
  }
  template <typename ...Args> inline
  std::string data::Logs::data_line (Args... args) {
    //TODO move CSV builder to Logs::data_line
    return data_line (std::string(""), args...);
  }
}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
