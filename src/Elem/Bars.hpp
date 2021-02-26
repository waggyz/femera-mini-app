#ifndef FMR_HAS_BARS_HPP
#define FMR_HAS_BARS_HPP
/** */

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Bars : public Elem {
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
    virtual ~Bars () noexcept=default;// Virtual destructor for virtual class
    Bars (Sims*) noexcept;
    Bars ()=delete;
    Bars (Bars const&)=delete;// not copyable
    Bars operator= (const Bars&)=delete;
};
}//end Femera namespace
#else
//end FMR_HAS_BARS_HPP
#endif
