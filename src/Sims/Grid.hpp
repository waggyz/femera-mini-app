#ifndef FMR_HAS_GRID_HPP
#define FMR_HAS_GRID_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Grid : public Geom {// Pure virtual? Grid?
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  // this->task may contain child grids
  public:
  protected:
  private:
  // methods ----------------------------------------------------------------
  public:
    int chck () override;
    int run  () override;
  protected:
#if 0
    int prep () override;
    int init_task (int* argc, char** argv) final override;
    int exit_task (int  err) final override;
#endif
  private:
  public:
    virtual ~Grid () noexcept=default;// Virtual destructor for virtual class
    Grid (Sims*) noexcept;
    Grid ()=delete;
    Grid (Grid const&)=delete;// not copyable
    Grid operator= (const Grid&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_GRID_HPP
#endif
