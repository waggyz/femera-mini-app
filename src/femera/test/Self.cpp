#include "../core.h"
#include "Self.hpp"

#include <valarray>

#if 0
//#include "gtest/gtest.h"
TEST( SelfTest, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
namespace femera {
  void test::Self::task_init (int*, char**) {
#if 0
    const fmr::Local_int  all_n = 4;
    const fmr::Local_int each_n = 2;
    const fmr::Local_int  mod_n = all_n / each_n;
    std::valarray<fmr::Local_int> pixs (each_n);
    FMR_PRAGMA_OMP(omp parallel) {
      const auto ix = this->proc->get_proc_id ();
      if (ix % mod_n < each_n) {
        FMR_PRAGMA_OMP(omp atomic write)
        pixs [ix % mod_n] = ix;
      }
      else {}//TODO FMR_THROW()?
      FMR_PRAGMA_OMP(omp barrier)
      const auto head = femera::form::text_line (40," %4s %4s %4s",
        this->get_base_name().c_str(), this->abrv.c_str(), "proc");
      const std::string text = std::to_string(ix);
      this->data->head_line (this->data->fmrout, head,
        "%4u proc[%u %% %u=%u]=%u%s", proc->get_proc_id (),
        ix, mod_n, ix%mod_n, pixs[ix%mod_n], proc->is_main() ? " *main":"");

      //TODO pixs = this->proc->gather ();
    }
#endif
  }
}//end femera namespace
