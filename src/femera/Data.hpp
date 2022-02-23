#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"
//#include "core.h"

#include <unordered_map>
#include <tuple>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
    using ss = std::string;
  protected:
    using File_ptrs_t = std::vector<FILE*>;
  public:
    File_ptrs_t fmrlog = {};//{::stdout};
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
  private:
    std::unordered_map <FILE*, fmr::Line_size_int> file_head_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
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
    ss wrap_line (File_ptrs_t, ss head, ss form,...);
    ss head_time (File_ptrs_t, ss head, ss form,...);
    ss head_time (ss head, ss form,...);
    ss head_perf (File_ptrs_t, ss head, ss form,...);
    ss head_perf (ss head, std::string form,...);
#endif
  public:
    fmr::Exit_int init     (int*, char**)        noexcept final override;
    fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept final override;
    std::string get_base_name () final override;
  public:
#if 0
    This_spt  get_task_spt (fmr::Local_int);
    This_spt  get_task_spt (Work::Task_path_t);
#endif
    T*        get_task_raw (fmr::Local_int);
    T*        get_task_raw (Work::Task_path_t);
    static constexpr
    This_spt  new_task     (const Work::Core_ptrs_t) noexcept;
  private:
    T*        derived      (Data*);
  protected:// Make it clear this class needs to be inherited from.
    Data ()            =default;
    Data (const Data&) =default;
    Data (Data&&)      =default;// shallow (pointer) copyable
    Data& operator =
      (const Data&)    =default;
    ~Data ()           =default;
  };
}//end femera:: namespace
#undef FMR_DEBUG

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
