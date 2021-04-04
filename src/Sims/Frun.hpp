#ifndef FMR_HAS_FRUN_HPP
#define FMR_HAS_FRUN_HPP
/** */

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
class Frun final : public Sims {//TODO Problably NOT derived from Sims or Work.
  //TODO Change Sims to Exec? Algo?
  /* This simulation scheduler runs all of the tasks that have been pre-loaded
   * into its task stack.
   */
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  public:
  protected:
  private:
  // methods ----------------------------------------------------------------
  protected:
    int prep () final override;
    int chck () final override;
    int exit_task (int  err) final override;
  private:
//    int init_task (int* argc, char** argv)final override;//TODO called?
    int run  () final override;
  public:
    virtual ~Frun  ()      noexcept=default;
    Frun           (Sims*) noexcept;
    Frun           ()              =delete;
    Frun           (Frun const&)   =delete;// not copyable
    Frun operator= (const Frun&)   =delete;
};
}//end Femera namespace
//end FMR_HAS_FRUN_HPP
#endif
