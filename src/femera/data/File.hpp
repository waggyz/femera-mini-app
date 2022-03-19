#ifndef FEMERA_HAS_FILE_HPP
#define FEMERA_HAS_FILE_HPP

#include "../Data.hpp"

// used in Data.ipp
#include "../../fmr/form.hpp"
#include <unordered_map>

namespace femera { namespace data {
  class File;// Derive a CRTP concrete class from Data.
  class File : public Data <File> { private: friend class Data;
  public:
    // TODO Make fmrlog, fmrout, fmrerr, fmrall into public functions?
    File_ptrs_t fmrlog = {};// main proc to stdout set by Logs::task_init (..)
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
    File_ptrs_t fmrall = {::stdout};
#if 1
  public:
    using Data_id = uintptr_t;// cast-compatible with FILE* (for stdout, stderr)
  public:// subclass ==========================================================//=======================================================================
  private:
    std::unordered_map <Data_id, Form_dims> file_data = {};
#endif
#if 1
  private: // TODO Replace below with above.
    std::unordered_map <FILE*, fmr::Line_size_int> file_name_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
#endif
    bool logs_init_stat = false;
  public:
    bool did_logs_init ();
    bool set_logs_init (bool);
  private:
    using ss   = std::string;
#if 0
  private:
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
    //TODO Move to femera::data::Text and/or femera::data::Logs
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
    ss wrap_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss time_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss data_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss perf_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss perf_line (ss name, ss& form,...);
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
