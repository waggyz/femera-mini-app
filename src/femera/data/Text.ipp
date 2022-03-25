#ifndef FEMERA_DATA_TEXT_IPP
#define FEMERA_DATA_TEXT_IPP

namespace femera {
  inline
  data::Text::Text (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Femera text file handler";
    this->abrv      ="text";
    this->task_type = task_cast (Plug_type::Text);
    this->info_d    = 3;
  }
  inline
  void data::Text::task_init (int*, char**) {
  }
  inline
  void data::Text::task_exit () {
  }
  template <typename ...Args> inline
  std::string data::Text::text_line
  (const data::File_ptrs_t& flist, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    std::vector<char> buf (w + 1, 0);
    std::snprintf (buf.data(), buf.size(), form.c_str(), args...);
    const auto line = std::string (buf.data());
    if (file != nullptr) {
//      this->time.add_idle_time_now ();//TODO move to Text should fix timing
      const auto c = fprintf (file,"%s\n", line.c_str());
//      this->time.add_busy_time_now ();
      if (c > 0) { this->time.add_count (1, 0, 0, fmr::Perf_int(c)); }
    }
    return line;
  }
  template <typename ...Args> inline
  std::string data::Text::text_line (const std::string& form, Args ...args) {
    return data::Text::text_line (data::File_ptrs_t ({}), form, args...);
  }
  template <typename ...Args> inline
  std::string data::Text::name_line (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size()];
    }
    const auto w = this->file_line_sz [file];
    const auto h = this->line_name_sz [file];
    const auto line = femera::form::name_line (h, w, label, form, args...);
    if (file != nullptr) {
//      this->time.add_idle_time_now ();
      const auto c = fprintf (file,"%s\n", line.c_str());
//      this->time.add_busy_time_now ();
      if (c > 0) { this->time.add_count (1, 0, 0, fmr::Perf_int(c)); }
    }
    return line;
  }
  template <typename ...Args> inline
  std::string data::Text::name_line
  (const std::string& label, const std::string& form, Args ...args) {
    return data::Text::name_line (data::File_ptrs_t ({}), label, form, args...);
  }
  template <typename ...Args> inline
  std::string data::Text::name_time (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size ()];
    }
    const auto h = this->line_name_sz [file];
    const auto w = this->file_line_sz [file];
    const auto line = femera::form::name_time (h, w, label, form, args...);
    if (file != nullptr) {
//      this->time.add_idle_time_now ();
      const auto c = fprintf (file,"%s\n", line.c_str());
//      this->time.add_busy_time_now ();
      if (c > 0) { this->time.add_count (1, 0, 0, fmr::Perf_int(c)); }
    }
    return line;
  }
  template <typename ...Args> inline
  std::string data::Text::name_time
  (const std::string& label, const std::string& form, Args ...args) {
    return data::Text::name_time (data::File_ptrs_t ({}), label, form, args...);
  }
}//end femera namespace

//end FEMERA_DATA_TEXT_IPP
#endif
