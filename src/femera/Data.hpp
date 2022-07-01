#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "proc/Main.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace fmr {//TODO Move to fmr.h ?
  using Data_path_t = std::string;/*
  * Data_path_t: sim, part, file, directory, or other data collection path and
  * (base) name, maps to initial (input) sources and final (output) data
  * destinations.
  * The base name, Vals_type (or vals name), and an integer index (e.g.
  * partition number or thread ID), together identify a data item (and a format
  * handler if appropriate) for its source and destination(s).
  *//*
  // built-in data sources and destinations
  Data_path_t log ="fmr:log";// fmr:log default ::stdout from main thread only
  Data_path_t out ="fmr:out";// fmr:out default ::stdout from all threads
  Data_path_t err ="fmr:err";// fmr:err default ::stderr from all threads
  Data_path_t in  ="fmr:in" ;// fmr:in  default ::stdin  to   each MPI thread
  // formatters: fmr::Vals_type::Info_line, fmr::Vals_type::Text_line, ...
  */
}
namespace femera { namespace data {
  using File_ptrs_t = std::vector <FILE*>;//TODO Replace with Data_path_t
} }//end femera::data:: namespace
namespace femera {
  template <typename T>
  class Data : public Work {
  private:// typedef
    using This_spt = FMR_SMART_PTR<T>;
#if 0
  private:// variable
    std::unordered_map<fmr::Data_path_t, std::vector<T*>> inp_path_task ={};
    std::unordered_map<fmr::Data_path_t, std::vector<T*>> out_path_task ={};
#endif
  public:// virtual methods
    std::string get_base_abrv () noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:// methods
    static constexpr
    This_spt new_task (Work::Core_ptrs_t) noexcept;
    //
    T* get_task (fmr::Local_int)                 noexcept;
    T* get_task (Work::Task_path_t)              noexcept;
    T* get_task (Work_type, fmr::Local_int ix=0) noexcept;
    T* get_task (Task_type, fmr::Local_int ix=0) noexcept;
  private:
#if 0
    T* get_task (fmr::Data_path_t, fmr::Vals_type,   fmr::Local_int ix=0);
    T* get_task (fmr::Data_path_t, fmr::Vals_name_t, fmr::Local_int ix=0);
#endif
    constexpr
    T* this_cast (Data*) noexcept;
    constexpr
    T* this_cast (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
    Data (Work::Core_ptrs_t) noexcept;// preferred constructor
    Data ()            =default;// early (init) constructor
    Data (Data&&)      =default;// movable
    Data& operator
      =  (const Data&) =delete; // not assignable
    Data (const Data&) =delete; // not copyable
    ~Data ()           =default;// destructor
  };
}//end femera:: namespace
#undef FMR_DEBUG

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
