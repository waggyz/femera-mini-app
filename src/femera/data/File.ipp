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
  std::string data::File::NEW_send (const fmr::Data_name_NEW_t& data_name,
    const std::string& lab1, const std::string& lab2, const std::string& lab3,
    const std::string& form, Args...args) {
    const auto line_width = uint (80);//TODO look up for data_name destination
    const auto msg = femera::form::text_line (line_width, "%4s %4s %4s "+ form,
      lab1.c_str (), lab2.c_str (), lab3.c_str (), args...);
#ifdef FMR_DEBUG
    printf ((msg+" "+data_name+"\n").c_str ());
#endif
   // call method to find data_name handler and output (append) to destination
    Data::NEW_send (data_name, msg);
    return msg;
  }
//TODO ======================== REMOVE BELOW ==================================
# if 1
  template <typename ...Args> inline
  std::string data::File::text_line                         // send (..)
  (const data::File_ptrs_t& flist, const std::string& form, Args ...args) {
# if 0
    const auto D = this->data->get_task (Task_type::Text);
//    const auto D = Work::cast_via_work<data::Text>(this->get_task (Task_type::Text));
    if (D != nullptr) {return D->text_line (flist, form, args...);}
    return "";
#else
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size ()];
    }
    const auto w = this->file_line_sz [file];
    std::valarray<char> buf (w + 1);// +1 for terminal null
    std::snprintf (&buf[0], buf.size(), form.c_str(), args...);
    const auto line = std::string (&buf[0]);
    if (file != nullptr) {
//      this->time.add_idle_time_now ();//TODO move to Text should fix timing
//      this->send (file_name, line);// or file_list, line_list
      const auto c = fprintf (file,"%s\n",line.c_str());
//      this->time.add_busy_time_now ();
      if (c > 0) { this->time.add_count (1, 0, 0, fmr::Perf_int(c)); }
    }
    return line;
#endif
  }
  template <typename ...Args> inline
  std::string data::File::text_line (const std::string& form, Args ...args) {
    return data::File::text_line (data::File_ptrs_t ({}), form, args...);
  }
  template <typename ...Args> inline //TODO move from File to Text handler.
  std::string data::File::name_line (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
# if 0
    if (this->did_logs_init ()) {
      const auto D = Work::cast_via_work<data::Text>(this->get_task (Task_type::Text));
//      const auto D = this->get_task (Task_type::Text);
      if (D != nullptr) {return D->name_line (flist, label, form, args...);}
    }
    return "";
#else
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
FMR_WARN_INLINE_OFF
      file = flist [this->proc->get_proc_id () % flist.size()];
FMR_WARN_INLINE_ON
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
#   endif
  }
  template <typename ...Args> inline
  std::string data::File::name_line
  (const std::string& label, const std::string& form, Args ...args) {
    return data::File::name_line (data::File_ptrs_t ({}), label, form, args...);
  }
  template <typename ...Args> inline
  std::string data::File::time_line (const data::File_ptrs_t& flist,
    const std::string& label, const std::string& form, Args ...args) {
    FILE* file = nullptr;
    if (flist.size () > 0 && this->proc != nullptr) {
      file = flist [this->proc->get_proc_id () % flist.size ()];
    }
    const auto h = this->line_name_sz [file];
    const auto w = this->file_line_sz [file];
    const auto line = femera::form::time_line (h, w, label, form, args...);
    if (file != nullptr) {
//      this->time.add_idle_time_now ();
      const auto c = fprintf (file,"%s\n", line.c_str());
//      this->time.add_busy_time_now ();
      if (c > 0) { this->time.add_count (1, 0, 0, fmr::Perf_int(c)); }
    }
    return line;
  }
  template <typename ...Args> inline
  std::string data::File::time_line
  (const std::string& label, const std::string& form, Args ...args) {
    return data::File::time_line (data::File_ptrs_t ({}), label, form, args...);
  }
#endif
//=============================================================================
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
