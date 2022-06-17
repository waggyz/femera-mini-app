#include "zyc.hpp"

#include "../fmr/perf/Meter.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <cstdio>

#if 0
#define TEST_ZYC_ARRAY_PTR auto
#define TEST_ZYC_CONST_PTR const auto
#else
#define TEST_ZYC_ARRAY_PTR auto* __restrict
#define TEST_ZYC_CONST_PTR const auto* __restrict
#endif

namespace zyclops { namespace test {
  namespace zyc = ::zyclops;
  //
  inline
  double dual_mult// returns a check value: 0.0
  (const uint test_n=10, const uint vals_n=1024, zyc::Zorder_int order=0) {
    if (test_n<=0 || vals_n<=0) {return 0.0;}
    double chk = 0.0;
    ZYC_PRAGMA_OMP(omp parallel num_threads(2) reduction(+:chk))
    {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    const uint zsz = uint (1) << order;
    const auto avec = std::vector<double> (vals_n, 1.0);
    const auto bvec = std::vector<double> (vals_n);
    auto cvec = std::vector<double> (vals_n);
    TEST_ZYC_CONST_PTR a = avec.data ();
    TEST_ZYC_CONST_PTR b = bvec.data ();
    TEST_ZYC_ARRAY_PTR c = cvec.data ();
    const auto byte_fma = double (test_n * (
      + avec.size () * sizeof (avec[0])       // 1 read
      + bvec.size () * sizeof (bvec[0])       // 1 read
      + cvec.size () * sizeof (cvec[0]) * 2));// 1 read + 1 write
    double secs_ref = 0.0, secs_aos = 0.0, secs_soa = 0.0;
#if 0
    auto s2vec = std::vector<double> (uint(zsz));// sum of squares
    TEST_ZYC_ARRAY_PTR s2 = s2vec.data ();
    const auto byte_s2 = double (test_n * (cvec.size () * sizeof (cvec[0]))
      + 2 * s2vec.size () * sizeof (s2vec[0]) );
    double secs_ref2 = 0.0, secs_aos2 = 0.0, secs_soa2 = 0.0;
#endif
    const auto flop_ref  = 2.0 * double (test_n * vals_n);
    const auto flop_dual = 2.0 * double (test_n * vals_n / uint (zsz))
      * double(std::pow (3,order));
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i]; }
      }
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {
        c[i] += a[i] * b[i];
      }
      secs_ref += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i]; }
      }
      const auto dual_n = vals_n / zsz;
      ZYC_PRAGMA_OMP(omp barrier)
    //-----------------------------------------------------------------------
      time.add_idle_time_now ();
      mdas_madd (cvec.data()[0], avec.data()[0], bvec.data()[0],
        order, dual_n);
      secs_aos += double (time.add_busy_time_now ());
      for (uint ti=0; ti<test_n; ti++) {// do not optimize calc away
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i] * c[0]; }
      }
      ZYC_PRAGMA_OMP(omp barrier);
      time.add_idle_time_now ();
      mdsa_madd (cvec.data()[0], avec.data()[0], bvec.data()[0],
        order, dual_n);
      secs_soa += double (time.add_busy_time_now ());
      for (uint ti=0; ti<test_n; ti++) {// do not optimize calc away
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i] * c[0]; }
      }
#if 0
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) { chk += c[i] * c[i]; }
      }
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {// sum of squares
        chk += c[i] * c[i];
      }
      secs_ref2 += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ()
      aos_dual_rsum_mult (s2vec.data()[0], cvec.data()[0], cvec.data()[0],
        order, dual_n);
      secs_aos2 += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ()
      soa_dual_rsum_mult (s2vec.data()[0], cvec.data()[0], cvec.data()[0],
        order, dual_n);
      secs_soa2 += double (time.add_busy_time_now ());
#endif
    }//end test_n loop
    printf (" zyc, ref, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      -1, test_n * vals_n, byte_fma, flop_ref, secs_ref, flop_ref/secs_ref,
      flop_ref/byte_fma, chk);
    printf (" zyc,mdas,madd,%2lu,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_aos,
        flop_dual/secs_aos, flop_dual/byte_fma, chk);
    printf (" zyc,mdsa,madd,%2lu,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_soa,
        flop_dual/secs_soa, flop_dual/byte_fma, chk);
#if 0
    printf (" zyc, ref,sum2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      -1, test_n * vals_n, byte_s2, flop_ref, secs_ref2, flop_ref/secs_ref2,
      flop_ref/byte_s2, chk);
    printf (" zyc,dual,tnai,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_aos,
        flop_dual/secs_aos, flop_dual/byte_fma, chk);
    //printf (" zyc,dual,tnai,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
    //  order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_aos,
    //    flop_dual/secs_aos, flop_dual/byte_fma, chk);
#endif
    ZYC_PRAGMA_OMP(omp barrier);
    }//end parallel region
    return chk;
  }//end dual_mult (..)
  inline
  double dual_div// returns a check value: 0.0, also does addition
  (const uint test_n=10, const uint vals_n=1024, zyc::Zorder_int order=0) {
    if (test_n<=0 || vals_n<=0) {return 0.0;}
    double chk = 0.0;
    ZYC_PRAGMA_OMP(omp parallel num_threads(2) reduction(+:chk))
    {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    const uint zsz = uint (1) << order;
    const auto avec = std::vector<double> (vals_n);
    const auto bvec = std::vector<double> (vals_n, 1.0);
    auto cvec = std::vector<double> (vals_n);
    TEST_ZYC_CONST_PTR a = avec.data ();
    TEST_ZYC_CONST_PTR b = bvec.data ();
    TEST_ZYC_ARRAY_PTR c = cvec.data ();
    const auto byte_fma = double (test_n * (
      + avec.size () * sizeof (avec[0])       // 1 read
      + bvec.size () * sizeof (bvec[0])       // 1 read
      + cvec.size () * sizeof (cvec[0]) * 2));// 1 read + 1 write
    const auto byte_add = double (test_n * (
      + avec.size () * sizeof (avec[0])       // 1 read
      + bvec.size () * sizeof (bvec[0])       // 1 read
      + cvec.size () * sizeof (cvec[0])));// 1 write
    double secs_ref = 0.0, secs_aos = 0.0, secs_soa = 0.0, secs_add = 0.0;
#if 0
    auto s2vec = std::vector<double> (uint(zsz));// sum of squares
    TEST_ZYC_ARRAY_PTR s2 = s2vec.data ();
    const auto byte_s2 = double (test_n * (cvec.size () * sizeof (cvec[0]))
      + 2 * s2vec.size () * sizeof (s2vec[0]) );
    double secs_ref2 = 0.0, secs_aos2 = 0.0, secs_soa2 = 0.0;
#endif
    const auto flop_add  = double (test_n * vals_n);
    const auto flop_ref  = 2.0 * double (test_n * vals_n);
    const auto flop_dual = 2.0 * double (test_n * vals_n / uint (zsz))
      * double(std::pow (3,order));//TODO check this
    for (uint test_i=0; test_i<test_n; test_i++) {
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i]; }
      }
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {
        c[i] += a[i] * b[i];
      }
      secs_ref += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i]; }
      }
      const auto dual_n = vals_n / zsz;
      ZYC_PRAGMA_OMP(omp barrier)
    //-----------------------------------------------------------------------
      time.add_idle_time_now ();
      mdas_div (cvec.data()[0], avec.data()[0], bvec.data()[0],
        order, dual_n);
      secs_aos += double (time.add_busy_time_now ());
      for (uint ti=0; ti<test_n; ti++) {// do not optimize calc away
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i] * c[0]; }
      }
      ZYC_PRAGMA_OMP(omp barrier);
      time.add_idle_time_now ();
      mdsa_div (cvec.data()[0], avec.data()[0], bvec.data()[0],
        order, dual_n);
      secs_soa += double (time.add_busy_time_now ());
      for (uint ti=0; ti<test_n; ti++) {// do not optimize calc away
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i] * c[0]; }
      }
      ZYC_PRAGMA_OMP(omp barrier);
      time.add_idle_time_now ();
      hca_add (cvec.data()[0], avec.data()[0], bvec.data()[0],
        order, dual_n);
      secs_add += double (time.add_busy_time_now ());
      for (uint ti=0; ti<test_n; ti++) {// do not optimize calc away
        for (uint i=0; i<vals_n; i++) { chk += a[i] * b[i] * c[0]; }
      }
    }//end test_n loop
    printf (" zyc, ref, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      -1, test_n * vals_n, byte_fma, flop_ref, secs_ref, flop_ref/secs_ref,
      flop_ref/byte_fma, chk);
    printf (" zyc,mdas, div,%2lu,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_aos,
        flop_dual/secs_aos, flop_dual/byte_fma, chk);
    printf (" zyc,mdsa, div,%2lu,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_soa,
        flop_dual/secs_soa, flop_dual/byte_fma, chk);
    printf (" zyc,  hc, add,%2lu,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
      order, (test_n * vals_n) / uint(zsz), byte_add, flop_add, secs_add,
        flop_add/secs_add, flop_add/byte_add, chk);
    ZYC_PRAGMA_OMP(omp barrier);
    }//end parallel region
    return chk;
  }//end dual_div (..)
  const auto test_n  = 5;
  const auto test_sz = 8*1024*1024;
  TEST(Zyc, MultidualMultiply) {
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 0), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 1), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 2), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 3), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 4), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 5), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 6), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 7), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 8), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz, 9), 0.0);
    EXPECT_DOUBLE_EQ( dual_mult (test_n, test_sz,10), 0.0);
  }
  TEST(Zyc, MultidualDivide) {
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 0), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 1), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 2), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 3), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 4), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 5), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 6), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 7), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 8), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz, 9), 0.0);
    EXPECT_DOUBLE_EQ( dual_div (test_n, test_sz,10), 0.0);
  }
} }//end zyclops::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
