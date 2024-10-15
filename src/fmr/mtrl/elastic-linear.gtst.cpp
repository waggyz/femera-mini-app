#include "elastic-linear.hpp"
#include "../perf/Meter.hpp"

#include <gtest/gtest.h>


#include <valarray>
#include <immintrin.h>
#ifdef FMR_HAS_OPENMP
#include <omp.h>
#endif

#define TMP_HAS_AVX
#define TMP_HAS_AVX2
//FIXME Need macros defined: FMR_HAS_AVX and FMR_HAS_AVX2

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
  const fmr::Perf_int Mega = fmr::Perf_int (1000000l);
  using Perf_time_t = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>;
  //
  //NOTE Intel i7-12800H floating point math: 62.1 GOps/sec
  // https://nanoreview.net/en/cpu/intel-core-i7-12800h
  //
  // https://www.intel.com/content/dam/support/us/en/documents/processors/APP-for-Intel-Core-Processors.pdf
  //i7-12800H 537.6 Gflop/s
  //
  inline
  double mtrl_iso3_base (const fmr::Perf_int test_n=500l *Mega/1) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
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
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
      };
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
          //
          fmr::mtrl::elas_dmat_base<fmr::Phys_float>
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
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );//TODO use type information for eps.
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-D: 3D isotropic D-matrix baseline");
      fprintf (stdout, "time: %g sec\n"      , double (busy_s));
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
  return (is_ok[0] ? perf[0] : -1.0);
  }
  inline
  fmr::Phys_float mtrl_iso3_lame (const fmr::Perf_int test_n=1200l *Mega) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
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
      timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));// returns true stress tensor.
      fmr::Perf_float busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
      };
      fmr::Phys_float     HT [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      fmr::Phys_float stress [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      //
      for (int phase =0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase == 1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Local_int n=0; n < phase_n; ++n) {
          //
          fmr::mtrl::elas_iso_lame<fmr::Phys_float>
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
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );//TODO use type information for eps.
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-L: 3D isotropic Lame formula");
      fprintf (stdout, "time: %g sec\n"      , double (busy_s));
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
  return (is_ok[0] ? perf[0] : -1.0);
  }
  inline
  fmr::Phys_float mtrl_iso3_scalar (const fmr::Perf_int test_n=1800l *Mega) {// about 10 sec
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
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
      timer.add_save (test_n * ( 9)     * sizeof(fmr::Phys_float));// returns true stress tensor.
      fmr::Perf_float busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu = 0.5;
      const fmr::Phys_float c1 = lambda + 2.0*mu;
      const fmr::Phys_float c2 = lambda;
      const fmr::Phys_float c3 = mu;
      //
      volatile fmr::Phys_float H [9] = {
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
      };
      fmr::Phys_float stress       [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      fmr::Phys_float stress_voigt [9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      //
      for (int phase=0; phase < 2; ++phase) {
        fmr::Perf_int phase_n = test_n / 10;// warmup
        if (phase == 1) {// busy time run
          phase_n = test_n;
        }
        timer.add_idle_time_now ();
        for (fmr::Perf_int n=0; n < (phase_n); ++n) {
          //
          fmr::mtrl::elas_iso_scalar<fmr::Phys_float>
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
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );//TODO use type information for eps.
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-S: 3D isotropic minimum scalar");
      fprintf (stdout, "time: %g sec\n"      , double (busy_s));
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
  return (is_ok[0] ? perf[0] : -1.0);
  }
#ifdef TMP_HAS_AVX
  inline
  fmr::Phys_float mtrl_iso3_avx (const fmr::Perf_int test_n=1250l *Mega) {
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      Perf_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * ( 3 + 12 + 6));// 21 FLOP
      timer.add_read (test_n * ( 12 + 2) * sizeof(fmr::Phys_float));// 14 actual, 11 effective
      timer.add_save (test_n * ( 12)     * sizeof(fmr::Phys_float));// 12 actua;.  9 effective
      fmr::Perf_float busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu     = 0.5;
      //
      fmr::Phys_float H0 = 1.0, H1 = 0.0, H2 = 0.0;
      fmr::Phys_float H3 = 0.0, H4 = 0.0, H5 = 0.0;
      fmr::Phys_float H6 = 0.0, H7 = 0.0, H8 = 0.0;
      //
      volatile __m256d vA0 = {H0, H1, H2, H3};
      volatile __m256d vA1 = {H4, H5, H6, H7};
      volatile __m256d vA2 = {H8,0.0,0.0,0.0};
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
          __m256d vA[3] = { vA0,vA1,vA2 };
          //
          fmr::mtrl::elas_iso_avx<fmr::Phys_float>
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
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );//TODO use type information for eps.
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-V: 3D isotropic AVX");
      fprintf (stdout, "time: %g sec\n"      , double (busy_s));
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
  return (is_ok[0] ? perf[0] : -1.0);
  }
#endif
#ifdef TMP_HAS_AVX2
  inline
  fmr::Phys_float mtrl_iso3_avx2 (const fmr::Perf_int test_n=1400l *Mega) {
    const auto omp_n = size_t( omp_get_max_threads ());
    std::valarray<fmr::Phys_float>  out (omp_n);// returns performance if correct, -1.0 if not
    std::valarray<fmr::Phys_float> perf (omp_n);
    std::valarray<bool>           is_ok (omp_n);
    //
    FMR_PRAGMA_OMP(omp parallel) {
      const auto omp_i = fmr::Local_int (omp_get_thread_num());
      //
      Perf_time_t timer;
      timer.start ();
      timer.add_unit (test_n);
      timer.add_flop (test_n * (4*3 +1 +8*3));// 37 FLOP
      timer.add_read (test_n * ( 12 + 2) * sizeof(fmr::Phys_float));// 14 actual, 11 effective
      timer.add_save (test_n * ( 12)     * sizeof(fmr::Phys_float));// 12 actua;.  9 effective
      fmr::Perf_float busy_s = 0.0;
      //
      const fmr::Phys_float lambda = 0.5;
      const fmr::Phys_float mu     = 0.5;
      //
      fmr::Phys_float H0 = 1.0, H1 = 0.0, H2 = 0.0;
      fmr::Phys_float H3 = 0.0, H4 = 0.0, H5 = 0.0;
      fmr::Phys_float H6 = 0.0, H7 = 0.0, H8 = 0.0;
      //
      volatile __m256d vA0 = {H0, H1, H2, H3};
      volatile __m256d vA1 = {H4, H5, H6, H7};
      volatile __m256d vA2 = {H8,0.0,0.0,0.0};
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
          __m256d vA[3] = { vA0,vA1,vA2 };
          //
          fmr::mtrl::elas_iso_avx2<fmr::Phys_float>
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
      timer.set_is_ok ( (abs(out[0] - correct_value) < 1e-10) );//TODO use type information for eps.
      //
      fprintf (stdout, "name: %s\n"          ,"Kernel MTR-V2: 3D isotropic AVX2");
      fprintf (stdout, "time: %g sec\n"      , double (busy_s));
      fprintf (stdout, "  AI: %g FLOP/byte\n", double (timer.get_ai ()));
      fprintf (stdout, "perf: %g FLOP/sec\n" , double (timer.get_busy_flop_speed ()));
      fprintf (stdout, "mtrl: %g mtrl/sec\n" , double (timer.get_busy_unit_speed ()));
      //
      is_ok [omp_i] = timer.get_is_ok ();
      perf  [omp_i] = timer.get_busy_unit_speed ();
    }//end parallel region
  return (is_ok[0] ? perf[0] : -1.0);
  }
#endif

TEST(ElasticIsotropic, TrivialTest) {
  EXPECT_EQ(1,1);
}
fmr::Perf_int test_div = 1000;// used to reduce the 10-sec tests for rapid development.
//TODO add another function parameter to run once for correctness, multiple for performance?
#if 1
  TEST( PerfMtrlIsoLame, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_lame (1200*Mega/test_div), mtrl_iso3_base (500*Mega/test_div) );
  }
#endif
#if 1
  TEST( PerfMtrlIsoScalar, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_scalar (1800*Mega/test_div), mtrl_iso3_base (500*Mega/test_div) );
  }
#endif
#ifdef TMP_HAS_AVX
  TEST( PerfMtrlIsoAVX, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_avx (1250*Mega/test_div), mtrl_iso3_base (500*Mega/test_div) );
  }
#endif
#ifdef TMP_HAS_AVX2
  TEST( PerfMtrlIsoAVX2, IsCorrectAndFaster ){
    EXPECT_GT( mtrl_iso3_avx2 (1400*Mega/test_div), mtrl_iso3_base (500*Mega/test_div) );
  }
#endif

#undef TMP_HAS_AVX
#undef TMP_HAS_AVX2


int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

