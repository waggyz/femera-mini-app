#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "proc/Main.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace fmr {//TODO Move to fmr.h ?
  using Data_name_t = std::string;/*
  * Data_name_t: sim, part, file, directory, or other data collection path and
  * (base) name.
  * The base name, Vals_type (or vals name), and an integer index (e.g.
  * partition number or thread ID), together identify a data item (and a format
  * handler if appropriate) for its source and destination(s). Each identifier
  * can map to an initial (input) source and final (output) data destinations.
  *//*
  // built-in data sources and destinations
  Data_name_t log ="fmr:log";// default ::stdout from main thread only
  Data_name_t out ="fmr:out";// default ::stdout from all threads
  Data_name_t err ="fmr:err";// default ::stderr from all threads
  Data_name_t in  ="fmr:in" ;// default ::stdin  to   each MPI thread
  // formatters: fmr::Vals_type::Info_line, fmr::Vals_type::Text_line, ...
  */
}//end fmr:: namespace
namespace femera { namespace data {
  using File_ptrs_t = std::vector <FILE*>;//TODO Replace with Data_name_t
} }//end femera::data:: namespace
namespace femera {
  template <typename T>
  class Data : public Work {
  private:// typedefs
    using This_spt = FMR_SMART_PTR<T>;
#if 0
  private:// variables
    std::unordered_map <fmr::Data_name_t, std::vector <T*>> inp_data_task ={};
    std::unordered_map <fmr::Data_name_t, std::vector <T*>> out_data_task ={};
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
    T* get_task (fmr::Data_name_t, fmr::Vals_type,   fmr::Local_int ix=0);
    T* get_task (fmr::Data_name_t, fmr::Vals_name_t, fmr::Local_int ix=0);
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
