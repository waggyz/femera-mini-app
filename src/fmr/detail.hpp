#ifndef FMR_HAS_FMR_DETAIL_HPP
#define FMR_HAS_FMR_DETAIL_HPP

#include "../femera/femera.hpp"

#include <cstdint>
#include <string>


//FIXME Move fmr::detail::* to femera::* or femera::detail::*

namespace fmr { namespace detail {
// This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
// https://google.github.io/styleguide/cppguide.html#Aliases

using Perf_float = float;
using Perf_int   = uint_fast64_t;

} }//end fmr::detail:: namespace

namespace fmr { namespace detail { namespace form {

std::string si_unit_string (const double val, std::string unit,
  const int min_digits, const std::string signchar);

} } }// end fmr::detail::form:: namespace

namespace fmr { namespace detail { namespace test {
  int early_main (int* argc, char** argv);
} } }// end fmr::detail::test:: namespace

#if 0
namespace fmr { namespace detail { namespace test {
  static int early_main (int* argc, char** argv);
} } }// end fmr::detail::test:: namespace
#endif



#undef FMR_DEBUG
//end FMR_HAS_FMR_DETAIL_HPP
#endif

#if 0
#ifdef FMR_HAS_GTEST
#include "gtest/gtest.h"
#endif
#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif
static int fmr::detail::test:: early_main (int* argc, char** argv) {
  // Use this for testing before femera::proc/test/Gtst.?pp has been coded.
#ifdef FMR_HAS_GTEST
#ifdef FMR_HAS_MPI
  int err=0;
  ::MPI_Init (argc,&argv);
  ::testing::InitGoogleTest (argc,argv);
  int proc_id=0; ::MPI_Comm_rank (MPI_COMM_WORLD,& proc_id);
  //
  //from: https://github.com/google/googletest/issues/822
  ::testing::TestEventListeners& listeners
    = ::testing::UnitTest::GetInstance ()->listeners ();
  if (proc_id != 0) {// Only print from master; release the others.
    delete listeners.Release (listeners.default_result_printer ());
  }
  err = RUN_ALL_TESTS();
  ::MPI_Finalize ();
  return err;
#else
  ::testing::InitGoogleTest (argc,argv);
  return RUN_ALL_TESTS();
#endif
#else
  return 1;// GoogleTest not enabled
#endif
}
#endif
