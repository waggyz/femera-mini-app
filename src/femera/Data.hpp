#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"
//#include "core.h"

namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_t = std::shared_ptr<T>;
  public:
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
#ifdef FMR_REMOVE_STUFF
    This_t    get_task (fmr::Local_int);
    This_t    get_task (Work::Task_path_t);//FIXME replace these with below
#endif
    T*    get_task_raw (fmr::Local_int);
    T*    get_task_raw (Work::Task_path_t);
    static constexpr
    This_t    new_task (const Work::Core_t) noexcept;
  private:
    T* derived (Data*);
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
