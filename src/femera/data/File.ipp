#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  void data::File::task_exit () {
  }
  inline
  bool data::File::did_logs_init ()
  noexcept {
    return this->logs_init_tf;
  }
  inline
  bool data::File::set_logs_init (const bool tf)
  noexcept {
    this->logs_init_tf = tf;
#if 0
    if (tf) {
      auto D = Work::cast_via_work<data::Text>(this->get_task (Task_type::Text));
      if (D != nullptr) {
        D->file_line_sz = this->file_line_sz;
        D->line_name_sz = this->line_name_sz;
    } }
#endif
    return this->logs_init_tf;
  }
  template <typename ...Args> inline// standard fmr:log line
  std::size_t data::File::NEW_send (const fmr::Data_name_NEW_t& data_name,
    const std::string& lab1, const std::string& lab2, const std::string& lab3,
    const std::string& form, Args...args) {
    const auto line_width = uint (80);//TODO look up for data_name destination
    const auto msg = femera::form::text_line (line_width, "%4s %4s %4s "+ form,
      lab1.c_str (), lab2.c_str (), lab3.c_str (), args...);
#ifdef FMR_DEBUG
    printf ((msg+" "+data_name+" in File.ipp\n").c_str ());
#endif
   // call method to find data_name handler and output (append) to destination
    return this->NEW_send (data_name, msg);
  }
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
