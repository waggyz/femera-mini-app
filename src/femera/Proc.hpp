#ifndef FEMERA_HAS_PROC_HPP
#define FEMERA_HAS_PROC_HPP

#include "Work.hpp"

#include "../fmr/proc.hpp"// used in Proc.ipp
#include <string>         // used in Proc.ipp

namespace femera {
  namespace proc {
    using Team_t = uintptr_t;// cast-compatible with ::MPI_comm from mpi.h
  }
  template <typename T>
  class Proc : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:                   // Set by derived instances during task_init()...
    proc::Team_t   team_id = 0;//
    fmr::Local_int team_n  = 1;//
    fmr::Local_int base_id = 0;//
    fmr::Local_int base_n  = 1;//
    fmr::Local_int proc_n  = 1;//
    fmr::Local_int proc_ix = 0;//
    fmr::Local_int main_ix = 0;// [0,proc_n-1]  ...because proc::Main uses them.
  public:
    std::string get_base_name () noexcept final override;
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:
    bool            is_main    () noexcept;
    fmr::Local_int get_team_n  () noexcept;
    fmr::Local_int get_proc_n  () noexcept;
    fmr::Local_int all_proc_n  () noexcept;
    fmr::Local_int get_proc_ix () noexcept;// index at this level [0,proc_n-1]
    fmr::Local_int get_proc_id (fmr::Local_int id=0) noexcept;//global thread id
    //                 proc_id = base_id + base_n * proc_ix
    proc::Team_t   get_team_id () noexcept;
    fmr::Local_int set_base_n  ();
    static constexpr
    This_spt new_task ()                        noexcept;
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    T*       get_task (fmr::Local_int)          noexcept;
    T*       get_task (Work::Task_path_t)       noexcept;
  private:
    T*       derived   (Proc*) noexcept;
    T*       derived   (Work*) noexcept;
    Proc<T>* this_cast (Work*) noexcept;
  protected:
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
