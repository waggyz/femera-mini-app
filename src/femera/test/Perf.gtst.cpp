#include "../femera.hpp"
#include "../../fmr/mtrl/elastic-general.hpp"


#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}
namespace femera { namespace test { namespace perf {
  const fmr::Perf_int Mega = fmr::Perf_int (1000000l);
  inline
  double test_proc_id_speed (const fmr::Perf_int n=100000l) {
    fmr::Local_int ans1=0, ans2=0;
    mini->test->time.add_idle_time_now ();
    for (fmr::Perf_int i=0; i<n; ++i) {
      ans1 += mini->proc->get_team_n ();// just returns a member variable
    }
    const auto base_time = mini->test->time.add_busy_time_now ();// - start;
    mini->test->time.add_idle_time_now ();
    for (fmr::Perf_int i=0; i<n; ++i) {
      ans2 += mini->proc->get_proc_id ();// descends the processing stack
    }
    const auto pids_time = mini->test->time.add_busy_time_now ();// - start;
    const auto ratio = double (pids_time / base_time);
    const auto base_str
      = fmr::form::si_unit (double (base_time) / double (n),"s");
    const auto pids_str
      = fmr::form::si_unit (double (pids_time) / double (n),"s");
    if (ans2 >= ans1) {// should always be true
      mini->data->send (fmr::log, "perf", "gtst", "proc",
        "%s /%s (pid/ref) = %.0fx slower each call",
        pids_str.c_str(), base_str.c_str(), ratio - 1.0);
    }
    return ratio;
  }
  TEST( TestPerf, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
FMR_WARN_INLINE_OFF
  TEST( TestProcIDPerf, PidsIsSlower ){
    EXPECT_GT( test_proc_id_speed (), 1.0 );
  }
FMR_WARN_INLINE_ON
  //
#include <valarray>
#ifdef FMR_HAS_OPENMP
#include <omp.h>
#include <immintrin.h>
#endif
#define HAS_AVX2
  //TODO Remove above includes.
  //  Move to Mtrl module.
  //
  // Empty assembly trick to prevent over-optimization of these test functions adapted from:
  // https://stackoverflow.com/questions/7083482/how-can-i-prevent-gcc-from-optimizing-out-a-busy-wait-loop
  /*
  // Test values
  // lambda = mu = 0.5
  const fmr::Phys_float test_strain [9] = {
    1.0, 0.0, 0.0,
    0.0, 0.0, 0.0,
    0.0, 0.0, 0.0
  }; */
  const fmr::Phys_float correct_value = 5.0;// sum (stress)
  //
  //NOTE Intel i7-12800H floating point math: 62.1 GOps/sec
  // https://nanoreview.net/en/cpu/intel-core-i7-12800h
  //
  // https://www.intel.com/content/dam/support/us/en/documents/processors/APP-for-Intel-Core-Processors.pdf
  //i7-12800H 537.6 Gflop/s
  //
  inline
  double mtrl_iso3_base (const fmr::Perf_int test_n=400l *Mega/1) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      femera::Work::Work_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (36*2 +3));
      timer.add_read (test_n * sizeof(fmr::Phys_float) * (9 + 3));
      timer.add_save (test_n * sizeof(fmr::Phys_float) * (9    ));
      double busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
      const fmr::Phys_float c1 = lambda + 2.0*mu;
      const fmr::Phys_float c2 = lambda;
      const fmr::Phys_float c3 = mu;
      const fmr::Phys_float D [36] = {
        c1, c2, c2, 0 , 0 , 0,
        c2, c1, c2, 0 , 0 , 0,
        c2, c2, c1, 0 , 0 , 0,
        0 , 0 , 0 , c3, 0 , 0,
        0 , 0 , 0 , 0 , c3, 0,
        0 , 0 , 0 , 0 , 0 , c3
      };
#if 0
      double H0 = 1.0, H1 = 0.0, H2 = 0.0;
      double H3 = 0.0, H4 = 0.0, H5 = 0.0;
      double H6 = 0.0, H7 = 0.0, H8 = 0.0;
#else
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
      };
#endif
      fmr::Phys_float strain_voigt [6] = { 0.0,0.0,0.0, 0.0,0.0,0.0 };
      fmr::Phys_float stress_voigt [6] = { 0.0,0.0,0.0, 0.0,0.0,0.0 };
      //
      std::valarray<fmr::Phys_float> stress (0.0, 9);
      for (int phase =0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase==1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Perf_int n=0; n < phase_n; ++n) {
#if 0
          __asm__ volatile ("" : "+g" (n) : :);
          __asm__ volatile ("" : "+g" (H0) : :);
          __asm__ volatile ("" : "+g" (H1) : :);
          __asm__ volatile ("" : "+g" (H2) : :);
          __asm__ volatile ("" : "+g" (H3) : :);
          __asm__ volatile ("" : "+g" (H4) : :);
          __asm__ volatile ("" : "+g" (H5) : :);
          __asm__ volatile ("" : "+g" (H6) : :);
          __asm__ volatile ("" : "+g" (H7) : :);
          __asm__ volatile ("" : "+g" (H8) : :);
          double H [9] = { H0,H1,H2,H3,H4,H5,H6,H7,H8 };// Snapshot the volatiles.
#endif
#if 1
//          __asm__ volatile ("" : "+g" (n) : :);
          //TODO instead call:
          fmr::mtrl::elas_dmat_base<fmr::Phys_float> (&stress[0], &D[0], &H[0],
            &strain_voigt[0], &stress_voigt[0]);
          //
#else
          strain_voigt [0] = H[0];
          strain_voigt [1] = H[4];
          strain_voigt [2] = H[8];
          strain_voigt [3] = H[5] + H[7];
          strain_voigt [4] = H[2] + H[6];
          strain_voigt [5] = H[1] + H[3];
          //
          for (fmr::Local_int i=0; i<6; ++i) { stress_voigt[i] = 0.0; }
          for (fmr::Local_int i=0; i < 6; ++i) {
            for (fmr::Local_int j=0; j < 6; ++j) {
              stress_voigt[i] += D [6*i+j] * strain_voigt [j];
          } }
          stress = {
            stress_voigt [0], stress_voigt [5], stress_voigt [4],
            stress_voigt [5], stress_voigt [1], stress_voigt [3],
            stress_voigt [4], stress_voigt [3], stress_voigt [2]
          };
#endif
        }//end kernel loop
        if (phase == 0) {// warmup
          printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
        } else {// busy time run
          busy_s = double (timer.add_busy_time_now ());
        }
        for (fmr::Local_int i=0; i<9; ++i) {
          out [omp_i]+= stress [i];
        }
      }//end phase loop
      //fprintf (stdout, "correct: %g\n", out[0]);
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-D: 3D isotropic D-matrix baseline");
      fprintf (stdout, "time: %g sec\n"      , busy_s);
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
//    return out [0];//FIXME race condition
  return (is_ok[0] ? perf[0] : -1.0);
  }
  inline
  double mtrl_iso3_lame (const fmr::Perf_int test_n=1100l *Mega) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      femera::Work::Work_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (18 + 6));
      timer.add_read (test_n * ( 9 + 2) * sizeof(fmr::Phys_float));
      timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));// returns true stress tensor.
      double busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
#if 0
      //
      double H0 = 1.0, H1 = 0.0, H2 = 0.0;
      double H3 = 0.0, H4 = 0.0, H5 = 0.0;
      double H6 = 0.0, H7 = 0.0, H8 = 0.0;
      std::valarray<double> HT (9);
      std::valarray<double> stress (9);
#else
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
      };
      fmr::Phys_float     HT [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      fmr::Phys_float stress [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
#endif
      //
      for (int phase =0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase == 1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Local_int n=0; n < phase_n; ++n) {
#if 0
          __asm__ volatile ("" : "+g" (n) : :);
          __asm__ volatile ("" : "+g" (H0) : :);
          __asm__ volatile ("" : "+g" (H1) : :);
          __asm__ volatile ("" : "+g" (H2) : :);
          __asm__ volatile ("" : "+g" (H3) : :);
          __asm__ volatile ("" : "+g" (H4) : :);
          __asm__ volatile ("" : "+g" (H5) : :);
          __asm__ volatile ("" : "+g" (H6) : :);
          __asm__ volatile ("" : "+g" (H7) : :);
          __asm__ volatile ("" : "+g" (H8) : :);
          const double H [9] = { H0,H1,H2,H3,H4,H5,H6,H7,H8 };// Snapshot the volatiles.
          //
          // Avoid valarray overhead by working with elements.
          HT [0] = H [0]; HT [1] = H [3]; HT [2] = H [6];
          HT [3] = H [1]; HT [4] = H [4]; HT [5] = H [7];
          HT [6] = H [2]; HT [7] = H [5]; HT [8] = H [8];
          for (fmr::Local_int i=0; i < 9; ++i) {
            stress[i] = mu * (H[i] + HT[i]);// 18 FLOP
          }
          const double lambda_trace = lambda * (H[0] + H[4] + H[8]);//3 FLOP
          //
          stress [0] += lambda_trace;// 1 FLOP
          stress [4] += lambda_trace;// 1 FLOP
          stress [8] += lambda_trace;// 1 FLOP
#else
//          __asm__ volatile ("" : "+g" (n) : :);
          fmr::mtrl::elas_iso_lame<fmr::Phys_float>
            (&stress[0], lambda, mu, &H[0], &HT[0]);
#endif
        }//end kernel loop
        if (phase == 0) {// warmup
          printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
        } else {// busy time run
          busy_s = double (timer.add_busy_time_now ());
        }
        for (fmr::Local_int i=0; i<9; ++i) {
          out [omp_i]+= stress [i];
        }
      }//end phase loop
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-L: 3D isotropic Lame formula");
      fprintf (stdout, "time: %g sec\n"      , busy_s);
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
//    return out;
  return (is_ok[0] ? perf[0] : -1.0);
  }
  inline
  fmr::Phys_float mtrl_iso3_scalar (const fmr::Perf_int test_n=300l *Mega) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      femera::Work::Work_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (15 + 6));
      timer.add_read (test_n * ( 9 + 2) * sizeof(fmr::Phys_float));
      timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));// returns true stress tensor.
      double busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
      const fmr::Phys_float c1 = lambda + 2.0*mu;
      const fmr::Phys_float c2 = lambda;
      const fmr::Phys_float c3 = mu;
      //
#if 0
      double H0 = 1.0, H1 = 0.0, H2 = 0.0;
      double H3 = 0.0, H4 = 0.0, H5 = 0.0;
      double H6 = 0.0, H7 = 0.0, H8 = 0.0;
      //
      std::valarray<double> stress (0.0, 9);
      std::valarray<double> stress_voigt (0.0, 6);
#else
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
      };
      fmr::Phys_float stress       [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      fmr::Phys_float stress_voigt [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
#endif
      //
      for (int phase=0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase == 1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Perf_int n=0; n < (phase_n); ++n) {
#if 0
          __asm__ volatile ("" : "+g" (n) : :);
          __asm__ volatile ("" : "+g" (H0) : :);
          __asm__ volatile ("" : "+g" (H1) : :);
          __asm__ volatile ("" : "+g" (H2) : :);
          __asm__ volatile ("" : "+g" (H3) : :);
          __asm__ volatile ("" : "+g" (H4) : :);
          __asm__ volatile ("" : "+g" (H5) : :);
          __asm__ volatile ("" : "+g" (H6) : :);
          __asm__ volatile ("" : "+g" (H7) : :);
          __asm__ volatile ("" : "+g" (H8) : :);
          const double H [9] = { H0,H1,H2,H3,H4,H5,H6,H7,H8 };// Snapshot the volatiles.
          //
          stress_voigt [0] = c1*H[0] + c2*H[4] + c2*H[8];// 5 FLOP
          stress_voigt [1] = c2*H[0] + c1*H[4] + c2*H[8];// 5 FLOP
          stress_voigt [2] = c2*H[0] + c2*H[4] + c1*H[8];// 5 FLOP
          stress_voigt [3] = (H[5] + H[7]) * c3;// 2 FLOP
          stress_voigt [4] = (H[2] + H[6]) * c3;// 2 FLOP
          stress_voigt [5] = (H[1] + H[3]) * c3;// 2 FLOP
          //
          stress = {
            stress_voigt[0], stress_voigt[5], stress_voigt[4],
            stress_voigt[5], stress_voigt[1], stress_voigt[3],
            stress_voigt[4], stress_voigt[3], stress_voigt[2]
          };
#else
//          __asm__ volatile ("" : "+g" (n) : :);
          fmr::mtrl::elas_iso_scalar<fmr::Phys_float>
            (&stress[0], c1, c2, c3, &H[0], &stress_voigt[0]);
#endif
        }// end kernel loop
        if (phase == 0) {// warmup
          printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
        } else {// busy time run
          busy_s = double (timer.add_busy_time_now ());
        }
        for (fmr::Local_int i=0; i<9; ++i) {
          out [omp_i]+= stress [i];
        }
      }//end phase loop
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-S: 3D isotropic minimum scalar");
      fprintf (stdout, "time: %g sec\n"      , busy_s);
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
//    return out;
  return (is_ok[0] ? perf[0] : -1.0);
  }
  double mtrl_iso3_avx2 (const fmr::Perf_int test_n=3000 *Mega) {
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      femera::Work::Work_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (4*3 +1 +8*3));// 37 FLOP
      timer.add_read (test_n * ( 12 + 2) * sizeof(fmr::Phys_float));// 14 actual, 11 effective
      timer.add_save (test_n * ( 12)     * sizeof(fmr::Phys_float));// 12 actua;.  9 effective
      double busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu     = 0.5;
      //
      fmr::Phys_float H0 = 1.0, H1 = 0.0, H2 = 0.0;
      fmr::Phys_float H3 = 0.0, H4 = 0.0, H5 = 0.0;
      fmr::Phys_float H6 = 0.0, H7 = 0.0, H8 = 0.0;
      //
#if 0
      volatile __m256d vA0 = {H3, H2,H1,H0};
      volatile __m256d vA1 = {H7, H6,H5,H4};
      volatile __m256d vA2 = {0.0, 0.0,0.0,H8};
#endif
#if 0
      volatile __m256d vA0 = {H0, H1, H2, 0.0};
      volatile __m256d vA1 = {H3, H4, H5, 0.0};
      volatile __m256d vA2 = {H6, H7, H8, 0.0};
#endif
#if 1
      volatile __m256d vA0 = {H0, H1, H2, H3};
      volatile __m256d vA1 = {H4, H5, H6, H7};
      volatile __m256d vA2 = {H8,0.0,0.0,0.0};
#endif
      //
      std::valarray<fmr::Phys_float> stress (12);
      //
      for (int phase=0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase == 1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Perf_int n=0; n < (phase_n); ++n) {
#if 0
          __asm__ volatile ("" : "+g" (n) : :);
          __asm__ volatile ("" : "+g" (vA0) : :);
          __asm__ volatile ("" : "+g" (vA1) : :);
          __asm__ volatile ("" : "+g" (vA2) : :);
          __m256d vA[3] = { vA0,vA1,vA2 };// Snapshot volatiles
#endif
          __m256d vA[3] = { vA0,vA1,vA2 };
#ifdef HAS_AVX2
#if 0
          // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
          // Structure of S and H
          //  3   2   1     0
          // sxx sxy sxz | sxy
          // sxy syy syz | sxz
          // sxz syz szz | ---
          __m256d Ssum=_mm256_setzero_pd();
          {
          const __m256d z0 =_mm256_set_pd(0.0,1.0,1.0,1.0);
          const __m256d ml =_mm256_set_pd(lambda,mu,mu,mu);
          vA[0]=_mm256_permute4x64_pd( vA[0]*z0,_MM_SHUFFLE(0,2,3,1) );
          Ssum+= vA[0]*ml;// 4 FLOP
          vA[1]=_mm256_permute4x64_pd( vA[1]*z0,_MM_SHUFFLE(1,3,2,0) );
          Ssum+= vA[1]*ml;// 4 FLOP
          vA[2]=_mm256_permute4x64_pd( vA[2]*z0,_MM_SHUFFLE(2,0,1,3) );
          Ssum+= vA[2]*ml;// 4 FLOP
          }
          //      3   2   1   0
          //     sxy 0.0 sxz sxx
          //     syx syz 0.0 syy
          //     0.0 szy szx szz
          //
          // mu*(sxy syz sxz)trace(H)*lambda : Ssum
          {
          const __m256d m2=_mm256_set_pd(2.0*mu,0.0,0.0,0.0);// 1 FLOP
          //      3   2   1   0
          // mu*(sxy 0.0 sxz)sxx*2*mu+lambda*trace(H)
          // mu*(sxy syz 0.0)syy*2*mu+lambda*trace(H)
          // mu*(0.0 syz sxz)szz*2*mu+lambda*trace(H)
#if 0
          printf("S step 2\n");
          print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
          vA[0]=_mm256_permute4x64_pd( Ssum + vA[0]*m2,_MM_SHUFFLE(3,2,0,3) );
          // 8 FLOP
          vA[1]=_mm256_permute4x64_pd( Ssum + vA[1]*m2,_MM_SHUFFLE(3,1,3,0) );
          // 8 FLOP
          vA[2]=_mm256_permute4x64_pd( Ssum + vA[2]*m2,_MM_SHUFFLE(3,3,1,2) );
          // 8 FLOP
          }
#if 0
          printf("S step 3\n");
          print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
#else
          fmr::mtrl::elas_iso_avx2<fmr::Phys_float>
            (&vA[0], lambda, mu);
#endif
#endif
#if 0
#ifdef HAS_AVX
// Does not have avx2 support
          //TODO Pull AVX into mtrl_iso3_avx (..)
          Phys_float VECALIGNED fS[12];
          _mm256_store_pd(&fS[0],vA[0]);
          _mm256_store_pd(&fS[4],vA[1]);
          _mm256_store_pd(&fS[8],vA[2]);
          {
          const Phys_float tr = (fS[0]+fS[5]+fS[10]) * lambda;
          const __m256d mw= _mm256_set1_pd(mu);
          _mm256_store_pd( &fS[0], mw * vA[0] );// sxx sxy sxz | syx
          _mm256_store_pd( &fS[4], mw * vA[1] );// syx syy syz | szx
          _mm256_store_pd( &fS[8], mw * vA[2] );// szx szy szz | ---
          fS[0]=2.0*fS[0]+tr; fS[5]=2.0*fS[5]+tr; fS[10]=2.0*fS[10]+tr;
          }
          fS[1]+= fS[4];
          fS[2]+= fS[8];
          fS[6]+= fS[9];
          fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
          vA[0] = _mm256_load_pd(&fS[0]); // [a3 a2 a1 a0]
          vA[1] = _mm256_load_pd(&fS[4]); // [a6 a5 a4 a3]
          vA[2] = _mm256_load_pd(&fS[8]); // [a9 a8 a7 a6]
#if 0
          __asm__ volatile ("" : "+g" (H0) : :);
          __asm__ volatile ("" : "+g" (H1) : :);
          __asm__ volatile ("" : "+g" (H2) : :);
          __asm__ volatile ("" : "+g" (H3) : :);
          __asm__ volatile ("" : "+g" (H4) : :);
          __asm__ volatile ("" : "+g" (H5) : :);
          __asm__ volatile ("" : "+g" (H6) : :);
          __asm__ volatile ("" : "+g" (H7) : :);
          __asm__ volatile ("" : "+g" (H8) : :);
          const double H [9] = { H0,H1,H2,H3,H4,H5,H6,H7,H8 };// Snapshot the volatiles.
#endif
#endif
#endif
          //TODO try to remove the following from this kernel?
          _mm256_storeu_pd( &stress[0], vA[0]);
          _mm256_storeu_pd( &stress[4], vA[1]);
          _mm256_storeu_pd( &stress[8], vA[2]);
        }// end kernel loop
        if (phase == 0) {// warmup
          printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
        } else {// busy time run
          busy_s = double (timer.add_busy_time_now ());
        }
        stress [ 3] = 0.0;// Zero the unused lanes.
        stress [ 7] = 0.0;
        stress [11] = 0.0;
        for (fmr::Local_int i=0; i<12; ++i) {
          out [omp_i]+= stress [i];
        }
      }//end phase loop
      //fprintf (stdout,"AVX2 out: %g\n", out[omp_i]);
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-V: 3D isotropic AVX2");
      fprintf (stdout, "time: %g sec\n"      , busy_s);
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
//    return out;
  return (is_ok[0] ? perf[0] : -1.0);
  }
  inline
  fmr::Phys_float mtrl_iso3_valarray (const fmr::Perf_int test_n=130l *Mega) {// about 10 sec
  //  Uses valarray operations within kernel.
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      femera::Work::Work_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (36*2 +3));
      timer.add_read (test_n * (9 + 3) * sizeof(double));
      timer.add_save (test_n * (9) * sizeof(double));
      double busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
      const fmr::Phys_float c1 = lambda + 2.0*mu;
      const fmr::Phys_float c2 = lambda;
      const fmr::Phys_float c3 = mu;
      const fmr::Phys_float D [36] = {
        c1, c2, c2, 0 , 0 , 0,
        c2, c1, c2, 0 , 0 , 0,
        c2, c2, c1, 0 , 0 , 0,
        0 , 0 , 0 , c3, 0 , 0,
        0 , 0 , 0 , 0 , c3, 0,
        0 , 0 , 0 , 0 , 0 , c3
      };
      fmr::Phys_float H0 = 1.0, H1 = 0.0, H2 = 0.0;
      fmr::Phys_float H3 = 0.0, H4 = 0.0, H5 = 0.0;
      fmr::Phys_float H6 = 0.0, H7 = 0.0, H8 = 0.0;
      //
      std::valarray<fmr::Phys_float> strain_voigt (0.0, 6);
      std::valarray<fmr::Phys_float> stress_voigt (0.0, 6);
      std::valarray<fmr::Phys_float> stress (0.0, 9);
      //
      for (int phase =0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase==1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Perf_int n=0; n < phase_n; ++n) {
          __asm__ volatile ("" : "+g" (n) : :);
          __asm__ volatile ("" : "+g" (H0) : :);
          __asm__ volatile ("" : "+g" (H1) : :);
          __asm__ volatile ("" : "+g" (H2) : :);
          __asm__ volatile ("" : "+g" (H3) : :);
          __asm__ volatile ("" : "+g" (H4) : :);
          __asm__ volatile ("" : "+g" (H5) : :);
          __asm__ volatile ("" : "+g" (H6) : :);
          __asm__ volatile ("" : "+g" (H7) : :);
          __asm__ volatile ("" : "+g" (H8) : :);
#if 0
          strain_voigt = {H0, H4, H8, H5+H7, H2+H6, H1+H3};//Snapshot not needed?
#else
          const fmr::Phys_float H [9] = { H0,H1,H2,H3,H4,H5,H6,H7,H8 };// Snapshot the volatiles.
          strain_voigt = {H[0], H[4], H[8], H[5]+H[7], H2+H[6], H[1]+H[3]};
#endif
          for (fmr::Local_int i=0; i<6; ++i) { stress_voigt[i] = 0.0; }
          for (fmr::Local_int i=0; i < 6; ++i) {
            for (fmr::Local_int j=0; j < 6; ++j) {
              stress_voigt[i] += D [6*i+j] * strain_voigt[j];
          } }
          stress = {
            stress_voigt[0], stress_voigt[5], stress_voigt[4],
            stress_voigt[5], stress_voigt[1], stress_voigt[3],
            stress_voigt[4], stress_voigt[3], stress_voigt[2]
          };
        }//end kernel loop
        if (phase == 0) {// warmup
          printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
        } else {// busy time run
          busy_s = double (timer.add_busy_time_now ());
        }
        for (fmr::Local_int i=0; i<9; ++i) {
          out [omp_i]+= stress [i];
        }
      }//end phase loop
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-N: 3D isotropic naive D-matrix");
      fprintf (stdout, "time: %g sec\n"      , busy_s);
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
//    return out;
  return (is_ok[0] ? perf[0] : -1.0);
  }
  fmr::Perf_int test_div = 1000;
#if 1
  TEST( PerfMtrlIsoLame, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_lame (1200l*Mega/test_div), mtrl_iso3_base (500l*Mega/test_div) );
  }
#endif
#if 1
  TEST( PerfMtrlIsoScalar, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_scalar (1800l*Mega/test_div), mtrl_iso3_base (500l*Mega/test_div) );
  }
#endif
#if 1
  TEST( PerfMtrlIsoAVX2, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_avx2 (1400*Mega/test_div), mtrl_iso3_base (500l*Mega/test_div) );
  }
#endif
#if 0
// not really worth continuing to test
  TEST( PerfMtrlIsoValarray, IsCorrectAndFaster ){
    EXPECT_DOUBLE_EQ( mtrl_iso3_valarray (130l*Mega/test_div), mtrl_iso3_base (500l*Mega/test_div) );
  }
#endif
} } }//end femera::test::perf:: namespace

