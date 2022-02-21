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
  bool test::Gtst::do_enable (int* argc, char** argv) {
    if (this->is_enabled) {
      FMR_THROW("GoogleTest is already enabled");
      return this->is_enabled;
    } else {
      try {::testing::InitGoogleTest (argc, argv); }// Is this thread safe?
      catch (...) {
        FMR_THROW("error initializing GoogleTest");
        this->is_enabled = false;
        return this->is_enabled;
    } }
    this->is_enabled = true;
    return this->is_enabled;
  }
  void test::Gtst::task_init (int* argc, char** argv) {
    FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
      int ar=argc[0];// Copy getopt variables.
      auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, "T")) != -1){
        // T  -T has no argument//NOTE -g is eaten by MPI
        switch (optchar) {
          case 'T':{ this->do_enable (argc,argv); break; }
          //this->proc->opt_add ('T'); break; }
      } }
      // Restore getopt variables.
      argc[0]=ar; opterr=oe; optopt=oo; optind=oi; optarg=oa;
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
    if (!this->is_enabled) {
      FMR_THROW("enable GoogleTest before RUN_ALL_TESTS()");
      return;
    }
    if (this->do_all_tests_on_exit) {
      if (this->did_run_all_tests) {
        FMR_THROW("already called GoogleTest RUN_ALL_TESTS()");
        return;
      }
      if (::testing::UnitTest::GetInstance()->test_suite_to_run_count() > 0) {
        int err =0;
#ifdef FMR_DEBUG
        printf ("running GoogleTest...\n");
#endif
        try {err= RUN_ALL_TESTS(); }//NOTE macro runs regardless of being enabled
        catch (...) {//TODO Check if the macro is noexcept.
          FMR_THROW("err""or in GoogleTest RUN_ALL_TESTS()");
          return;
        }
        if (err) {
          const auto msg = std::string("warn""ing: GoogleTest returned ")
            + std::to_string(err) + ".";
          FMR_THROW(msg);
          return;
        }
        this->did_run_all_tests = true;
    } }
  }
}

#undef FMR_DEBUG
