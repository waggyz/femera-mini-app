#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  void data::File::task_exit () {
  }
  template <typename ...Args> inline// standard fmr:log line
  std::size_t data::File::send (const fmr::Data_name_t& data_name,
    const std::string& lab1, const std::string& lab2, const std::string& lab3,
    const std::string& form, Args...args) {
    const auto line_width = uint (80);//TODO look up for data_name destination
    const auto msg = femera::form::text_line (line_width, "%4s %4s %4s "+ form,
      lab1.c_str (), lab2.c_str (), lab3.c_str (), args...);
#ifdef FMR_DEBUG
    printf ((msg+" "+data_name+" in File.ipp\n").c_str ());
#endif
   // call method to find data_name handler and output (append) to destination
    return this->send (data_name, msg);
  }
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
