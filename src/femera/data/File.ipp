#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  void data::File::task_exit () {
  }
  template <typename ...Args> inline// standard fmr:log line
  fmr::Global_int data::File::send (const fmr::Data_name_t& data_name,
    const std::string& lab1, const std::string& lab2, const std::string& lab3,
    const std::string& form, Args...args) {
#if 0
//#include <algorithm>
//#include <unistd.h>   // isatty (FILE*)
    std::transform(out3.begin(), out3.end(), out3.begin(), ::tolower);
    const std::string norm = "\\e[0m";
    auto out3 = lab3;
    if (out3 == "fa""il") { out3 ="\\e[30m\\e[41m"+ lab3 + norm; }
    if (out3 == "wa""rn") { out3 ="\\e[07m"+ lab3 + norm; }
    if (out3 == "pa""ss") { out3 ="\\e[32m"+ lab3 + norm; }//TODO ANSI colors?
    if (out3 == "ok"    ) { out3 ="\\e[32m"+ lab3 + norm; }
#endif
    const auto line_width = uint (80);//TODO look up for data_name destination
    const auto msg = form::text_line (line_width, "%4s %4s %4s "+ form,
      lab1.c_str (), lab2.c_str (), lab3.c_str (), args...);
#ifdef FMR_DEBUG
    printf ((data_name+" "+msg+" in File.ipp\n").c_str ());
#endif
    // Call non-variadic method to find data_name handler
    // and output (append) to destination.
    return this->send (data_name, msg);
  }//
  //
}//end femera namespace
//end FEMERA_FILE_IPP
#endif
