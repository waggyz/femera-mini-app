#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

// form.hpp and unistd.h needed by File.ipp variadic template methods
#include "../../fmr/form.hpp"
/*
 * Variadic read/save/send methods in File.ipp package heterogenous
 * data into a fmr::Vals structure (or std::string)
 * then call a non-variadic member function in File.cpp to pass it on to a
 * specialized handler (in, e.g., Text.cpp, Dlim.cpp, Logs.cpp, Cgns.cpp,...).
 */
namespace femera { namespace data {
  class File;// Derive as a CRTP concrete class from Data.
  class File final: public Data <File> { friend class Data;// interface to Data classes
  // handles Bank, Dlim, Logs, Text internal;
  // and CGNS, Gmsh, Moab, Pets external data handlers.
  // Bank : Vals : Bulk : hierarchy handles internal data (RRAM) organization.
  // - Vals are thread-local.
  // - Bulk are low-level allocators.
    private:
    using css = const std::string;// for convenience
  public:
    bool did_logs_init () noexcept;// true if first Logs initialized ok
    fmr::Dim_int get_verb () noexcept;// gets logs verbosity
    fmr::Dim_int set_verb (fmr::Dim_int) noexcept;// sets logs verbosity
    //
    // send (..) methods append data and return bytes sent
    template <typename ...Args>
    fmr::Global_int send (const fmr::Data_name_t&,// fmr::Data_type::Logs_line
      css& lab1, css& lab2, css& lab3, css& form, Args...);
    //
    fmr::Global_int send (const fmr::Data_name_t&,// fmr::Data_type::Text_line
      css& text) noexcept;
#if 0
    template <typename D>
    fmr::Global_int send (const fmr::Data_type, const D data)
    noexcept;// Data_type provides typeof the data expected for send (for cast?).
    template <typename V>
    fmr::Global_int send (const fmr::Data_name_t&,// fmr::Data_type::Text_line
      fmr::Global_int size)// use for new array in, e.g.,  databank
    noexcept;
#endif
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    File (femera::Work::Core_ptrs_t) noexcept;
    File () = delete;//NOTE Use the constructor above.
#if 0
  private:
    std::unordered_map <File_id, Page_dims> file_data = {};
#endif
#if 0
  private:
    Path_list_t inp_file_list ={};// initialize these from command line args
    Path_list_t out_file_list ={};//                   ""
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
  };//
} }//end femera::data namespace
  //
#include "File.ipp"
//end FEMERA_HAS_FILE_HPP
#endif
