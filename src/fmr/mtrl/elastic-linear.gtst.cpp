#include "elastic-linear.hpp"

#include "../perf/Meter.hpp"

#include <gtest/gtest.h>
#include <valarray>

#ifdef FMR_HAS_OPENMP
#include <omp.h>
#endif

#include <immintrin.h>

// Empty assembly trick might prevent over-optimization of these functions.
// Adapted from:
// https://stackoverflow.com/questions/7083482/how-can-i-prevent-gcc-from-optimizing-out-a-busy-wait-loop
//
//NOTE Intel i7-12800H floating point math: 62.1 GOps/sec
// https://nanoreview.net/en/cpu/intel-core-i7-12800h
//
// https://www.intel.com/content/dam/support/us/en/documents/processors/APP-for-Intel-Core-Processors.pdf
//i7-12800H 537.6 Gflop/s

//TODO add another function parameter to run once for correctness,
//     multiple for performance?

using Perf_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;

fmr::Perf_int test_div = 1000;// reduce 10-sec rapid development tests.

const static auto perf_NaN = std::numeric_limits<fmr::Perf_float>::quiet_NaN();
const static fmr::Perf_int Mega = fmr::Perf_int (1000000l);
//
const static fmr::mtrl::elastic::Isotropic_lame_parameters<fmr::Phys_float>
  test_lame = {2.0, 1.0};// lambda, mu
const static fmr::mtrl::elastic::Cubic_dmat_constants<fmr::Phys_float>
  test_cubic = {test_lame[0] + 2.0*test_lame[1], test_lame[0], test_lame[1]};
volatile fmr::Phys_float test_H [9] = {
  1.0, 0.0, 0.0,
  0.0, 0.0, 1.0,
  0.0, 1.0, 0.0
};
const fmr::Phys_float correct_value = 24.0;// sum (stress)
const fmr::Phys_float correct_eps
  = 50.0 * std::numeric_limits<fmr::Phys_float>::epsilon();

inline
fmr::Perf_float mtrl_iso3_dmat
 (const fmr::Perf_int test_n=500l *Mega/1) {// ~ 10 sec
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float>
    out (omp_n);// performance if correct, nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * (36*2 +3));
    timer.add_read (test_n * sizeof(fmr::Phys_float) * (9 + 3));
    timer.add_save (test_n * sizeof(fmr::Phys_float) * (9    ));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float     mu = test_lame[1];
    const fmr::Phys_float c1 = lambda + 2.0*mu;
    const fmr::Phys_float c2 = lambda;
    const fmr::Phys_float c3 = mu;
    const fmr::mtrl::elastic::dmat_full<fmr::Phys_float> D = {
      c1, c2, c2, 0 , 0 , 0,
      c2, c1, c2, 0 , 0 , 0,
      c2, c2, c1, 0 , 0 , 0,
      0 , 0 , 0 , c3, 0 , 0,
      0 , 0 , 0 , 0 , c3, 0,
      0 , 0 , 0 , 0 , 0 , c3
    };
    volatile fmr::Phys_float H [9] = {
      test_H[0], test_H[1], test_H[2],
      test_H[3], test_H[4], test_H[5],
      test_H[6], test_H[7], test_H[8]
    };
    fmr::Phys_float strain_voigt [6] = { 0.0,0.0,0.0, 0.0,0.0,0.0 };
    fmr::Phys_float stress_voigt [6] = { 0.0,0.0,0.0, 0.0,0.0,0.0 };
    //
    fmr::Phys_float stress [9] = { 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0 };
    for (int phase =0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// warmup
      if (phase==1) {// busy time run
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Perf_int n=0; n < phase_n; ++n) {
        //
        fmr::mtrl::elastic::linear_3d_dmat_base<fmr::Phys_float>
          (&stress[0], &D[0], &H[0], &strain_voigt[0], &stress_voigt[0]);
        //
      }//end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s = timer.add_busy_time_now ();
      }
      for (fmr::Local_int i=0; i<9; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-D: elastic 3D isotropic D-matrix baseline "
      "per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" , double
      (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return is_ok[0] ? perf[0] : perf_NaN;
}
#ifdef FMR_HAS_MKL
//TODO placeholder for Intel MKL symmetric version
inline
fmr::Perf_float mtrl_iso3_spmv
  (const fmr::Perf_int test_n=500l *Mega/1) {// ~ 10 sec
//
}
#endif
inline
fmr::Perf_float mtrl_iso3_lame
 (const fmr::Perf_int test_n=1200l *Mega) {// about 10 sec
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float>
    out (omp_n);// performance if correct, nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * (18 + 6));
    timer.add_read (test_n * ( 9 + 2) * sizeof(fmr::Phys_float));
    timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float mu = test_lame[1];
    volatile fmr::Phys_float H [9] = {
      test_H[0], test_H[1], test_H[2],
      test_H[3], test_H[4], test_H[5],
      test_H[6], test_H[7], test_H[8]
    };
    fmr::Phys_float     HT [9] = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
    fmr::Phys_float stress [9] = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
    //
    for (int phase =0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// warmup
      if (phase == 1) {// busy time run
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Local_int n=0; n < phase_n; ++n) {
        //
        fmr::mtrl::elastic::linear_3d_isotropic_lame<fmr::Phys_float>
          (&stress[0], lambda, mu, &H[0], &HT[0]);
        //
      }//end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s = timer.add_busy_time_now ();
      }
      for (fmr::Local_int i=0; i<9; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-L: 3D elastic isotropic Lame formula per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" ,
      double (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return (is_ok[0] ? perf[0] : perf_NaN);
}
inline
fmr::Perf_float mtrl_iso3_scalar_a
 (const fmr::Perf_int test_n=1800l *Mega) {// about 10 sec
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float>  out (omp_n);// performance if correct,
                                              // nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * (15 + 6));
    timer.add_read (test_n * ( 9 + 2) * sizeof(fmr::Phys_float));
    timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float mu = test_lame[1];
    const fmr::Phys_float c1 = lambda + 2.0*mu;
    const fmr::Phys_float c2 = lambda;
    const fmr::Phys_float c3 = mu;
    //
    volatile fmr::Phys_float H [9] = {
      test_H[0], test_H[1], test_H[2],
      test_H[3], test_H[4], test_H[5],
      test_H[6], test_H[7], test_H[8]
    };
    fmr::Phys_float stress [9] = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
    //
    for (int phase=0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// warmup
      if (phase == 1) {// busy time run
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Perf_int n=0; n < (phase_n); ++n) {
        //
        fmr::mtrl::elastic::linear_3d_cubic_scalar_a<fmr::Phys_float>
          (&stress[0], c1, c2, c3, &H[0]);
        //
      }// end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s = timer.add_busy_time_now ();
      }
      for (fmr::Local_int i=0; i<9; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-S: 3D elastic isotropic minimum scalar A "
      "per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" ,
      double (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return (is_ok[0] ? perf[0] : perf_NaN);
}
inline
fmr::Perf_float mtrl_iso3_scalar_b
 (const fmr::Perf_int test_n=1800l *Mega) {// about 10 sec
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float>
    out (omp_n);// performance if correct, nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * (15 + 6));
    timer.add_read (test_n * ( 9 + 2) * sizeof(fmr::Phys_float));
    timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float mu = test_lame[1];
    const fmr::Phys_float c1 = lambda + 2.0*mu;
    const fmr::Phys_float c2 = lambda;
    const fmr::Phys_float c3 = mu;
    //
    volatile fmr::Phys_float H [9] = {
      test_H[0], test_H[1], test_H[2],
      test_H[3], test_H[4], test_H[5],
      test_H[6], test_H[7], test_H[8]
    };
    fmr::Phys_float stress       [9]={0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
    fmr::Phys_float stress_voigt [9]={0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
    //
    for (int phase=0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// warmup
      if (phase == 1) {// busy time run
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Perf_int n=0; n < (phase_n); ++n) {
        //
        fmr::mtrl::elastic::linear_3d_cubic_scalar_b<fmr::Phys_float>
          (&stress[0], c1, c2, c3, &H[0], &stress_voigt[0]);
        //
      }// end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s = timer.add_busy_time_now ();
      }
      for (fmr::Local_int i=0; i<9; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-S: 3D elastic isotropic minimum scalar B "
      "per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" ,
      double (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return (is_ok[0] ? perf[0] : perf_NaN);
}
#ifdef FMR_HAS_AVX
inline
fmr::Perf_float mtrl_iso3_avx
 (const fmr::Perf_int test_n=1250l *Mega) {
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float> 
    out (omp_n);// performance if correct, nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * ( 3 + 12 + 6));// 21 FLOP
    timer.add_read (test_n * ( 12 + 2)      // 14 actual, 11 effective
      * sizeof(fmr::Phys_float));
    timer.add_save (test_n * ( 12)          // 12 actual.  9 effective
      * sizeof(fmr::Phys_float));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float mu     = test_lame[1];
    //
    volatile __m256d vA0 = {test_H[0], test_H[1], test_H[2], 0.0};
    volatile __m256d vA1 = {test_H[3], test_H[4], test_H[5], 0.0};
    volatile __m256d vA2 = {test_H[6], test_H[7], test_H[8], 0.0};
    //
    __m256d s[3];//32-byte aligned to use _store instead of _storeu in kernel
    s[0] = _mm256_setzero_pd ();
    s[1] = _mm256_setzero_pd ();
    s[2] = _mm256_setzero_pd ();
    fmr::Phys_float* stress = (fmr::Phys_float*) &s[0];// cast to floating pt.
    //
    for (int phase=0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// warmup
      if (phase == 1) {// busy time run
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Perf_int n=0; n < (phase_n); ++n) {
        __m256d vA[3] = { vA0,vA1,vA2 };
        //
        fmr::mtrl::elastic::linear_3d_isotropic_avx<fmr::Phys_float>
          (&stress[0], lambda, mu, &vA[0]);
        //
      }// end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s =  timer.add_busy_time_now ();
      }
      stress [ 3] = 0.0;// Zero the unused lane.
      stress [ 7] = 0.0;
      stress [11] = 0.0;
      for (fmr::Local_int i=0; i<12; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-V: 3D elastic isotropic AVX per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" ,
      double (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return (is_ok[0] ? perf[0] : perf_NaN);
}
#endif
#ifdef FMR_HAS_AVX2
inline
fmr::Perf_float mtrl_iso3_avx2
 (const fmr::Perf_int test_n=1400l *Mega) {
  const auto omp_n = size_t( omp_get_max_threads ());
  std::valarray<fmr::Phys_float>
    out (omp_n);// performance if correct, nan if not
  std::valarray<fmr::Perf_float> perf (omp_n);
  std::valarray<bool>           is_ok (omp_n);
  //
  FMR_PRAGMA_OMP(omp parallel) {
    const auto omp_i = fmr::Local_int (omp_get_thread_num());
    //
    Perf_time_t timer;
    timer.start ();
    timer.add_unit (test_n);
    timer.add_flop (test_n * (4*3 +1 +8*3));// 37 FLOP
    timer.add_read (test_n * ( 12 + 2)      // 14 actual, 11 effective
      * sizeof(fmr::Phys_float));
    timer.add_save (test_n * ( 12)          // 12 actua;.  9 effective
      * sizeof(fmr::Phys_float));
    fmr::Perf_float busy_s = 0.0;
    //
    const fmr::Phys_float lambda = test_lame[0];
    const fmr::Phys_float mu     = test_lame[1];
    //
    volatile __m256d vA0 = {test_H[0], test_H[1], test_H[2], 0.0};
    volatile __m256d vA1 = {test_H[3], test_H[4], test_H[5], 0.0};
    volatile __m256d vA2 = {test_H[6], test_H[7], test_H[8], 0.0};
    //
    fmr::Phys_float stress [12] = {
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0
    };
    //
    for (int phase=0; phase < 2; ++phase) {
      fmr::Perf_int phase_n = test_n / 10;// short warmup run
      if (phase == 1) {                   // perf test run (busy time)
        phase_n = test_n;
      }
      timer.add_idle_time_now ();
      for (fmr::Perf_int n=0; n < (phase_n); ++n) {
        __m256d vA[3] = { vA0,vA1,vA2 };
        //
        fmr::mtrl::elastic::linear_3d_isotropic_avx2<fmr::Phys_float>
          (&vA[0], lambda, mu);
        //
        _mm256_storeu_pd( &stress[0], vA[0]);
        _mm256_storeu_pd( &stress[4], vA[1]);
        _mm256_storeu_pd( &stress[8], vA[2]);
      }// end kernel loop
      if (phase == 0) {// warmup
        printf ("warm: %g sec\n", double (timer.add_idle_time_now ()));
      } else {// busy time run
        busy_s = timer.add_busy_time_now ();
      }
      stress [ 3] = 0.0;// Zero the unused lanes.
      stress [ 7] = 0.0;
      stress [11] = 0.0;
      for (fmr::Local_int i=0; i<12; ++i) {
        out [omp_i]+= stress [i];
      }
    }//end phase loop
    timer.set_is_ok ( (abs(out[omp_i] - correct_value) < correct_eps) );
    //
    fprintf (stdout, "name: %s\n"          ,
      "Kernel MTR-2: 3D elastic isotropic AVX2 per-core performance");
    fprintf (stdout, "time: %g sec\n"      , double (busy_s));
    fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
    fprintf (stdout, "perf: %g FLOP/sec\n" ,
      double (timer.get_busy_flop_speed ()));
    fprintf (stdout, "mtrl: %g mtrl/sec\n" ,
      double (timer.get_busy_unit_speed ()));
    fprintf (stdout, " ans: %g ?= %g: %s\n",
      double (out[omp_i]), double (correct_value),
      timer.get_is_ok () ? "ok" : "FAIL");
    //
    is_ok [omp_i] = timer.get_is_ok ();
    perf  [omp_i] = timer.get_busy_unit_speed ();
  }//end parallel region
return (is_ok[0] ? perf[0] : perf_NaN);
}
#endif

TEST(PerfMtrlElasticLinear, TrivialTest) {
  EXPECT_EQ(1,1);
}

const auto base_dmat_speed = mtrl_iso3_dmat (500*Mega/test_div);
//
#ifdef FMR_HAS_MKL
//TODO placeholder for Intel MKL symmetric version
TEST( PerfMtrlElasticLinearIso, MKLSymmetricIsCorrectAndFaster ){
  EXPECT_GT(
    mtrl_iso3_symm (500*Mega/test_div), base_dmat_speed);
}
#endif
TEST( PerfMtrlElasticLinearIso, ScalarAIsCorrectAndFaster ){
  EXPECT_GT(
    mtrl_iso3_scalar_a (1200*Mega/test_div), base_dmat_speed);
}
TEST( PerfMtrlElasticLinearIso, ScalarBIsCorrectAndFaster ){
  EXPECT_GT(
    mtrl_iso3_scalar_b (1200*Mega/test_div), base_dmat_speed);
}
TEST( PerfMtrlElasticLinearIso, LameIsCorrectAndFaster ){
  EXPECT_GT(
    mtrl_iso3_lame (1200*Mega/test_div), base_dmat_speed);
}
#ifdef FMR_HAS_AVX
  TEST( PerfMtrlElasticLinearIso, AVXIsCorrectAndFaster ){
    EXPECT_GT(
      mtrl_iso3_avx  (1250*Mega/test_div), base_dmat_speed);
  }
#endif
#ifdef FMR_HAS_AVX2
  TEST( PerfMtrlElasticLinearIso, AVX2IsCorrectAndFaster ){
    EXPECT_GT(
      mtrl_iso3_avx2 (1400*Mega/test_div), base_dmat_speed);
  }
#endif

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

