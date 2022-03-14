#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from Data.
  class File : public Data <File> { private: friend class Data;
  private:
    using ss   = std::string;
  public:
    File_ptrs_t fmrlog = {};// main proc to stdout set by Logs::task_init(..)
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
    File_ptrs_t fmrall = {::stdout};
#if 1
  public:
    using Data_id = uintptr_t;// cast-compatible with FILE* (for stdout, stderr)
  public:
    class Data_form_t {// for file and stream (e.g., stdout) reading and saving
    // describes local (partitioned) representation of data.
    public:
//TODO Data_ptrs_t data;// data handler; OR use std::vector<task_type>?
      File_ptrs_t    file_ptrs ={};
      //std::valarray<fmr::Dim_int> item_dims = {1};
      //fmr::Local_int line_n    = 0;// lines of data
      fmr::Local_int page_size = 0;// lines / page
      fmr::Enum_int  line_size = 0;// items / line
      fmr::Enum_int  item_size = 1;// vals  / item
      fmr::Dim_int   head_size = 0;// optional header size in lines
      fmr::Dim_int   name_size = 0;// optional line name size in chars
//      std::vector<Task_type> task_type = task_cast (Plug_type::None);
//      std::vector<File_type> file_type = File_type::Unknown;
    public:
      Data_form_t (fmr::Enum_int l, fmr::Local_int p=0, fmr::Enum_int i=1,
      fmr::Dim_int m=0, fmr::Dim_int h=0)
      : page_size (p), line_size (l), item_size (i), head_size(h),name_size(m){}
    };
  private:
    std::unordered_map <Data_id, Data_form_t> file_data = {};
#endif
#if 1
  private: // TODO Replace below with above.
    std::unordered_map <FILE*, fmr::Line_size_int> file_head_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
#endif
  public:
    bool did_logs_init = false;
#if 0
  private:
    using ss = std::string;
    using Dt = fmr::Data_type;
    using Ft = fmr::File_type;// fmr::File_type::Auto : by extension
  public:
    set_default (Ft);
    //
    void add_file (ss& file_name, Ft =Ft::Auto, Dt =Dt::All);
    void inp_file (ss& file_name, Ft =Ft::Auto, Dt =Dt::All);
    void out_file (ss& file_name, Ft =Ft::Auto, Dt =Dt::All);
    //
    void scan_file (ss& file_name="", Ft =Ft::Auto, Dt =Dt::All);
    void read_file (ss& file_name="", Ft =Ft::Auto, Dt =Dt::All);
    void save_file (ss& file_name="", Ft =Ft::Auto, Dt =Dt::All);
#endif
  public:
    template <typename ...Args>
    ss text_line (const File_ptrs_t&,                 const ss& form, Args...);
    template <typename ...Args>
    ss text_line (                                    const ss& form, Args...);
    template <typename ...Args>
    ss head_line (const File_ptrs_t&, const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_line (                    const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_time (const File_ptrs_t&, const ss& head, const ss& form, Args...);
    template <typename ...Args>
    ss head_time (                    const ss& head, const ss& form, Args...);
#if 0
    ss wrap_line (File_ptrs_t, ss& head, ss& form,...);//TODO
    ss time_line (File_ptrs_t, ss& head, ss& form,...);//TODO
    ss data_line (File_ptrs_t, ss& head, ss& form,...);//TODO
    ss perf_line (File_ptrs_t, ss& head, ss& form,...);//TODO
    ss perf_line (ss head, ss& form,...);
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
