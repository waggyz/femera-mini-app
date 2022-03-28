#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

// used in File.ipp
//#include "Text.hpp"

#include <unordered_map>
#include <valarray>

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from Data.
  class File : public Data <File> {// private: friend class Data;
  public:
    File_ptrs_t fmrlog = {};// main proc to stdout set by Logs::task_init (..)
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
    File_ptrs_t fmrall = {::stdout};
#if 0
  public:
    using Open_file_id = uintptr_t;// cast-compatible with FILE* (stdout, stderr)
    //TODO bad idea: do not use open file handles as file IDs
  private:
    std::unordered_map <File_id, Page_dims> file_data = {};
#endif
  private:
    Path_list_t inp_file_list ={};// initialized from command line
    Path_list_t out_file_list ={};//              "
#if 1
    // TODO Replace below with femera::data::Page_dims struct map.
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
    std::unordered_map <FILE*, fmr::Line_size_int> line_name_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
#endif
    bool logs_init_stat = false;
  public:
    bool did_logs_init ();
    bool set_logs_init (bool);
  private:
    using ss = std::string;
#if 0
  private:
    using Dt  = fmr::Data_type;
    using Ft  = fmr::File_type;// fmr::File_type::Auto : by extension
    using Fnl = std::deque <std::string>;
  public:
    void set_default (Ft);
    //
    List_file_id  add_file (Fnl& name_list, Dt =Dt::All, Ft =Ft::Auto);
    List_file_id  inp_file (Fnl& name_list, Dt =Dt::All, Ft =Ft::Auto);
    List_file_id  out_file (Fnl& name_list, Dt =Dt::All, Ft =Ft::Auto);
    //
    List_file_id scan_file (Fnl& name_list={""}, Dt =Dt::All, Ft =Ft::Auto);
    List_file_id read_file (Fnl& name_list={""}, Dt =Dt::All, Ft =Ft::Auto);
    void         save_file (Fnl& name_list={""}, Dt =Dt::All, Ft =Ft::Auto);
    void         send_file (Fnl& name_list={""}, Dt =Dt::All, Ft =Ft::Auto);
#endif
  public:
    //TODO? Move to femera::data::Text and/or femera::data::Logs
    template <typename ...Args>
    ss text_line (const File_ptrs_t&,                 const ss& form, Args...);
    template <typename ...Args>
    ss text_line (                                    const ss& form, Args...);
    template <typename ...Args>
    ss name_line (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_line (                    const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_time (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_time (                    const ss& name, const ss& form, Args...);
#if 0
    ss wrap_line (File_ptrs_t, ss& name, ss& form,...);//TODO ?
    ss time_line (File_ptrs_t, ss& name, ss& form,...);
    ss data_line (File_ptrs_t, ss& name, ss& form,...);
    ss perf_line (File_ptrs_t, ss& name, ss& form,...);
    ss perf_line (ss name, ss& form,...);
#endif
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    File (femera::Work::Core_ptrs_t) noexcept;
    File () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data namespace

#include "File.ipp"

//end FEMERA_HAS_FILE_HPP
#endif
