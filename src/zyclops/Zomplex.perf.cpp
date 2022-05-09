#include "Zomplex.hpp"

#include "../fmr/perf/Meter.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <cstdio>

namespace zyc { namespace test {
  //
  inline
  double dual_fma// reutrns flop/sec
  (const uint test_n=10, const uint vals_n=1024, const int order=-1) {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    if (test_n<=0 || vals_n<=0) {return 0.0;}
    const auto avec = std::vector<double> (vals_n, 1.0);
    const auto bvec = std::vector<double> (vals_n);
    auto cvec = std::vector<double> (vals_n);
    const auto a = avec.data ();
    const auto b = bvec.data ();
    auto c = cvec.data ();
    double chk = 0.0;
    const auto byte = double (test_n * (
      + avec.size () * sizeof (avec[0])
      + bvec.size () * sizeof (bvec[0])
      + cvec.size () * sizeof (cvec[0]) * 2));
    for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];}// warm up
    time.add_idle_time_now ();
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (uint i=0; i<vals_n; i++) {
        c[i] += a[i] * b[i];
    } }
    auto secs = double (time.add_busy_time_now ());
    for (uint i=0; i<vals_n; i++) {chk += c[i];}
    auto flop = 2.0 * double (test_n * vals_n);
    printf (" zyc, ref, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      -1, test_n * vals_n, byte, flop, secs, flop/secs, flop/byte, chk);
    if (order < 0) { return chk; }
    const auto zsz = int(1) << order;
    const auto n = int (vals_n);
    time.add_idle_time_now ();
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix(i,j);
            ck[j] += (ix<0) ? 0.0 : ak [ix] * bk [j];
    } } } }
    secs = double (time.add_busy_time_now ());
    for (uint i=0; i<vals_n; i++) {chk += c[i];}
    flop = 2.0 * double (test_n * vals_n / uint(zsz))
      * double(std::pow (3,order));
    printf (" zyc,dual, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop, secs, flop/secs,
      flop/byte, chk);
    time.add_idle_time_now ();
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix(j,i);
            ck[i] += (ix<0) ? 0.0 : ak [ix] * bk [i];
    } } } }
    secs = double (time.add_busy_time_now ());
    for (uint i=0; i<vals_n; i++) {chk += c[i];}
    printf (" zyc,dual,tfma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop, secs, flop/secs,
      flop/byte, chk);
    auto crvec = std::vector<double> (uint(zsz * zsz));
    auto cr = crvec.data ();
    time.add_idle_time_now ();
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {// build CR form of ak
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix(i,j);
#if 0
            if (ix>=0) { t[zsz*i + j]= ak [ix]; }
#else
            cr[zsz* i + j] = (ix<0) ? 0.0 : ak [ix];
#endif
        } }
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            ck[i] += cr [zsz* i + j] * bk [j];// regular matmul
    } } } }
    secs = double (time.add_busy_time_now ());
    for (uint i=0; i<vals_n; i++) {chk += c[i];}
    printf (" zyc,dual,naiv,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop, secs, flop/secs,
      flop/byte, chk);
    return chk;
  }
  TEST(Zomplex, DualMultiply) {
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 0), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 1), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 2), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 3), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 4), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 5), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 6), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 7), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, 16*1024*1024, 8), 0.0);
  }
} }//end femerea::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
