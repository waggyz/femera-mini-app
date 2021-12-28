#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Proc : public Work {
  private:
    using This_t = std::shared_ptr<T>;
  public:
    fmr::Exit_int init     (int*, char**)        noexcept override;
    fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept override;
    This_t        get_task (size_t i);
    This_t        get_task (std::vector<size_t> tree);
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

//end FEMERA_TEST_HPP
#endif
