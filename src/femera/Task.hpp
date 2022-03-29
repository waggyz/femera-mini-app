#ifndef FEMERA_HAS_TASK_HPP
#define FEMERA_HAS_TASK_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Task : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:
    bool do_exit_zero = false;// Used by Jobs to exit normally from MPI.
  public:
    std::string get_base_name ()               noexcept final override;
    //
    fmr::Exit_int   init (int*, char**)        noexcept final override;
    fmr::Exit_int   exit (fmr::Exit_int err=0) noexcept final override;
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    static constexpr
    This_spt new_task ()                  noexcept;
    static
    This_spt new_task (int*, char**)      noexcept;
    T*       get_task (fmr::Local_int)    noexcept;
    T*       get_task (Work::Task_path_t) noexcept;
  private:
    constexpr
    T* this_cast (Task*) noexcept;
    constexpr
    T* this_cast (Work*) noexcept;
  protected:// make it clear this class needs to be inherited
    Task (Work::Core_ptrs_t) noexcept;
    Task ()            =default;
    Task (const Task&) =default;
    Task (Task&&)      =default;// shallow (pointer) copyable
    Task& operator =
      (const Task&)    =default;
    ~Task ()           =default;
  };
}//end femera:: namespace

#include "Task.ipp"

//end FEMERA_HAS_TASK_HPP
#endif
