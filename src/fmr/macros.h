#ifndef FMR_HAS_MACROS_H
#define FMR_HAS_MACROS_H

#ifndef FMR_VERBMAX
#define FMR_VERBMAX 7
#endif

#if 0
//TODO FMR_TIMELVL not used yet.
#ifndef FMR_TIMELVL
#define FMR_TIMELVL FMR_VERBMAX
#endif
#endif

#undef FMR_USE_PROC_ROOT

#define MAKESTR(s) STR(s)
#define STR(s) #s
// These help keep source code tidy.
#define MAIN master
#ifdef _OPENMP
#define FMR_HAS_OPENMP
#define FMR_PRAGMA_OMP(x) _Pragma (#x)
//
#undef FMR_BANK_LOCAL
#define FMR_VALS_LOCAL
//NOTE define only one of the two above.
// Define FMR_xxxx_LOCAL to make data arrays thread-local to each OpenMP thread.
//
#define FMR_RUNS_LOCAL
// Define FMR_RUNS_LOCAL for thread-local sim runners on each OpenMP thread.
#undef FMR_OMP_LOCAL
// Define FMR_OMP_LOCAL to make & use thread-local drivers for each OpenMP
// thread. This may be needed to avoid race conditions in OpenMP parallel
// regions. It should also help if the threads in an MPI process are in more
// than one NUMA domain.
//NOTE NUMA data locality is NOT automatic when >= 1 MPI thread/NUMA domain,
//     e.g. 5 MPI processes w/8 threads each on a 2-domain 2x20-core node.
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

#ifdef __INTEL_COMPILER
#else
#define FMR_WARN_INLINE_ON _Pragma ("GCC diagnostic warning \"-Winline\"")
#define FMR_WARN_INLINE_OFF _Pragma ("GCC diagnostic ignored \"-Winline\"")
#endif

#undef FMR_DEBUG

//end FMR_HAS_MACROS_H
#endif
