#ifndef FMR_HAS_SMAT_PERF_HPP
#define FMR_HAS_SMAT_PERF_HPP
// Small matrix utilities: 1x1, 2x2, 3x3, 4x4

#include "math.hpp"

#include <immintrin.h>

namespace femera { namespace test { namespace smat {

  fmr::Perf_float time_m2x2 (
    int test_case=0, uint n=1024*1024/8/sizeof(double), uint reps=512);
  fmr::Perf_float time_m3x3 (
    int test_case=0, uint n=1024*1024/16/sizeof(double), uint reps=512);
  fmr::Perf_float time_m4x4 (
    int test_case=0, uint n=1024*1024/32/sizeof(double), uint reps=512);

} } } // namespace femera::test::smat

#include "smat.ipp"

//end FMR_HAS_SMAT_PERF_HPP
#endif
