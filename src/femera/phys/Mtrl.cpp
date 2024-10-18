//#include "../core.h"

#include "Mtrl.hpp"

//#include "FE.hpp"
//#include "FD.hpp"
//#include "FV.hpp"

//#include <unistd.h>          // argc opterr optopt optind optarg

namespace femera {
#if 0
  void phys::Mtrl::task_init (int* argc, char** argv) {
    this->scan (argc, argv);
#else
  void phys::Mtrl::task_init (int*, char**) {
#endif
    this->set_init (true);
  }
#if 0
  void phys::mtrl::scan (int* argc, char** argv) {
    if (argc != nullptr && argv != nullptr) {
      FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
        int ac=argc[0];// Copy getopt variables.
        auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
        opterr = 0; int optchar;
        while ((optchar = getopt (argc[0], argv, "TP")) != -1) {
          // T  -T has no argument//NOTE -g gets eaten by MPI
          //NOTE -fmr:T, -fmr:P are working because -f -m -r and -: are skipped,
          //     leaving only the recognized option -T, -P.
          switch (optchar) {
            case 'T':{ this->do_test = true; break; }
            //TODO: this->proc->add_optc (optchar); break; }
            case 'P':{ this->do_perf = true; break; }
        } }
        // Restore getopt variables.
        argc[0]=ac; opterr=oe; optopt=oo; optind=oi; optarg=oa;
  } } }
#endif
}//end femera namespace
