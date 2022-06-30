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
   * The base name, Vals_type (or vals name), and part(ition) number (or some
   * other identifying integer) together identify a data item (and a data
   * format handler if appropriate) for its source and destination(s).
   *//*
  // built-in Data_path_t (std::string)
  Data_path_t log ="fmr:log";// fmr:log default ::stdout from main thread only
  Data_path_t out ="fmr:out";// fmr:out default ::stdout from all threads
  Data_path_t err ="fmr:err";// fmr:err default ::stderr from all threads
  Data_path_t in  ="fmr:in" ;// fmr:in  default ::stdin  from mpi main threads
  //
  Data::send (fmr::Data_path_t, fmr::Vals_type/Vals_name_t, Local_int thread_id,
    std::string lab1, std::string lab2, std::string lab3, std::string form, ...)
  data->send (fmr:log, fmr::Vals_type::Logs_line, this->proc->get_proc_id (),
    "this", "log", "msg", "i: %u, j: %u", uint (i), uint (j));
  */
}
namespace femera { namespace data {
  using File_ptrs_t = std::vector <FILE*>;//TODO Replace with ? File_name_t
} }//end femera::data:: namespace
namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  public:
    std::string   get_base_abrv ()           noexcept final override;
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:
    static constexpr
    This_spt new_task (Work::Core_ptrs_t)  noexcept;
    //
    T* get_task (fmr::Local_int)                 noexcept;
    T* get_task (Work::Task_path_t)              noexcept;
    T* get_task (Work_type, fmr::Local_int ix=0) noexcept;
    T* get_task (Task_type, fmr::Local_int ix=0) noexcept;
  private:
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
