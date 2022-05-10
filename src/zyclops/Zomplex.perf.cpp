#include "Zomplex.hpp"

#include "../fmr/perf/Meter.hpp"

#include "gtest/gtest.h"

#include <vector>
#include <cstdio>

#undef ZYC_DO_MOST_NAIVE

namespace zyc { namespace test {
  //
  inline
  double dual_fma// reutrns a check value (0.0)
  (const uint test_n=10, const uint vals_n=1024, const int order=-1) {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    if (test_n<=0 || vals_n<=0) {return 0.0;}
    const auto zsz = int(1) << order;
    const auto avec = std::vector<double> (vals_n, 1.0);
    const auto bvec = std::vector<double> (vals_n);
    auto cvec = std::vector<double> (vals_n);
    auto crvec = std::vector<double> (uint(zsz * zsz));// buffer for naive algo.
    const auto a = avec.data ();
    const auto b = bvec.data ();
    auto c = cvec.data ();
    auto cr = crvec.data ();
    double chk = 0.0;
    const auto byte = double (test_n * (
      + avec.size () * sizeof (avec[0])
      + bvec.size () * sizeof (bvec[0])
      + cvec.size () * sizeof (cvec[0]) * 2));
    double secs_ref = 0.0, secs_fma = 0.0, secs_tfma = 0.0, secs_tnaiv = 0.0;
#ifdef ZYC_DO_MOST_NAIVE
     double secs_naiv = 0.0;
#endif
    const auto flop_ref = 2.0 * double (test_n * vals_n);
    const auto flop_dual = 2.0 * double (test_n * vals_n / uint(zsz))
      * double(std::pow (3,order));
    for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];}// warm up
    for (uint test_i=0; test_i<test_n; test_i++) {
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {
        c[i] += a[i] * b[i];
      }
      secs_ref += double (time.add_busy_time_now ());
      for (uint i=0; i<vals_n; i++) {chk += c[i];}
      if (order < 0) { return chk; }
      const auto n = int (vals_n);
      time.add_idle_time_now ();
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix (i,j);
            ck[j] += (ix<0) ? 0.0 : ak [ix] * bk [j];
      } } }
      secs_fma += double (time.add_busy_time_now ());
      for (uint i=0; i<vals_n; i++) {chk += c[i];}
      time.add_idle_time_now ();
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix (j,i);
            ck[i] += (ix<0) ? 0.0 : bk [i] * ak [ix];
      } } }
      secs_tfma += double (time.add_busy_time_now ());
      for (uint i=0; i<vals_n; i++) {chk += c[i];}
#ifdef ZYC_DO_MOST_NAIVE
      time.add_idle_time_now ();
      for (int k=0; k<n; k+=zsz) {// transposed version below is faster
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {// build CR form of ak
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix (i,j);
#if 0
            if (ix>=0) { cr[zsz*i + j]= ak [ix]; }// set only nonzeros
#else
            cr[zsz* i + j] = (ix<0) ? 0.0 : ak [ix];// can be vectorized
#endif
        } }
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            ck[i] += cr [zsz* i + j] * bk [j];// regular matmul
      } } }
      secs_naiv += double (time.add_busy_time_now ());
      for (uint i=0; i<vals_n; i++) {chk += c[i];}
#endif
      time.add_idle_time_now ();
      for (int k=0; k<n; k+=zsz) {
        const auto ak = &a [k];
        const auto bk = &b [k];
        auto ck = &c [k];
        for (int i=0; i<zsz; i++) {// build transposed CR form of ak (faster)
          for (int j=0; j<zsz; j++) {
            const auto ix = zyc::dual_ix (j,i);
            cr[zsz* i + j] = (ix<0) ? 0.0 : ak [ix];
        } }
        for (int i=0; i<zsz; i++) {
          for (int j=0; j<zsz; j++) {
            ck[i] += bk [i] * cr [zsz* i + j];// transposed matmul
      } } }
      secs_tnaiv += double (time.add_busy_time_now ());
      for (uint i=0; i<vals_n; i++) {chk += c[i];}
    }//end test_n loop
    printf (" zyc, ref, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      -1, test_n * vals_n, byte, flop_ref, secs_ref, flop_ref/secs_ref,
      flop_ref/byte, chk);
    printf (" zyc,dual, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop_dual, secs_fma,
        flop_dual/secs_fma, flop_dual/byte, chk);
    printf (" zyc,dual,tfma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop_dual, secs_tfma,
        flop_dual/secs_tfma, flop_dual/byte, chk);
#ifdef ZYC_DO_MOST_NAIVE
    printf (" zyc,dual,naiv,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop_dual, secs_naiv,
        flop_dual/secs_naiv, flop_dual/byte, chk);
#endif
    printf (" zyc,dual,tnai,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte, flop_dual, secs_tnaiv,
        flop_dual/secs_tnaiv, flop_dual/byte, chk);
    return chk;
  }
  const auto zn = 2*1024*1024;
  TEST(Zomplex, DualMultiply) {
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 0), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 1), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 2), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 3), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 4), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 5), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 6), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 7), 0.0);
    EXPECT_DOUBLE_EQ( dual_fma (10, zn, 8), 0.0);
  }
} }//end femerea::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
