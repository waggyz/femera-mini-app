#include "Meter.hpp"

#include "gtest/gtest.h"
#include <unistd.h> // usleep
#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

// Make something to test.
fmr::perf::Meter perf = fmr::perf::Meter();

TEST(FmrPerfMeter, Startup) {
  const auto first_idle
    = fmr::perf::Float (perf.add_idle_time_now ()) * fmr::perf::Float (1e-9);
  EXPECT_FLOAT_EQ(perf.get_idle_time (), first_idle);
  EXPECT_FLOAT_EQ(perf.get_busy_time (), 0);
  EXPECT_GT(perf.get_idle_time (), 0);
  EXPECT_GT(perf.get_work_time (), 0);
  EXPECT_FLOAT_EQ(perf.get_idle_time (), perf.get_work_time ());
  perf.add_idle_time_now ();
  EXPECT_GT(perf.get_idle_time (), first_idle);
}
TEST(FmrPerfMeter, Naptime) {
  perf.unit_name = "naps";
  perf.add_idle_time_now ();
  ::usleep (1000);// 1 ms
  perf.add_busy_time_now ();
  EXPECT_GT(perf.get_busy_time (),       0.99 * 1e-3);
  EXPECT_GT(perf.get_unit_speed (),      0.50 * 1e+3);
  EXPECT_GT(perf.get_busy_unit_speed (), 0.80 * 1e+3);
  EXPECT_GT(perf.get_busy_unit_speed (), perf.get_unit_speed ());
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
