#include "../femera.hpp"
#include "Bank.hpp"

#include "gtest/gtest.h"

namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();
  //
  inline
  int time_bank (uint N=10, uint n=1024, uint sz=1024) {// returns total kB
    if (N<1 || n<1 || sz<1) { return 0; }
    auto time = fmr::perf::Meter <fmr::Perf_int, fmr::Perf_float>();
    auto vals = femera::data::Bank();
    const double bytes = double (n * sz * sizeof (double));
    double timez=0.0, timenz=0.0;
    for (uint v=0; v<N; v++ ) {
      time.add_idle_time_now ();
      for (uint i=0; i<n; i++) {
        vals.set ("vals_"+std::to_string (i), sz, double (v % 2));
      }
      const auto add_s = double (time.add_busy_time_now ());
      if (v > 0 && v < (N - 1)) {
        if ((v % 2)==0) {timez  += add_s;}
        else            {timenz += add_s;}
      }
      for (uint i=0; i<n; i++) {
        vals.get<float> ("vals_"+std::to_string (i));// convert to float
      }
      const auto cast_s = double (time.add_busy_time_now ());
      vals.del_all ();
      printf ("%.2e B / %.2e s = %.2e B/s of double (%u), %.2e s to float\n",
        bytes, add_s, bytes / add_s, v % 2, cast_s);
    }
    printf ("nonzero init %.1f%% slower than zero init\n",
      100.0 * (timenz / timez - 1.0));
    return int (N * bytes / 1024.0);
  }
  TEST(NewVals, Time) {
    EXPECT_EQ( time_bank (10, 256, 1024*16*4),
      (10 * 256 * 1024*16*4 * sizeof(double)) / 1024);
  }
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
  return femera::test::mini->exit (femera::test::mini->init (&argc,argv));
}
