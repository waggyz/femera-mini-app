#ifndef HAS_DEVS_H
#define HAS_DEVS_H
/** */
#include "Flog.hpp"

//#include "Geom.hpp"
#include "Sims.hpp"
#include "Phys.hpp"
#include "Data.hpp"

#include "Main.hpp"

#if 0 // set by compiling with -DFMR_EXPOSE_LEAVES
// Change includes in Plug.hpp and proc.hpp to expose derived class leaf nodes.
#define FMR_EXPOSE_LEAVES
#endif

#ifndef FMR_VERBMAX
#define FMR_VERBMAX 7
#endif

#ifndef FMR_TIMELVL
#define FMR_TIMELVL FMR_VERBMAX
#endif

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

#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include main.hpp")
//end HAS_DEVS_H
#endif
