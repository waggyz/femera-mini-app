#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"
//#include "core.h"

namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
    using File_ptrs_t = std::vector<FILE*>;
  public:
//    data::Logs* log =nullptr;
    File_ptrs_t fmrlog = {::stdout};
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
  public:
    std::string text_line (const File_ptrs_t, const std::string form,...);
    std::string text_line (const std::string form,...);
    std::string head_line
      (const File_ptrs_t, const std::string head, const std::string form,...);
    std::string head_line
      (const std::string head, const std::string form,...);
//    std::string time_line (File_ptrs_t, std::string head, std::string form,...);
//    std::string time_line (std::string head, std::string form,...);
  public:
    fmr::Exit_int init     (int*, char**)        noexcept final override;
    fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept final override;
  public:
    This_spt  get_task_spt (fmr::Local_int);
    This_spt  get_task_spt (Work::Task_path_t);
    T*        get_task_raw (fmr::Local_int);
    T*        get_task_raw (Work::Task_path_t);
    static constexpr
    This_spt  new_task     (const Work::Core_ptrs) noexcept;
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

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
