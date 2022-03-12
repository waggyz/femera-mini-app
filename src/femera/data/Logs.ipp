#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  data::Logs::Logs (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Femera logger";
    this->abrv      ="logs";
    this->task_type = task_cast (Plug_type::Logs);
    this->info_d    = 3;
  }
  inline
  void data::Logs::task_init (int*, char**) {
    // set default logger (data->fmrlog) to stdout only from the main thread
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      const auto P = this->proc->get_task (Plug_type::Fomp);
      if (P == nullptr) {
        n = 1;
      } else {
        n = P->get_proc_n ();// number of OpenMP threads / mpi process
    } }
    this->data->fmrlog = Data::File_ptrs_t (n, nullptr);
    if (n > 0) {this->data->fmrlog[0] = ::stdout;}
    this->data->did_logs_init = true;
  }
  inline
  void data::Logs::task_exit () {
  }
  template <typename ...Args> inline
  std::string data::Logs::data_line (Args... args) {
    return make_data_line (std::string(""), args...);
  }
  inline
  std::string data::Logs::csv_item (const std::string& str) {
    return +"\""+str+"\"";
  }
  inline
  std::string data::Logs::csv_item (const char* str) {
    return "\""+std::string(str)+"\"";
  }
  inline
  std::string data::Logs::csv_item (const float f) {
    std::vector<char> buf (15 + 1, 0);
    std::snprintf (&buf[0], buf.size(),"%1.7e", double(f));
    return std::string(&buf[0]);
  }
  inline
  std::string data::Logs::csv_item (const double f) {
    std::vector<char> buf (23 + 1, 0);
    std::snprintf (&buf[0], buf.size(),"%1.15E", f);
    return std::string(&buf[0]);
  }
  template <typename I> inline
  std::string data::Logs::csv_item (const I integer,
    typename std::enable_if<std::is_integral<I>::value >::type*) {
    return std::to_string (integer);
  }
  inline
  std::string data::Logs::make_data_line (const std::string line) {
    return line;
  }
  template <typename L> inline
  std::string data::Logs::make_data_line (const std::string line, const L last) {
    return line + data::Logs::csv_item (last);
  }
  template <typename F, typename ...R> inline
  std::string data::Logs::make_data_line
  (const std::string line, const F first, R... rest) {
    return make_data_line (line + data::Logs::csv_item (first)+",", rest...);
  }
}//end femera namespace

//end FEMERA_DATA_LOGS_IPP
#endif
