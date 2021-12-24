#include "perf.hpp"

#include "gtest/gtest.h"

#include <limits>


TEST(FmrPerf, PerfIntSizes) {
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Elapsed));
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Count));
}
TEST(FmrPerf, MaxPerfCountGT10G) {
  EXPECT_GT(std::numeric_limits<fmr::perf::Count>::max(), size_t(10000000000));
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
