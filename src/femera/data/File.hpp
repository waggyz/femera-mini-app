#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from Data.
  class File : public Data <File> { private: friend class Data;
  private:
    using ss = std::string;
  public:
    File_ptrs_t fmrlog = {};// main proc to stdout set by Logs::task_init(..)
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
    File_ptrs_t fmrall = {::stdout};
  private:
    std::unordered_map <FILE*, fmr::Line_size_int> file_head_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
  public:
    bool did_logs_init = false;
  public:
    template <typename ...Args>
    ss text_line (const File_ptrs_t,                 const ss& form, Args...);
    template <typename ...Args>
    ss text_line (                                   const ss& form, Args...);
    template <typename ...Args>
    ss head_line (const File_ptrs_t, const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_line (                   const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_time (const File_ptrs_t, const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_time (                   const ss& head, const ss& form, Args...);
#if 0
    ss wrap_line (File_ptrs_t, ss head, ss form,...);//TODO
    ss time_line (File_ptrs_t, ss head, ss form,...);//TODO
    ss data_line (File_ptrs_t, ss head, ss form,...);//TODO
    ss perf_line (File_ptrs_t, ss head, ss form,...);//TODO
    ss perf_line (ss head, ss form,...);
#endif
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    File (femera::Work::Core_ptrs_t) noexcept;
    File () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data namespace

#include "File.ipp"

//end FEMERA_HAS_FILE_HPP
#endif
