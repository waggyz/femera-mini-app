#include "perf.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

TEST(FmrPerfMeter, PerfIntSizes) {
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Elapsed));
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Count));
}
int main (int argc, char** argv) {
#ifdef FMR_HAS_MPI
  int err=0;
  ::MPI_Init (&argc,&argv);
  ::testing::InitGoogleTest (&argc,argv);
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
  ::testing::InitGoogleTest (&argc,argv);
  return RUN_ALL_TESTS();
#endif
}
