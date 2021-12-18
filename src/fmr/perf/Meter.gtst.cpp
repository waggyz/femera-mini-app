#include "Meter.hpp"

#include "gtest/gtest.h"
#include <unistd.h> // usleep
#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

// Make something to test.
fmr::perf::Meter perf = fmr::perf::Meter();

TEST(FmrPerfMeter, Startup) {
  const auto first_idle = perf.add_idle_time_now ();
  EXPECT_FLOAT_EQ(perf.get_idle_s (), first_idle);
  EXPECT_FLOAT_EQ(perf.get_busy_s (), 0);
  EXPECT_GT(perf.get_idle_s (), 0);
  EXPECT_GT(perf.get_work_s (), 0);
  EXPECT_FLOAT_EQ(perf.get_idle_s (), perf.get_work_s ());
  perf.add_idle_time_now ();
  EXPECT_GT(perf.get_idle_s (), first_idle);
}
TEST(FmrPerfMeter, Naptime) {
  perf.set_unit ("naps");
  perf.add_count ();// default is add 1 unit ("naps")
  perf.add_idle_time_now ();// done preparing; get busy sleeping now
  ::usleep (1000);// 1 ms nap
  perf.add_busy_time_now ();// done being busy sleeping
  EXPECT_GT(perf.get_busy_s (),          0.99 * 1e-3);
  EXPECT_GT(perf.get_unit_speed (),      0.50 * 1e+3);
  EXPECT_GT(perf.get_busy_unit_speed (), 0.80 * 1e+3);
  EXPECT_GT(perf.get_busy_unit_speed (), perf.get_unit_speed ());
}
TEST(FmrPerfMeter, ArithmeticIntensity) {
  perf.add_count (1000, 500, 500, 0);
  EXPECT_FLOAT_EQ(perf.get_ai (), 1.0);
  EXPECT_FLOAT_EQ(perf.get_ai (), perf.get_arithmetic_intensity ());
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
