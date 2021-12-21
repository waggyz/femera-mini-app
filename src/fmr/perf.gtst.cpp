#include "perf.hpp"

#include <limits>

#include "../femera/main-early.gtst.ipp"

TEST(FmrPerf, PerfIntSizes) {
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Elapsed));
  EXPECT_EQ(sizeof (fmr::perf::Timepoint), sizeof (fmr::perf::Count));
}
TEST(FmrPerf, MaxPerfCountGT10G) {
  EXPECT_GT(std::numeric_limits<fmr::perf::Count>::max(), 10000000000);
}
