#ifndef FEMERA_HAS_SIMS_HPP
#define FEMERA_HAS_SIMS_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Sims : public Work {
  private:
    using This_t = std::shared_ptr<T>;
  public:
    fmr::Exit_int init     (int*, char**)        noexcept final override;
    fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept final override;
    This_t        get_task (fmr::Local_int i);
    This_t        get_task (Work::Path_t tree);
  private:
    T* derived (Sims*);
  protected:// make it clear this class needs to be inherited
    Sims ()            =default;
    Sims (const Sims&) =default;
    Sims (Sims&&)      =default;// shallow (pointer) copyable
    Sims& operator =
      (const Sims&)    =default;
    ~Sims ()           =default;
  };
}//end femera:: namespace

#include "Sims.ipp"

//end FEMERA_HAS_SIMS_HPP
#endif
