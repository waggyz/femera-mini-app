#include "Gtst.hpp"
#include "../core.h"

#ifdef FMR_HAS_GTEST
#include <gtest/gtest.h>
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#if 1
namespace femera { namespace test { namespace gtst {

  class MinimalistPrinter : public ::testing::EmptyTestEventListener {
    /* from: https://github.com/google/googletest/blob/main/docs/advanced.md
     */
#if 0
  // Called before any test activity starts.
  void OnTestProgramStart(const UnitTest& /* unit_test */) override {}
#endif

  // Called after all test activities have ended.
  void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
    const auto test_n = unit_test.total_test_count ();
    const auto pass_n = unit_test.successful_test_count ();
    const auto skip_n = unit_test.skipped_test_count ();
    const auto  bad_n = unit_test.failed_test_count ();
    const auto ms     = double (unit_test.elapsed_time ());
    if (pass_n > 0) {
    fprintf (stdout, "%4s %4s %4s %4i ok /%4i test%s in %g ms\n",
      "test", "gtst", "pass", pass_n, test_n, (test_n == 1) ? "" : "s", ms);
    }
    if (skip_n > 0) {
    fprintf (stdout, "%4s %4s %4s %4i    /%4i test%s in %g ms\n",
      "test", "gtst", "skip", skip_n, test_n, (test_n == 1) ? "" : "s", ms);
    }
    if ( bad_n) {
    fprintf (stdout, "%4s %4s %4s %4i    /%4i test%s in %g ms\n",
      "test", "gtst", "FA""IL", bad_n, test_n, (test_n == 1) ? "" : "s", ms);
  } }
#if 0
    // Called before a test starts.
    void OnTestStart(const ::testing::TestInfo& test_info) override {
      fprintf (stdout, "test gtst  run %s.%s starting.\n",
        test_info.test_suite_name(), test_info.name());
    }
#endif
    // Called after a failed assertion or a SUCCESS().
    void OnTestPartResult(const ::testing::TestPartResult& test_part_result)
    override {//NOTE not called for successful tests
      fprintf (stdout, "%4s %4s %4s in %s:%d\n%s\n", "test", "gtst",
        test_part_result.failed() ? "FA""IL" : "ok",
        test_part_result.file_name(),
        test_part_result.line_number(),
        test_part_result.summary());
    }
#if 0
    // Called after a test ends.
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
      fprintf (stdout, "test gtst  run %s.%s ending.\n",
        test_info.test_suite_name(), test_info.name());
    }
#endif
  };

TEST( MiniGtest, RunsGtests ){
  EXPECT_EQ( 1, 1 );
#if 0
  // The following generates a SUCCESS() for listeners' OnTestPartResult (..).
  SUCCEED();
#endif
}

} } }//end femera::test::gtst:: namespace
#endif

namespace femera {
  void test::Gtst::scan (int* argc, char** argv) {
    if (argc != nullptr && argv != nullptr) {
      FMR_PRAGMA_OMP(omp MAIN) {//NOTE getopt is NOT thread safe.
        int ar=argc[0];// Copy getopt variables.
        auto oe=opterr; auto oo=optopt; auto oi=optind; auto oa=optarg;
        opterr = 0; int optchar;
        while ((optchar = getopt (argc[0], argv, "T")) != -1) {
          // T  -T has no argument//NOTE -g is eaten by MPI
          switch (optchar) {
            case 'T':{ this->do_enable (argc, argv); break; }
            //this->proc->opt_add ('T'); break; }
        } }
        // Restore getopt variables.
        argc[0]=ar; opterr=oe; optopt=oo; optind=oi; optarg=oa;
  } } }
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
    this->scan (argc, argv);
#ifdef FMR_HAS_MPI
    /*from: https://github.com/google/googletest/issues/822
     * and: https://github.com/google/googletest/blob/main/docs/advanced.md
     */
    ::testing::TestEventListeners& listeners
      = ::testing::UnitTest::GetInstance()->listeners();
    // Remove all default listeners and replace with custom listeners.
    delete listeners.Release (listeners.default_result_printer());
    if ( !this->is_enabled || !this->proc->is_main()) {
      // Print from main thread only,... and log all threads to files.
      listeners.Append(new test::gtst::MinimalistPrinter);
    }
    //TODO Add another custom listener to log all threads to files
//    listeners.Append(new test::gtst::MinimalistPrinter);
#endif
  this->set_init (true);
  }
  void test::Gtst::task_exit () {
    if (!this->is_enabled || !this->do_all_tests_on_exit) {
      return;
    }
    if (this->did_run_all_tests) {
      FMR_THROW("already called GoogleTest RUN_ALL_TESTS()");
      return;
    }
#if 0
    const auto run_suite_n
      = ::testing::UnitTest::GetInstance()->test_suite_to_run_count();
    if (run_suite_n <= 0) {//TODO doesn't always get count?
      return;
    }
#endif
    int err =0;
#ifdef FMR_DEBUG
    printf ("running %i GoogleTest test suite%s...\n",
      run_suite_n, (run_suite_n==1) ? "" : "s" );
#endif
    try {err= RUN_ALL_TESTS(); }//NOTE macro runs regardless of being enabled
    catch (...) {//TODO Check if the macro is noexcept.
      FMR_THROW("err""or thrown during GoogleTest RUN_ALL_TESTS()");
      return;
    }
    if (err) {
      const auto msg = std::string("warn""ing: GoogleTest returned ")
        + std::to_string (err) +".";
      FMR_THROW(msg);
      return;
    }
    this->did_run_all_tests = true;
  }
}

#undef FMR_DEBUG
