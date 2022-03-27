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
    const auto all_n = this->proc->all_proc_n ();
    if (true) {//TODO detail
      fmr::Local_int mpi_n=0, omp_n=0;
      const auto Pmpi = this->proc->get_task (Plug_type::Fmpi);
      if (Pmpi) {mpi_n = Pmpi->get_proc_n ();}
      const auto Pomp = this->proc->get_task (Plug_type::Fomp);
      if (Pomp) {omp_n = Pomp->get_proc_n ();}
      this->data->name_line (data->fmrlog, "self task_init",
        "%4u MPI %4u OpenMP, %4u total processes", mpi_n, omp_n, all_n);
    }
#if 0
    const auto Pnode = this->proc->get_task (Plug_type::Node);
    const auto core_n = Pnode->node_n * Pnode->get_core_n ();
    if (all_n < core_n) {//TODO
      printf ("Femera uses fewer threads (%u) than physical cores (%u)\n",
        all_n, core_n);
    }
    if (all_n > core_n) {
      printf ("Femera uses more threads (%u) than physical cores (%u)\n",
        all_n, core_n);
    }
#endif
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
      const auto name = femera::form::text_line (40," %4s %4s %4s",
        this->get_base_name().c_str(), this->abrv.c_str(), "proc");
      const std::string text = std::to_string(ix);
      this->data->name_line (this->data->fmrout, name,
        "%4u proc[%u %% %u=%u]=%u%s", proc->get_proc_id (),
        ix, mod_n, ix%mod_n, pixs[ix%mod_n], proc->is_main() ? " *main":"");

      //TODO pixs = this->proc->gather ();
    }
#endif
  }
}//end femera namespace
