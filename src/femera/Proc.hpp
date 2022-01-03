#ifndef FEMERA_HAS_PROC_HPP
#define FEMERA_HAS_PROC_HPP

#include "Work.hpp"
//#include "core.h"

namespace femera {
  template <typename T>
  class Proc : public Work {
  private:
    using This_t = std::shared_ptr<T>;
  public:
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    This_t    get_task (fmr::Local_int);
    This_t    get_task (Work::Path_t);
    static constexpr
    This_t    new_task () noexcept;
    static
    This_t    new_task (int*, char**) noexcept;
  private:
    T* derived (Proc*);
  protected:// make it clear this class needs to be inherited
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
