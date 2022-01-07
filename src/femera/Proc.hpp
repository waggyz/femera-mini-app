#ifndef FEMERA_HAS_PROC_HPP
#define FEMERA_HAS_PROC_HPP

#include "Work.hpp"

namespace femera {
  namespace proc {
    using Team_t = uintptr_t;// cast-compatible with ::MPI_comm from mpi.h
  }
  template <typename T>
  class Proc : public Work {
  public:
  private:
    using This_t = std::shared_ptr<T>;
  protected:
    proc::Team_t   team_id     = 0;
    fmr::Local_int team_n      = 1;
    fmr::Local_int proc_ix     = 0;
    fmr::Local_int proc_n      = 1;
    //
    fmr::Local_int node_n      = 1;
  public:// because proc::Main must access it
    proc::Team_t   get_team_id () noexcept;
    fmr::Local_int get_team_n  () noexcept;
    //
    fmr::Local_int get_proc_ix () noexcept;
    fmr::Local_int get_proc_n  () noexcept;
    //
#if 0
    fmr::Local_int get_node_n       ();
    fmr::Local_int get_node_ix      ();
#endif
    static constexpr fmr::Local_int get_node_core_n  ();
    static constexpr fmr::Local_int get_node_hype_n  ();
    static fmr::Local_int get_node_numa_n  ();
    static fmr::Local_int get_node_core_ix ();
    static fmr::Local_int get_node_hype_ix ();
    static fmr::Local_int get_node_numa_ix ();
    static fmr::Local_int get_node_used_byte ();
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    This_t    get_task (fmr::Local_int);
    This_t    get_task (Work::Task_path_t);
    static constexpr
    This_t    new_task () noexcept;
    static constexpr
    This_t    new_task (const Work::Core_t) noexcept;
  private:
    T* derived (Proc*);
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
