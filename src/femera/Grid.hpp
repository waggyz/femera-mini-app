#ifndef FEMERA_HAS_GRID_HPP
#define FEMERA_HAS_GRID_HPP

#include "Work.hpp"

namespace femera {
  template <typename T>
  class Grid : public Work {// abstract class for materials (Grid)
  //TODO change Grid to Geom?
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:
    bool do_exit_zero = false;// Used by Jobs to exit normally from MPI.
  public:
    std::string get_base_abrv ()             noexcept final override;
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
    T* this_cast (Grid*) noexcept;
    static constexpr
    T* this_cast (Work*) noexcept;
  protected:// make it clear this class needs to be inherited
    Grid (Work::Core_ptrs_t) noexcept;
    Grid ()            =default;
    Grid (const Grid&) =default;
    Grid (Grid&&)      =default;// shallow (pointer) copyable
    Grid& operator =
      (const Grid&)    =default;
    ~Grid ()           =default;
  };
}//end femera:: namespace

#include "Grid.ipp"

//end FEMERA_HAS_GRID_HPP
#endif
