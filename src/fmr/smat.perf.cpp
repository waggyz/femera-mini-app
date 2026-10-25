#include "../femera/femera.hpp"
//#include "fmr.hpp"
#include "smat.hpp"

#include <valarray>

#include <gtest/gtest.h>

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  auto perf_time = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>();
  // Baseline times
  fmr::Perf_float m2x2_base_time = -1.0;
  fmr::Perf_float m3x3_base_time = -1.0;
  fmr::Perf_float m4x4_base_time = -1.0;

  fmr::Perf_float m2x2_dets_time = -1.0;// with dets
  fmr::Perf_float m3x3_dets_time = -1.0;
  fmr::Perf_float m4x4_dets_time = -1.0;
  //
  inline
  fmr::Perf_float time_m2x2 (int test_case=0, uint n=4096*1024) {
    auto m2x2_det = std::valarray<double>(5*n);
    auto m2x2_out = std::valarray<double>(4*n);
    auto m2x2_in = std::valarray<double>(4*n);
    for (uint i = 0; i < n; ++i){
      m2x2_in[4*i+0] = 1.0;// Invertible matrix:
      m2x2_in[4*i+1] = 0.0;// [ 1.0  0.0 ]
      m2x2_in[4*i+2] = 0.0;// [ 0.0  2.0 ]
      m2x2_in[4*i+3] = 2.0;// det: 2.0
    }
    int bad_dets = 0;
    fmr::Perf_float this_time =-1.0;
    switch (test_case) {
      case 0: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (4*n);
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_out[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_inv_ref (&m2x2_in[4*i], &m2x2_out[4*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        m2x2_base_time = this_time;
        break;
      }
      case 1: {
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (4*n);
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_out[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_inv_a (&m2x2_in[4*i], &m2x2_out[4*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        break;
      }
      case 2: {
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (4*n);
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_out[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_inv_b (&m2x2_in[4*i], &m2x2_out[4*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        break;
      }
      case 3: {
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (5*n);
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_det[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_inv_det_ref (
            &m2x2_in[4*i], &m2x2_det[5*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        m2x2_dets_time = this_time;
        break;
      }
      case 4: {
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (5*n);
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_det[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_invt_det_ref (
            &m2x2_in[4*i], &m2x2_det[5*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        break;
      }
      default: {
        return fmr::Perf_float(-1.0);
      }
    }// end switch test_case
    return bad_dets == 0 ? this_time : fmr::Perf_float(-1.0);
  }


  inline
  fmr::Perf_float time_m3x3 (int test_case=0, uint n=1024*1024) {
    auto m3x3_det = std::valarray<double>(10*n);
    auto m3x3_out = std::valarray<double>(9*n);
    auto m3x3_in = std::valarray<double>(9*n);
    for (uint i = 0; i < n; ++i){
      m3x3_in[9*i+0] = 1.0;// Invertible matrix:
      m3x3_in[9*i+1] = 0.0;// [ 1.0  0.0  0.0 ]
      m3x3_in[9*i+2] = 0.0;// [ 0.0  2.0  0.0 ]
      m3x3_in[9*i+3] = 0.0;// [ 0.0  0.0  4.0 ]
      m3x3_in[9*i+4] = 2.0;// det: 8.0
      m3x3_in[9*i+5] = 0.0;
      m3x3_in[9*i+6] = 0.0;
      m3x3_in[9*i+7] = 0.0;
      m3x3_in[9*i+8] = 4.0;
    }
    int bad_dets = 0;
    fmr::Perf_float this_time =-1.0;
    switch (test_case) {
      case 0: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n);
        perf_time.add_save (4*n);
        perf_time.add_flop (41*n);
        // Warm up
        for (uint i = 0; i < (n*9); ++i) {
          m3x3_out[i] = 5.0 * m3x3_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m3x3_inv_ref (&m3x3_in[9*i], &m3x3_out[9*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        m3x3_base_time = this_time;
        break;
      }
      case 1: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (9*n);
        perf_time.add_save (9*n);
        perf_time.add_flop (49*n);
        // Warm up
        for (uint i = 0; i < (n*9); ++i) {
          m3x3_out[i] = 5.0 * m3x3_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m3x3_inv_a (&m3x3_in[9*i], &m3x3_out[9*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        break;
      }
      case 2: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (9*n);
        perf_time.add_save (10*n);
        perf_time.add_flop (41*n);
        // Warm up
        for (uint i = 0; i < (n*9); ++i) {
          m3x3_det[i] = 5.0 * m3x3_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m3x3_invt_det_ref (&m3x3_in[9*i], &m3x3_det[9*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        m3x3_dets_time = this_time;
        break;
      }
      case 3: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (9*n);
        perf_time.add_save (10*n);
        perf_time.add_flop (49*n);
        // Warm up
        for (uint i = 0; i < (n*9); ++i) {
          m3x3_det[i] = 5.0 * m3x3_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m3x3_invt_det_a (&m3x3_in[9*i], &m3x3_det[9*i]));
        }
        this_time = perf_time.add_busy_time_now ();
        break;
      }
      default: {
        return fmr::Perf_float(-1.0);
      }
    }// end switch test_case
    return bad_dets == 0 ? this_time : fmr::Perf_float(-1.0);
  }



TEST(FmrSmatPerf, BaseTime) {// Sets baseline times
  EXPECT_GT(time_m2x2(0), 0.0);// m2x2_base_time set AFTER test
  EXPECT_GT(time_m2x2(3), 0.0);

  EXPECT_GT(time_m3x3(0), 0.0);
  EXPECT_GT(time_m3x3(2), 0.0);
}
TEST(FmrSmatPerf, M2x2Inv) {
  EXPECT_GT(time_m2x2(1), m2x2_base_time);// slower than baseline?
  EXPECT_GT(time_m2x2(2), m2x2_base_time);// slower than baseline
}
TEST(FmrSmatPerf, M2x2InvDets) {
  EXPECT_GT(time_m2x2(4), m2x2_dets_time);
}

TEST(FmrSmatPerf, M3x3Inv) {
  EXPECT_GT(time_m3x3(1), m3x3_base_time);// slower than baseline
  EXPECT_GT(time_m3x3(3), m3x3_base_time);// slower than baseline
}

} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
