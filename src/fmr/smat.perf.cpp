#include "../femera/femera.hpp"
#include "smat.hpp"

#include <gtest/gtest.h>

#include <valarray>

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  const auto perf_tol_rel = fmr::Perf_float(fmr::perf::perf_tol_rel);
  auto perf_time = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>();

  // Baseline times
  fmr::Perf_float m2x2_base_time = -1.0;
  fmr::Perf_float m3x3_base_time = -1.0;
  fmr::Perf_float m4x4_base_time = -1.0;

  fmr::Perf_float m2x2_dets_time = -1.0;// with dets
  fmr::Perf_float m3x3_dets_time = -1.0;
  fmr::Perf_float m4x4_dets_time = -1.0;
  //
  // Should try to exercise L3 cache
  // Target abouut 1 MB/core and repeat tests.

void send_perf_results (std::string label, int test_case,
  fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>& perf,
  fmr::Perf_float this_time,
  int bad_dets, uint test_count);

  void send_perf_results (std::string label, int test_case,
    fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>& perf,
    fmr::Perf_float this_time,
    int bad_dets, uint test_count) {
      mini->data->send (fmr::out, "math", "smat", label.c_str(),
        "(%i) %s: %s/%s, errs: %u",
        test_case,
        fmr::form::si_unit (perf.get_busy_unit_speed (), "inv /s").c_str(),
        fmr::form::si_unit (perf.get_unit (), "inv ").c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        bad_dets);
      mini->data->send (fmr::out, "math", "smat", label.c_str(),
        "(%i) %s: %s/%s, reps: %u",
        test_case,
        fmr::form::si_unit (perf.get_busy_flop_speed (), "flop/s").c_str(),
        fmr::form::si_unit (perf.get_flop (), "flop").c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        test_count);
      mini->data->send (fmr::out, "math", "smat", label.c_str(),
        "(%i) %s: %s/%s,   AI: %g",
        test_case,
        fmr::form::si_unit (perf.get_busy_data_speed (), "B   /s").c_str(),
        fmr::form::si_unit (perf.get_byte (), "B   ").c_str(),
        fmr::form::si_unit (this_time, "s").c_str(),
        double(perf.get_arithmetic_intensity ()));
  }

  inline
  fmr::Perf_float time_m2x2 (
    int test_case=0, uint n=1024*1024/8/sizeof(double), uint reps=512) {
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
    fmr::Perf_float this_time = 0.0;
    switch (test_case) {
      case 0: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (4*n*sizeof(double));
          perf_time.add_save (4*n*sizeof(double));
          perf_time.add_flop (8*n);
          // Warm up
          for (uint i = 0; i < (n*4); ++i) {
            m2x2_out[i] = 5.0 * m2x2_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m2x2_inv_ref (&m2x2_in[4*i], &m2x2_out[4*i]));
          }
          this_time += perf_time.add_busy_time_now ();
          // this_time is redundant with timer accumulators, but it's ok.
        }
        m2x2_base_time = this_time;
        break;
      }
      case 1: {
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
        // Set timer count, bytes in/out, flops
        perf_time.add_count (n);
        perf_time.add_read (4*n*sizeof(double));
        perf_time.add_save (4*n*sizeof(double));
        perf_time.add_flop (8*n);
        // Warm up
        for (uint i = 0; i < (n*4); ++i) {
          m2x2_out[i] = 5.0 * m2x2_in[i];
        }
        perf_time.add_idle_time_now ();// Start timing.
        for (uint i = 0; i < n; ++i) {
          bad_dets += abs(::fmr::math::m2x2_inv_a (&m2x2_in[4*i], &m2x2_out[4*i]));
        }
        this_time += perf_time.add_busy_time_now ();
        }
        break;
      }
      case 2: {
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (4*n*sizeof(double));
          perf_time.add_save (4*n*sizeof(double));
          perf_time.add_flop (8*n);
          // Warm up
          for (uint i = 0; i < (n*4); ++i) {
            m2x2_out[i] = 5.0 * m2x2_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m2x2_inv_b (&m2x2_in[4*i], &m2x2_out[4*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        break;
      }
      case 3: {
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (4*n*sizeof(double));
          perf_time.add_save (5*n*sizeof(double));
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
          this_time += perf_time.add_busy_time_now ();
        }
      m2x2_dets_time = this_time;
      break;
      }
      case 4: {
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (4*n*sizeof(double));
          perf_time.add_save (5*n*sizeof(double));
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
          this_time += perf_time.add_busy_time_now ();
        }
      break;
      }
      case 5: {
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (4*n*sizeof(double));
          perf_time.add_save (5*n*sizeof(double));
          perf_time.add_flop (8*n);
          // Warm up
          for (uint i = 0; i < (n*4); ++i) {
            m2x2_det[i] = 5.0 * m2x2_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::smat_invt_det<2> (
              &m2x2_in[4*i], &m2x2_det[5*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
      break;
      }
      default: {
        return fmr::Perf_float(-1.0);
      }
    }// end switch test_case
    send_perf_results ("m2x2", test_case, perf_time, this_time, bad_dets, reps);
    return bad_dets == 0 ? this_time : fmr::Perf_float(-1.0);
  }

  inline
  fmr::Perf_float time_m3x3 (
    int test_case=0, uint n=1024*1024/16/sizeof(double), uint reps=512) {
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
    fmr::Perf_float this_time = 0.0;
    switch (test_case) {
      case 0: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (9*n*sizeof(double));
          perf_time.add_save (9*n*sizeof(double));
          perf_time.add_flop (41*n);
          // Warm up
          for (uint i = 0; i < (n*9); ++i) {
            m3x3_out[i] = 5.0 * m3x3_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m3x3_inv_ref (&m3x3_in[9*i], &m3x3_out[9*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        m3x3_base_time = this_time;
        break;
      }
      case 1: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (9*n * sizeof(double));
          perf_time.add_save (9*n * sizeof(double));
          perf_time.add_flop (49*n);
          // Warm up
          for (uint i = 0; i < (n*9); ++i) {
            m3x3_out[i] = 5.0 * m3x3_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m3x3_inv_a (&m3x3_in[9*i], &m3x3_out[9*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        break;
      }
      case 2: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (9*n * sizeof(double));
          perf_time.add_save (10*n * sizeof(double));
          perf_time.add_flop (41*n);
          // Warm up
          for (uint i = 0; i < (n*9); ++i) {
            m3x3_det[i] = 5.0 * m3x3_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m3x3_invt_det_ref (&m3x3_in[9*i], &m3x3_det[9*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        m3x3_dets_time = this_time;
        break;
      }
      case 3: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (9*n * sizeof(double));
          perf_time.add_save (10*n * sizeof(double));
          perf_time.add_flop (49*n);
          // Warm up
          for (uint i = 0; i < (n*9); ++i) {
            m3x3_det[i] = 5.0 * m3x3_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::m3x3_invt_det_a (&m3x3_in[9*i], &m3x3_det[9*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        break;
      }
      case 4: {// Scope this timing block.
        bad_dets = 0;
        perf_time.reset ();
        for (uint r = 0; r < reps; ++r) {
          // Set timer count, bytes in/out, flops
          perf_time.add_count (n);
          perf_time.add_read (9*n * sizeof(double));
          perf_time.add_save (10*n * sizeof(double));
          perf_time.add_flop (49*n);
          // Warm up
          for (uint i = 0; i < (n*9); ++i) {
            m3x3_det[i] = 5.0 * m3x3_in[i];
          }
          perf_time.add_idle_time_now ();// Start timing.
          for (uint i = 0; i < n; ++i) {
            bad_dets += abs(::fmr::math::smat_invt_det<3> (&m3x3_in[9*i], &m3x3_det[9*i]));
          }
          this_time += perf_time.add_busy_time_now ();
        }
        break;
      }
      default: {
        return fmr::Perf_float(-1.0);
      }
    }// end switch test_case
    send_perf_results ("m3x3", test_case, perf_time, this_time, bad_dets, reps);
    return bad_dets == 0 ? this_time : fmr::Perf_float(-1.0);
  }

TEST(FmrSmatPerf, BaseTime) {// Sets baseline times
  EXPECT_GT(time_m2x2(0), 0.0);// m2x2_base_time set AFTER test
  EXPECT_GT(time_m3x3(0), m2x2_base_time);// slower than 2x2
}
//NOTE These all run about the same as baseline on dev laptop.
//     The conditionals below are for checking again on prod node.
TEST(FmrSmatPerf, M2x2Inv) {
#if FMR_PERF_PCT < 15
  EXPECT_GT(time_m2x2(1), m2x2_base_time);// slower than baseline?
  EXPECT_GT(time_m2x2(2), m2x2_base_time);// slower than baseline
#else
  EXPECT_NEAR(time_m2x2(1), m2x2_base_time, perf_tol_rel * m2x2_base_time);
  EXPECT_GT(time_m2x2(2), m2x2_base_time);// slower than baseline
#endif
}
TEST(FmrSmatPerf, M2x2InvDets) {
#if FMR_PERF_PCT < 15
  EXPECT_GT(time_m2x2(3), m2x2_base_time);// slower than w/out det
  EXPECT_GT(time_m2x2(4), m2x2_dets_time);// slower than baseline
  EXPECT_GT(time_m2x2(5), m2x2_dets_time);
#else
  EXPECT_NEAR(time_m2x2(3), m2x2_base_time, perf_tol_rel * m2x2_base_time);
  EXPECT_NEAR(time_m2x2(4), m2x2_dets_time, perf_tol_rel * m2x2_dets_time);
  EXPECT_NEAR(time_m2x2(5), m2x2_dets_time, perf_tol_rel * m2x2_dets_time);
#endif
}

TEST(FmrSmatPerf, M3x3Inv) {
#if FMR_PERF_PCT < 15
  EXPECT_GT(time_m3x3(1), m3x3_base_time);// slower than baseline
#else
  EXPECT_NEAR(time_m3x3(1), m3x3_base_time, perf_tol_rel * m3x3_base_time);
#endif
}
TEST(FmrSmatPerf, M3x3InvDets) {
#if FMR_PERF_PCT < 15
  EXPECT_GT(time_m3x3(2), m3x3_base_time);// slower than w/out dets?
  EXPECT_GT(time_m3x3(3), m3x3_dets_time);// slower than baseline?
  EXPECT_GT(time_m3x3(4), m3x3_dets_time);
#else
  EXPECT_NEAR(time_m3x3(2), m3x3_base_time, perf_tol_rel * m3x3_base_time);
  EXPECT_NEAR(time_m3x3(3), m3x3_dets_time, perf_tol_rel * m3x3_dets_time);
  EXPECT_NEAR(time_m3x3(4), m3x3_dets_time, perf_tol_rel * m3x3_dets_time);
#endif
}
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
