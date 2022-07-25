#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

// form.hpp needed by File.ipp variadic template methods
#include "../../fmr/form.hpp"

#if 0
/* Variadic read/save/send methods in File.ipp package heterogenous
  data into a fmr::Vals structure (or std::string?, vector<std::string>?)
  then call a non-variadic member function in File.cpp to pass it on to a
  specialized handler (in e.g. Text.cpp, Dlim.cpp, Loggs.cpp, Cgns.cpp,...)
  to handle that structure.
*//*
  std::size_t File::send (fmr::Data_name_t,                // returns bytes sent
    std::string lab1, std::string lab2, std::string lab3, std::string form, ...)
  data->send (fmr:log, fmr::Vals_type::Logs_line, this->proc->get_proc_id (),
    "this", "log", "msg", "i: %u, j: %u", uint (i), uint (j));
  data->send (fmr:log,// can be reduced to this for fmr:log ?
    "this", "log", "msg", "i: %u, j: %u", uint (i), uint (j));
*/
#endif

#include <unordered_map>
#include <valarray>           // used in File.ipp

namespace femera { namespace data {
  class File;// Derive as a CRTP concrete class from Data.
  class File final: public Data <File> {// private: friend class Data;
  private:
    using css = const std::string;
  public:
    bool did_logs_init () noexcept;
    std::size_t send (const fmr::Data_name_t&, css& text,
       fmr::Dim_int out_d=1)
    noexcept;
    template <typename ...Args>
    std::size_t send (const fmr::Data_name_t&,// standard fmr:log line
      css& lab1, css& lab2, css& lab3, css& form, Args...);
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    File (femera::Work::Core_ptrs_t) noexcept;
    File () =delete;//NOTE Use the constructor above.
#if 0
  public:
    using Open_file_id = uintptr_t;// cast-compatible with FILE* (stdout,stderr)
    // bad idea: do not use open file handles as file IDs
  private:
    std::unordered_map <File_id, Page_dims> file_data = {};
#endif
#if 0
  private:
    Path_list_t inp_file_list ={};// initialize these from command line args
    Path_list_t out_file_list ={};//               "
    Path_list_t log_file_list ={};
    Path_list_t err_file_list ={};
#endif
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
  };
} }//end femera::data namespace

#include "File.ipp"

//end FEMERA_HAS_FILE_HPP
#endif
