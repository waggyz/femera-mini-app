#include "Beds.hpp"

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
      int ar=argc[0];// Copy getopt variables.
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
      argc[0]=ar; opterr=oe; optopt=oo; optind=oi; optarg=oa;
    }
#ifdef FMR_HAS_GTEST
    this->add_task (Test<test::Gtst>::new_task (this->get_core()));
#endif
    if (do_test) {// -T option in args
      this->add_task (Test<test::Self>::new_task (this->get_core()));
    }
    if (do_test || do_perf) {// -T or -P option in args
      this->add_task (Test<test::Perf>::new_task (this->get_core()));
  } }
}//end femera namespace
