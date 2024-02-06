#ifndef FEMERA_HAS_PROC_HPP
#define FEMERA_HAS_PROC_HPP

#include "Work.hpp"

#include "../fmr/proc.hpp"
// "../fmr/proc.hpp" used in Proc.ipp

namespace femera {
  template <typename T>
  class Proc : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:                   // Set by child instances during task_init()...
    fmr::Team_int  team_id = 0;//
    fmr::Local_int team_n  = 1;//
    fmr::Local_int base_id = 0;//
    fmr::Local_int base_n  = 1;//
    fmr::Local_int proc_n  = 1;//
    fmr::Local_int proc_ix = 0;//
    fmr::Local_int main_ix = 0;// [0,proc_n-1] ...because proc::Main uses them.
  public:
    std::string   get_base_abrv ()           noexcept final override;
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:
    bool            is_main    (bool = true) noexcept;
    fmr::Local_int get_team_n  () noexcept;
    fmr::Local_int get_proc_n  () noexcept;
    fmr::Local_int all_proc_n  () noexcept;
    fmr::Local_int get_proc_ix () noexcept;// index at this level [0,proc_n-1]
    fmr::Local_int get_proc_id (fmr::Local_int id=0) noexcept;//global thread id
    //                 proc_id = base_id + base_n * proc_ix
    fmr:: Team_int get_team_id () noexcept;
    fmr::Local_int set_base_n  () noexcept;
    fmr::Local_int get_proc_n  (Work_type) noexcept;
    fmr::Local_int get_proc_n  (Task_type) noexcept;
    fmr::Local_int get_proc_ix (Work_type) noexcept;
    fmr::Local_int get_proc_ix (Task_type) noexcept;
    static constexpr
    This_spt new_task () noexcept;
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    T*       get_task (fmr::Local_int ix)       noexcept;
    T*       get_task (Work::Task_path_t)       noexcept;
    T*       get_task (Work_type, fmr::Local_int ix=0) noexcept;
    T*       get_task (Task_type, fmr::Local_int ix=0) noexcept;
  private:
    static constexpr
    T* this_cast (Proc*) noexcept;
    static constexpr
    T* this_cast (Work*) noexcept;
  protected:
    Proc (Work::Core_ptrs_t) noexcept;
    Proc ()            =default;
    Proc (const Proc&) =default;
    Proc (Proc&&)      =default;// shallow (pointer) copyable
    Proc& operator =
      (const Proc&)    =default;
    ~Proc ()           =default;
  };
}//end femera:: namespace

#include "Proc.ipp"

//end FEMERA_HAS_PROC_HPP
#endif
