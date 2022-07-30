#include "../core.h"

#ifdef FMR_HAS_GTEST
#include "Gtst.hpp"
#endif
#include "Self.hpp"
#include "Perf.hpp"

#include <unistd.h>          // argc opterr optopt optind optarg

namespace femera {
  void test::Beds::task_init (int* argc, char** argv) {
    this->scan (argc, argv);
#ifdef FMR_HAS_GTEST
    this->add_task (std::move(Test<test::Gtst>::new_task (this->get_core())));
#endif
    if (this->do_test) {// -T option in args
      this->add_task (std::move(Test<test::Self>::new_task (this->get_core())));
    }
    if (this->do_test || this->do_perf) {// -T or -P option in args
      this->add_task (std::move(Test<test::Perf>::new_task (this->get_core())));
    }
  this->set_init (true);
  }
  void test::Beds::scan (int* argc, char** argv) {
    if (argc != nullptr && argv != nullptr) {
      FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
        int ac=argc[0];// Copy getopt variables.
        auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
        opterr = 0; int optchar;
        while ((optchar = getopt (argc[0], argv, "TP")) != -1) {
          // T  -T has no argument//NOTE -g gets eaten by MPI
          //TODO -fmr:T, -fmr:T is working because -f -m -r and -: are skipped,
          //     leaving only the recognized option -T, -P.
          switch (optchar) {
            case 'T':{ this->do_test = true; break; }
            //TODO: this->proc->add_optc (optchar); break; }
            case 'P':{ this->do_perf = true; break; }
        } }
        // Restore getopt variables.
        argc[0]=ac; opterr=oe; optopt=oo; optind=oi; optarg=oa;
  } } }
}//end femera namespace
