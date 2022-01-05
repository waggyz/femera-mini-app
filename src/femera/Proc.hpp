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
    proc::Team_t team_id = 0;
  public:
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    This_t    get_task (fmr::Local_int);
    This_t    get_task (Work::Task_path_t);
    static constexpr
    This_t    new_task () noexcept;
    static constexpr
    This_t    new_task (const Work::Core_t) noexcept;
    //
    proc::Team_t get_team_id () noexcept;
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
