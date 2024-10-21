#ifndef FEMERA_HAS_ALGO_HPP
#define FEMERA_HAS_ALGO_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Algo : public Work {// abstract class for materials (Algo)
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:
    bool do_exit_zero = false;// Used by Jobs to exit normally from MPI.
  public:
    std::string get_base_abrv ()               noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t) noexcept;
    static constexpr
    This_spt new_task ()                  noexcept;
    static
    This_spt new_task (int*, char**)      noexcept;
    T*       get_task (fmr::Local_int)    noexcept;
    T*       get_task (Work::Task_path_t) noexcept;
    T*       get_task (Work_type, fmr::Local_int ix=0) noexcept;
    T*       get_task (Task_type, fmr::Local_int ix=0) noexcept;
  private:
    static constexpr
    T* this_cast (Algo*) noexcept;
    static constexpr
    T* this_cast (Work*) noexcept;
  protected:// make it clear this class needs to be inherited
    Algo (Work::Core_ptrs_t) noexcept;
    Algo ()            =default;
    Algo (const Algo&) =default;
    Algo (Algo&&)      =default;// shallow (pointer) copyable
    Algo& operator =
      (const Algo&)    =default;
    ~Algo ()           =default;
  };
}//end femera:: namespace

#include "Algo.ipp"

//end FEMERA_HAS_ALGO_HPP
#endif
