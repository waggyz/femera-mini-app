#include "zyc.hpp"

#include "../fmr/perf/Meter.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <cstdio>

#undef ZYC_TEST_MOST_NAIVE

#if 0
#define TEST_ZYC_ARRAY_PTR auto
#define TEST_ZYC_CONST_PTR const auto
#else
#define TEST_ZYC_ARRAY_PTR auto* __restrict
#define TEST_ZYC_CONST_PTR const auto* __restrict
#endif

namespace zyc { namespace test {
  //
  inline
  double dual_aos_fma// interleaved storage (AoS), returns a check value (0.0)
  (const uint test_n=10, const uint vals_n=1024, const int order=-1) {
    if (test_n<=0 || vals_n<=0) {return 0.0;}
    double chk = 0.0;
    ZYC_PRAGMA_OMP(omp parallel num_threads(2))
    {
    auto time = fmr::perf::Meter<uint64_t,float> ();
    const uint zsz = uint (1) << order;
    const auto avec = std::vector<double> (vals_n, 1.0);
    const auto bvec = std::vector<double> (vals_n);
    auto cvec = std::vector<double> (vals_n);
    auto crvec = std::vector<double> (uint(zsz * zsz));// buffer for naive algo.
    auto s2vec = std::vector<double> (uint(zsz));// sum of squares
    TEST_ZYC_CONST_PTR a = avec.data ();
    TEST_ZYC_CONST_PTR b = bvec.data ();
    TEST_ZYC_ARRAY_PTR c = cvec.data ();
    TEST_ZYC_ARRAY_PTR cr = crvec.data ();
    TEST_ZYC_ARRAY_PTR s2 = crvec.data ();
    const auto byte_fma = double (test_n * (
      + avec.size () * sizeof (avec[0])
      + bvec.size () * sizeof (bvec[0])
      + cvec.size () * sizeof (cvec[0]) * 2));
    const auto byte_s2 = double (test_n * (cvec.size () * sizeof (cvec[0])));
    double secs_ref  = 0.0, secs_fma = 0.0, secs_tfma = 0.0, secs_tnaiv = 0.0;
    double secs_ref2 = 0.0, secs_s2 = 0.0, secs_ts2 = 0.0, secs_tnai2 = 0.0;
#ifdef ZYC_TEST_MOST_NAIVE
    double secs_naiv = 0.0, secs_nai2 = 0.0;
#endif
    const auto flop_ref = 2.0 * double (test_n * vals_n);
    const auto flop_dual = 2.0 * double (test_n * vals_n / uint (zsz))
      * double(std::pow (3,order));
    for (uint test_i=0; test_i<test_n; test_i++) {
      ZYC_PRAGMA_OMP(omp barrier)
      for (uint ti=0; ti<test_n; ti++) {// warm up
        for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];
      } }
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {
        c[i] += a[i] * b[i];
      }
      secs_ref += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ();
      for (uint i=0; i<vals_n; i++) {
        chk += c[i] * c[i];
      }
      secs_ref2 += double (time.add_busy_time_now ());
      ZYC_PRAGMA_OMP(omp barrier)
      time.add_idle_time_now ();
      if (order >= 0) {
        const auto n = uint (vals_n);
        for (uint ti=0; ti<test_n; ti++) {// warm up
          for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];
        } }
        time.add_idle_time_now ();
  //      const double zero =0.0;
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_CONST_PTR ak = &a [k];
          TEST_ZYC_CONST_PTR bk = &b [k];
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
#if 0
              ck [j] += (((i^j)==(i-j)) ? ak [i-j] : zero) * bk [j];
#endif
#if 0
              const auto ix = zyc::dual_ix (i,j);
              ck[j] += (ix<0) ? 0.0 : ak [ix] * bk [j];
#endif
#if 0
              ck [j] += (zyc::is_dual_nz (i,j) ? ak [zyc::dual_ux (i,j)] : 0.0)
                * bk [j];
#endif
#if 1
              ck [j] += zyc::cr_dual_elem (ak, i, j) * bk [j];
#endif
#if 0
              ck [j] += (((i^j)==(i-j)) ? ak [i-j] : 0.0) * bk [j];
#endif
        } } }
        secs_fma += double (time.add_busy_time_now ());
        ZYC_PRAGMA_OMP(omp barrier)
        for (uint ti=0; ti<test_n; ti++) {// warm up
          for (uint i=0; i<vals_n; i++) {
            chk += a[i] * b[i];
        } }
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              s2 [j] += zyc::cr_dual_elem (ck, i, j) * ck [j];
        } } }
        secs_s2 += double (time.add_busy_time_now ());
        for (uint i=0; i<zsz; i++) {chk += s2[i];}
        ZYC_PRAGMA_OMP(omp barrier)
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_CONST_PTR ak = &a [k];
          TEST_ZYC_CONST_PTR bk = &b [k];
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
#if 0
              const auto ix = zyc::dual_ix (j,i);
              ck[i] += (ix<0) ? 0.0 : bk [i] * ak [ix];
#endif
#if 0
              ck [j] += bk [j]
                * (zyc::is_dual_nz (j,i) ? ak [zyc::dual_ux (j,i)] : 0.0);
#endif
#if 1
              ck [j] +=  bk [j] * zyc::cr_dual_elem (ak, j, i);
#endif
        } } }
        secs_tfma += double (time.add_busy_time_now ());
        ZYC_PRAGMA_OMP(omp barrier)
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              s2 [j] += ck [j] * zyc::cr_dual_elem (ck, j, i);
        } } }
        secs_ts2 += double (time.add_busy_time_now ());
        for (uint i=0; i<zsz; i++) {chk += s2[i];}
        ZYC_PRAGMA_OMP(omp barrier)
#ifdef ZYC_TEST_MOST_NAIVE
        for (uint ti=0; ti<test_n; ti++) {// warm up
          for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];
        } }
        time.add_idle_time_now ();
        for (int k=0; k<n; k+=zsz) {
          TEST_ZYC_CONST_PTR ak = &a [k];
          TEST_ZYC_CONST_PTR bk = &b [k];
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {// build CR form of ak
            for (zyc::Zsize_t j=0; j<zsz; j++) {
#if 0
              const auto ix = zyc::dual_ix (i,j);
              if (ix>=0) { cr[zsz*i + j]= ak [ix]; }// set only nonzeros
#endif
#if 0
              cr[zsz* i + j]
                = zyc::is_dual_nz (i,j) ? ak [zyc::dual_ux (i,j)] : 0.0 ;
#endif
#if 1
              cr [zsz* i + j] = zyc::cr_dual_elem (ak, i, j);
#endif
          } }
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              ck[i] += cr [zsz* i + j] * bk [j];// regular matmul
        } } }// transposed version below is faster
        secs_naiv += double (time.add_busy_time_now ());
        ZYC_PRAGMA_OMP(omp barrier)
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              cr [zsz* i + j] = zyc::cr_dual_elem (ck, i, j);
          } }
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              s2 [i] += cr [zsz* i + j] * ck [j];// regular matmul
        } } }
        secs_nai2 += double (time.add_busy_time_now ());
        for (uint i=0; i<zsz; i++) {chk += s2[i];}
        ZYC_PRAGMA_OMP(omp barrier)
#endif
        for (uint ti=0; ti<test_n; ti++) {// warm up
          for (uint i=0; i<vals_n; i++) {chk += a[i] * b[i];
        } }
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_CONST_PTR ak = &a [k];
          TEST_ZYC_CONST_PTR bk = &b [k];
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {// transposed CR of ak (faster)
            for (zyc::Zsize_t j=0; j<zsz; j++) {
#if 0
              const auto ix = zyc::dual_ix (j,i);
              cr[zsz* i + j] = (ix<0) ? 0.0 : ak [ix];
#endif
#if 0
              cr[zsz* i + j]
                = zyc::is_dual_nz (j,i) ? ak [zyc::dual_ux (j,i)] : 0.0;
#endif
#if 1
              cr [zsz* i + j] = zyc::cr_dual_elem (ak, j, i);
#endif
          } }
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              ck[i] += bk [i] * cr [zsz* i + j];// transposed matmul
        } } }
        secs_tnaiv += double (time.add_busy_time_now ());
        ZYC_PRAGMA_OMP(omp barrier)
        time.add_idle_time_now ();
        for (uint k=0; k<n; k+=zsz) {
          TEST_ZYC_ARRAY_PTR ck = &c [k];
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              cr [zsz* i + j] = zyc::cr_dual_elem (ck, j, i);
          } }
          for (zyc::Zsize_t i=0; i<zsz; i++) {
            for (zyc::Zsize_t j=0; j<zsz; j++) {
              s2 [i] += ck [i] * cr [zsz* i + j];// transposed matmul
        } } }
        secs_tnai2 += double (time.add_busy_time_now ());
        for (uint i=0; i<zsz; i++) {chk += s2[i];}
        ZYC_PRAGMA_OMP(omp barrier)
        time.add_idle_time_now ();
      } }//end test_n loop
      printf (" zyc, ref, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        -1, test_n * vals_n, byte_fma, flop_ref, secs_ref, flop_ref/secs_ref,
        flop_ref/byte_fma, chk);
      printf (" zyc, ref,sum2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        -1, test_n * vals_n, byte_s2, flop_ref, secs_ref2, flop_ref/secs_ref2,
        flop_ref/byte_s2, chk);
      printf (" zyc,dual, fma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_fma,
          flop_dual/secs_fma, flop_dual/byte_fma, chk);
      printf (" zyc,dual,sum2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_s2, flop_dual, secs_s2,
          flop_dual/secs_s2, flop_dual/byte_s2, chk);
      printf (" zyc,dual,tfma,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_tfma,
          flop_dual/secs_tfma, flop_dual/byte_fma, chk);
      printf (" zyc,dual, ts2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_s2, flop_dual, secs_ts2,
          flop_dual/secs_ts2, flop_dual/byte_s2, chk);
#ifdef ZYC_TEST_MOST_NAIVE
      printf (" zyc,dual,naiv,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_naiv,
          flop_dual/secs_naiv, flop_dual/byte_fma, chk);
      printf (" zyc,dual,nai2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_s2, flop_dual, secs_nai2,
          flop_dual/secs_nai2, flop_dual/byte_s2, chk);
#endif
      printf (" zyc,dual,tnai,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_fma, flop_dual, secs_tnaiv,
          flop_dual/secs_tnaiv, flop_dual/byte_fma, chk);
      printf (" zyc,dual,tns2,%2i,%10u,%7.3e,%7.3e,%7.3e,%7.3e,%6.4f,%3.1f\n",
        order, (test_n * vals_n) / uint(zsz), byte_s2, flop_dual, secs_tnai2,
          flop_dual/secs_tnai2, flop_dual/byte_s2, chk);
    }//end parallel region
    return chk;
  }
  const auto test_n  = 5;
  const auto test_sz = 8*1024*1024;
  TEST(Zyc, MultidualMultiply) {//TODO move to zyc.perf.cpp
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 0), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 1), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 2), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 3), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 4), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 5), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 6), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 7), 0.0);
    EXPECT_DOUBLE_EQ( dual_aos_fma (test_n, test_sz, 8), 0.0);
  }
} }//end femerea::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
