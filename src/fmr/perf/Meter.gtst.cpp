#include "Meter.hpp"

#include <unistd.h> // usleep

#include "../../femera/main-early.gtst.ipp"

// Make something to test.
fmr::perf::Meter perf = fmr::perf::Meter();

TEST(FmrPerfMeter, StructSize) {
  EXPECT_EQ(sizeof (fmr::perf::Meter), 64);
}
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
  perf.set_unit_name ("naps");
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