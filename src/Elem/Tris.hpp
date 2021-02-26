#ifndef FMR_HAS_TRIS_HPP
#define FMR_HAS_TRIS_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Tris : public Elem {
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
    virtual ~Tris () noexcept=default;// Virtual destructor for virtual class
    Tris (Sims*) noexcept;
    Tris ()=delete;
    Tris (Tris const&)=delete;// not copyable
    Tris operator= (const Tris&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_TRIS_HPP
#endif
