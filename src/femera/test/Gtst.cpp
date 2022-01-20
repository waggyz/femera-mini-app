#include "Gtst.hpp"
#include "../core.h"

#ifdef FMR_HAS_GTEST
#include "gtest/gtest.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void test::Gtst::task_init (int* argc, char** argv) {
    FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
      int argc2=argc[0];// Copy getopt variables.
      auto opterr2=opterr; auto optopt2=optopt;
      auto optind2=optind; auto optarg2=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, "T")) != -1){
        // T  -T has no argument//NOTE -g is eaten by MPI
        switch (optchar) {
          case 'T':{ this->is_enabled=true; break; }
          //this->proc->opt_add ('T'); break; }
      } }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
    }
    // Init even if not is_enabled as it could be enabled later.
    try {::testing::InitGoogleTest (argc, argv); }// Is this thread safe?
    catch (...) {
      FMR_THROW("error initializing GoogleTest");
      return;
    }
#ifdef FMR_HAS_MPI
    //from: https://github.com/google/googletest/issues/822
    ::testing::TestEventListeners& listeners
      = ::testing::UnitTest::GetInstance()->listeners();
    if ( !this->is_enabled | !this->proc->is_main()) {// Only report from main
      delete listeners.Release (listeners.default_result_printer());
    }
#endif
  }
  void test::Gtst::task_exit () {
    if (this->is_enabled) {
      int err =0;
#ifdef FMR_DEBUG
      printf ("Running GoogleTest...\n");
#endif
      try {err= RUN_ALL_TESTS(); }//NOTE macro runs regardless of being enabled
      catch (...) {//TODO Check if the macro is noexcept.
        FMR_THROW("error in GoogleTest RUN_ALL_TESTS()");
      }
      if (err) {
        auto msg = std::string("Warn""ing: GoogleTest returned ")
          + std::to_string(err) + ".";
        FMR_THROW(msg);
  } } }
}

#undef FMR_DEBUG
