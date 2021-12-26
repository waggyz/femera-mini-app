#ifndef FMR_HAS_FEMERA_DETAIL_HPP
#define FMR_HAS_FEMERA_DETAIL_HPP



#ifndef FMR_VERBMAX
#define FMR_VERBMAX 7
#endif

#ifndef FMR_TIMELVL
#define FMR_TIMELVL FMR_VERBMAX
#endif

namespace femera { namespace test {
  int early_main (int* argc, char** argv);
} }// end femera::test:: namespace

// These help keep source code tidy.
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

#if 0 //def FMR_DO_WARN_EXTRA_INCLUDE
#define FMR_WARN_EXTRA_INCLUDE(x) _Pragma( message(#x) )
#else
#define FMR_WARN_EXTRA_INCLUDE(x) // Warn extra include disabled.
#endif

#undef FMR_DEBUG
//end FMR_HAS_FEMERA_DETAIL_HPP
#endif
