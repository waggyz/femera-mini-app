#include "heat.hpp"

#include "../perf/Meter.hpp"

#include <gtest/gtest.h>

#ifdef FMR_HAS_OPENMP
#include <omp.h>
#endif

#include <immintrin.h>

TEST(PerfMtrlElasticLinear, TrivialTest) {
  EXPECT_EQ(1,1);
}

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

