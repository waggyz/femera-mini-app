#include "../core.h"

#ifdef FMR_HAS_GTEST
#include "Gtst.hpp"
#endif
#include "Self.hpp"
#include "Perf.hpp"

#include <unistd.h>          // argc opterr optopt optind optarg

namespace femera {
  void test::Beds::task_init (int* argc, char** argv) {
    bool do_test=false, do_perf=false;
    FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
      int ac=argc[0];// Copy getopt variables.
      auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, "TP")) != -1){
        // T  -T has no argument//NOTE -g gets eaten by MPI
        switch (optchar) {
          case 'T':{ do_test = true; break; }
          //TODO: this->proc->opt_add (optchar); break; }
          case 'P':{ do_perf = true; break; }
      } }
      // Restore getopt variables.
      argc[0]=ac; opterr=oe; optopt=oo; optind=oi; optarg=oa;
    }
#ifdef FMR_HAS_GTEST
    this->add_task (std::move(Test<test::Gtst>::new_task (this->get_core())));
#endif
    if (do_test) {// -T option in args
      this->add_task (std::move(Test<test::Self>::new_task (this->get_core())));
    }
    if (do_test || do_perf) {// -T or -P option in args
      this->add_task (std::move(Test<test::Perf>::new_task (this->get_core())));
  } }
  void test::Beds::task_exit () {
    if (true) {
      const auto all_n = this->proc->all_proc_n ();
      fmr::Local_int mpi_n=0, omp_n=0;
      const auto Pmpi = this->proc->get_task (Plug_type::Fmpi);
      if (Pmpi) {mpi_n = Pmpi->get_proc_n ();}
      const auto Pomp = this->proc->get_task (Plug_type::Fomp);
      if (Pomp) {omp_n = Pomp->get_proc_n ();}
      this->data->name_line (data->fmrlog, "beds task_exit",
        "%4u MPI %4u OpenMP, %4u total CPU processes", mpi_n, omp_n, all_n);
    }
  }
}//end femera namespace
