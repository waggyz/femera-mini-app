#include "Test.hpp"

#ifdef FMR_HAS_GTEST
#include <gtest/gtest.h>
#endif
#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

int femera::test:: early_main (int* argc, char** argv) {
  // This was for testing before femera::proc/test/Gtst.?pp had been coded.
#ifdef FMR_HAS_GTEST
#ifdef FMR_HAS_MPI
  int err=0;
  int did_mpi_init=0;
  err = MPI_Initialized (& did_mpi_init);
  if (!did_mpi_init) {err= ::MPI_Init (argc,&argv);}
  ::testing::InitGoogleTest (argc,argv);
  int proc_id=0; ::MPI_Comm_rank (MPI_COMM_WORLD,& proc_id);
  //
  //from: https://github.com/google/googletest/issues/822
  ::testing::TestEventListeners& listeners
    = ::testing::UnitTest::GetInstance ()->listeners ();
  if (proc_id != 0) {// Only print from main thread; release the others.
    delete listeners.Release (listeners.default_result_printer ());
  }
  err = RUN_ALL_TESTS();
  ::MPI_Initialized (& did_mpi_init);
  if (did_mpi_init) {::MPI_Finalize ();}
  return err;
#else
  ::testing::InitGoogleTest (argc,argv);
  return RUN_ALL_TESTS();
#endif
#else
  return 1;// GoogleTest not enabled
#endif
}
