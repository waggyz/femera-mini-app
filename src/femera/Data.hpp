#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"
#include "../fmr/form.hpp"// used in Data.ipp

#include <unordered_map>
#include <tuple>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:
    using File_ptrs_t = std::vector<FILE*>;
  public:
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    std::string   get_base_name ()           noexcept final override;
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    T*       get_task (fmr::Local_int)          noexcept;
    T*       get_task (Work::Task_path_t)       noexcept;
    T*       get_task (Task_type, fmr::Local_int ix=0) noexcept;
    T*       get_task (Plug_type, fmr::Local_int ix=0) noexcept;
  private:
    T* derived (Data*) noexcept;
    T* derived (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
    Data ()            =default;
    Data (const Data&) =default;
    Data (Data&&)      =default;// shallow (pointer) copyable
    Data& operator =
      (const Data&)    =default;
    ~Data ()           =default;
  };
}//end femera:: namespace
#undef FMR_DEBUG

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
