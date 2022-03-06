#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"
#include "../fmr/form.hpp"// used in Data.ipp

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
    File_ptrs_t fmrall = {::stdout};
  private:
    std::unordered_map <FILE*, fmr::Line_size_int> file_head_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
  public:
    fmr::Exit_int    init (int*, char**)        noexcept final override;
    fmr::Exit_int    exit (fmr::Exit_int err=0) noexcept final override;
    std::string  get_base_name ()               noexcept final override;
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    T*       get_task (fmr::Local_int)          noexcept;
    T*       get_task (Work::Task_path_t)       noexcept;
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
    T* derived (Data*) noexcept;
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
