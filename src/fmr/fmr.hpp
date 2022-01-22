#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

//#include "../femera/femera.hpp"

#include <cstdint>

namespace fmr {
/* NOTE prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
 *
 * This "using" syntax is preferred to "using" in the Google C++ Style Guide:
 * https://google.github.io/styleguide/cppguide.html#Aliases
 *
 * sizeof (Dim_int <= Enum_int <= Local_int < Global_int)
 * sizeof (Elid_int >= Enum_int + fmr::math::Poly + Dim_int)
*/
  using   Exit_int = int           ;// system return code
  //
  using    Dim_int = uint_fast8_t  ;// spatial dim., hier. depth, poly. order,
  using   Enum_int = uint_fast16_t ;
  using  Local_int = uint32_t      ;
  using   Elid_int = uint32_t      ;//(See above.)
  using Global_int = uint64_t      ;//TODO signed?
  //
  using   Perf_int = uint_fast64_t ;// unit counters, time (ns) counters
  using Perf_float = float         ;
  //
  using Geom_float = double        ;//TODO try float.
  using Phys_float = double        ;
  using Solv_float = double        ;
  using Cond_float = float         ;// Preconditioning and scaling
  using Post_float = float         ;// Post-processing
  using Plot_float = float         ;// Visualization
}//end fmr:: namespace
namespace femera { namespace test {
  int early_main (int* argc, char** argv);
} }// end femera::test:: namespace

#ifndef FMR_VERBMAX
#define FMR_VERBMAX 7
#endif

#ifndef FMR_TIMELVL
#define FMR_TIMELVL FMR_VERBMAX
#endif

// These help keep source code tidy.
#define MAIN master
#ifdef _OPENMP
#define FMR_PRAGMA_OMP(x) _Pragma (#x)
#else
#define FMR_PRAGMA_OMP(x) // pragma omp not supported
#endif

#ifdef FMR_HAS_PRAGMA_SIMD
#define FMR_PRAGMA_OMP_SIMD _Pragma ("omp simd")
#else
#define FMR_PRAGMA_OMP_SIMD // pragma omp simd not supported
#endif

#ifdef __INTEL_COMPILER
// pragma vector (un)aligned supported by intel compiler
#define FMR_PRAGMA_VECTOR(x) _Pragma (#x)
#else
#define FMR_PRAGMA_VECTOR(x) // pragma vector ... not supported
#endif

#undef FMR_DEBUG
//end FMR_HAS_FMR_HPP
#endif
