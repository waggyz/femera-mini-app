#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  data::File::File (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Femera file handler";
    this->abrv      ="file";
    this->task_type = task_cast (Plug_type::File);
    this->info_d    = 2;
  }
  inline
  void data::File::task_exit () {
  }
  inline
  bool data::File::did_logs_init () {
    return this->logs_init_stat;
  }
  inline
  bool data::File::set_logs_init (const bool tf) {
    this->logs_init_stat = tf;
    return this->logs_init_stat;
  }
  template <typename ...Args> inline
  std::string data::File::text_line
  (const data::File_ptrs_t& flist, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    std::vector<char> buf (w + 1, 0);
    std::snprintf (&buf[0], buf.size(), form.c_str(), args...);
    const auto line = std::string (&buf[0]);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename ...Args> inline
  std::string data::File::text_line (const std::string& form, Args ...args) {
    return data::File::text_line (data::File_ptrs_t ({}), form, args...);
  }
  template <typename ...Args> inline
  std::string data::File::name_line (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    const auto h = this->file_name_sz [file];
    const auto line = femera::form::name_line (h, w, label, form, args...);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename ...Args> inline
  std::string data::File::name_line
  (const std::string& label, const std::string& form, Args ...args) {
    return data::File::name_line (data::File_ptrs_t ({}), label, form, args...);
  }
  template <typename ...Args> inline
  std::string data::File::name_time (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size ()];
    }
    const auto h = this->file_name_sz [file];
    const auto w = this->file_line_sz [file];
    const auto line = femera::form::name_time (h, w, label, form, args...);
    if (file != nullptr) { fprintf (file,"%s\n", line.c_str()); }
    return line;
  }
  template <typename ...Args> inline
  std::string data::File::name_time
  (const std::string& label, const std::string& form, Args ...args) {
    return data::File::name_time (data::File_ptrs_t ({}), label, form, args...);
  }
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
