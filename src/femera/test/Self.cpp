#include "../core.h"
#include "Self.hpp"

#include <valarray>

namespace femera {
  void test::Self::task_init (int*, char**) {
    const fmr::Local_int  all_n = 4;
    const fmr::Local_int each_n = 2;
    const fmr::Local_int  mod_n = all_n / each_n;
    std::valarray<fmr::Local_int> pixs (each_n);
    FMR_PRAGMA_OMP(omp parallel) {
      const auto ix = proc->get_proc_id ();
      if (ix % mod_n < each_n) {
        FMR_PRAGMA_OMP(omp atomic write)
        pixs [ix % mod_n] = ix;
      }
      else {}//TODO FMR_THROW()?
#if 1
      FMR_PRAGMA_OMP(omp barrier)
      const auto head = femera::form::text_line (40," %4s %4s %4s",
        this->get_base_name().c_str(), this->abrv.c_str(), "proc");
      const std::string text = std::to_string(ix);
      this->data->head_line (this->data->fmrout, head,
        "%4u proc[%u %% %u=%u]=%u%s", proc->get_proc_id (),
        ix, mod_n, ix%mod_n, pixs[ix%mod_n], proc->is_main() ? " *main":"");
#endif
      //TODO pixs = this->proc->gather ();
    }
#if 0
    for (fmr::Local_int ix=0; ix<n; ix++) {if (pixs[ix] != ix) {
      //TODO FMR_THROW()?
      const auto head = femera::form::text_line (40," %4s %4s %4s",
        this->get_base_name().c_str(), this->abrv.c_str(), "ID");
      this->data->head_line (this->data->fmrerr, head,
        "ERROR in get_proc_id () %u != %u", pixs[ix], ix);
    } }
#endif
  }
}//end femera namespace
