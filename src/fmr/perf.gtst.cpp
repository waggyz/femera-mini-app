#include "perf.hpp"

#include "gtest/gtest.h"

#include <limits>


TEST(FmrPerf, PerfIntSizes) {
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Elapsed));
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Count));
}
TEST(FmrPerf, MaxPerfCountGT10G) {
  EXPECT_GT(std::numeric_limits<fmr::perf::Count>::max(), 10000000000);
}
int main (int argc, char** argv) {
  return fmr::detail::test:: early_main (&argc, argv);
}
