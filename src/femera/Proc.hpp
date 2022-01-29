#ifndef FEMERA_HAS_PROC_HPP
#define FEMERA_HAS_PROC_HPP

#include "Work.hpp"

namespace femera {
  namespace proc {
    using Team_t = uintptr_t;// cast-compatible with ::MPI_comm from mpi.h
  }
  template <typename T>
  class Proc : public Work {
  private:
    using This_t = std::shared_ptr <T>;
  protected:                   // Set by derived instances during task_init()...
    proc::Team_t   team_id = 0;
    fmr::Local_int team_n  = 1;
//    fmr::Local_int proc_id = 0;
    fmr::Local_int proc_ix = 0;// index at this level [0,proc_n-1]
    fmr::Local_int proc_n  = 1;
    fmr::Local_int main_ix = 0;
    fmr::Local_int base_id = 0;
    fmr::Local_int base_n  = 1;
  public:                      //              ...because proc::Main uses these.
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    //
    bool            is_main    ();
    proc::Team_t   get_team_id () noexcept;
    fmr::Local_int get_team_n  () noexcept;
    fmr::Local_int get_proc_ix () noexcept;
    fmr::Local_int get_proc_n  () noexcept;
    fmr::Local_int set_base_n  ();
    fmr::Local_int all_proc_n  ();
    fmr::Local_int get_proc_id ();// global thread identifier
    // proc_id = base_id + base_n * proc_ix
    //
    static constexpr
    This_t new_task     () noexcept;
    static constexpr
    This_t new_task     (const Work::Core_t) noexcept;
    T*     get_task_raw (fmr::Local_int);
    T*     get_task_raw (Work::Task_path_t);
  private:
    This_t get_task_spt (fmr::Local_int);//FIXME Remove these?
    This_t get_task_spt (Work::Task_path_t);
  private:
    T*          derived (Proc*) noexcept;
    T*          derived (Work*) noexcept;
    Proc<T>*  this_cast (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
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
