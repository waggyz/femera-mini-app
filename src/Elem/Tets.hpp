#ifndef FMR_HAS_TETS_HPP
#define FMR_HAS_TETS_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Tets : public Elem {
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
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
    virtual ~Tets () noexcept=default;// Virtual destructor for virtual class
    Tets (Sims*) noexcept;
    Tets ()=delete;
    Tets (Tets const&)=delete;// not copyable
    Tets operator= (const Tets&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_TETS_HPP
#endif
