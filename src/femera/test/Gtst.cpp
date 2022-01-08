#include "Gtst.hpp"

#ifdef FMR_HAS_GTEST
#include "gtest/gtest.h"
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
          case 'T':{ this->is_enabled=true; break; }//this->proc->opt_add ('T'); break; }
      } }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
    }
    // Init even if not is_enabled as it could be enabled later.
    try {::testing::InitGoogleTest (argc, argv); }// Is this thread safe?
    catch (...) {
      throw std::runtime_error ("error initializing GoogleTest");
      return;
    }
#ifdef FMR_HAS_MPI
    //from: https://github.com/google/googletest/issues/822
    ::testing::TestEventListeners& listeners
      = ::testing::UnitTest::GetInstance()->listeners();
    if (this->is_enabled) {
      if (false) {//FIXME !this->proc->is_master()){
        delete listeners.Release (listeners.default_result_printer());
    } }
    else {
      delete listeners.Release (listeners.default_result_printer());
    }
#endif
  }
  void test::Gtst::task_exit () {
    if (this->is_enabled) {
      int err =0;
      try {err= RUN_ALL_TESTS(); }//NOTE macro runs regardless of being enabled
      catch (...) {
        throw std::runtime_error ("error in GoogleTest RUN_ALL_TESTS()");
      }
      if (err) {
        auto msg = std::string("Warn""ing: GoogleTest returned ")
          + std::to_string(err) + std::string (".");
        throw std::runtime_error (msg.c_str());
  } } }
}
