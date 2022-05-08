#include "Zomplex.hpp"

#include "../fmr/perf/Meter.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <cstdio>

namespace zyc { namespace test {
  //
  inline
  double dual_fma
  (const int test_n=10, const uint vals_n=1024, const uint order=0) {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    if (test_n<=0 || vals_n<=0) {return 0;}
    const auto ea = uint(1) << order;
    const auto a = std::vector<double> (vals_n * ea);
    const auto b = std::vector<double> (vals_n * ea, 1.0);
    double sum = 0.0;
    for (uint i=0; i<vals_n; i++) {sum += a[i] * b[i];}// warm up
    time.add_idle_time_now ();
    for (int test_i=0; test_i<test_n; test_i++) {
      for (uint i=0; i<vals_n; i++) {
        sum += a[i] * b[i];
    } }
    const auto t = double (time.add_busy_time_now ());
    return double (2.0 * test_n * vals_n * ea) / t + sum;
  }
  TEST(NewVals, Time) {
    EXPECT_GT( dual_fma (10, 16*1024*1024, 0), 0.0);
  }
} }//end femerea::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
