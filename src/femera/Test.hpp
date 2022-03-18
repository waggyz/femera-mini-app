#ifndef FEMERA_HAS_TEST_HPP
#define FEMERA_HAS_TEST_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Test : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  public:
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    std::string get_base_name ()             noexcept final override;
  public:
    static constexpr
    This_spt  new_task (const Work::Core_ptrs_t) noexcept;
    T*        get_task (fmr::Local_int)          noexcept;
    T*        get_task (Work::Task_path_t)       noexcept;
  private:
    constexpr
    T* this_cast (Test*) noexcept;
    constexpr
    T* this_cast (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
    Test ()            =default;
    Test (const Test&) =default;
    Test (Test&&)      =default;// shallow (pointer) copyable
    Test& operator =
      (const Test&)    =default;
    ~Test ()           =default;
  };
}//end femera:: namespace

#include "Test.ipp"

//end FEMERA_HAS_TEST_HPP
#endif
