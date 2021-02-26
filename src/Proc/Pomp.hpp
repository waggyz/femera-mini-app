#ifndef FMR_HAS_WORK_POMP_HPP
#define FMR_HAS_WORK_POMP_HPP
/** */
#ifndef FMR_HAS_OMP
#define FMR_HAS_OMP
#endif

#if 0
//#include "omp.h"    // Expose this later, if needed
#endif

#if 0
//NOTE These do not work here.
#define OMP_PROC_BIND true
#define OMP_PLACES cores
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf DEBUG
#endif

namespace Femera {
class Proc;
class Pomp final : public Proc{
  public:
    virtual ~Pomp () noexcept=default;
    Pomp (Proc*,Data*);
    Pomp () = delete;
    bool is_in_parallel () final override;
    bool is_master   ()    final override;
    int  get_proc_id ()    final override;
    int  get_proc_n  ()    final override;
    int  set_proc_n  (int) final override;
#if 0
    int run_sync (Sims*) final override;
    int run_list (Sims*) final override;
    int run_fifo (Sims*) final override;
#else
    int run (Sims*) final override;
#endif
//    int run_batch (Sims*) final override;
  protected:
    int  prep      () final override;
    int  init_task (int* argc, char** argv )final override;
    int  exit_task (int err )final override;
  private:
};

}//end Femera namespace, end FMR_HAS_WORK_OMP_HPP
#undef FMR_DEBUG
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Proc.hpp")
#endif
