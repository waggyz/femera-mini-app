#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include <string>
#include <cstdint>

namespace fmr {
/* prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
 *
 * sizeof (Dim_int <= Enum_int <= Local_int <= Global_int)
 * sizeof (Elid_int >= Enum_int + fmr::math::Poly + Dim_int) TODO remove
 *
 * This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
 * https://google.github.io/styleguide/cppguide.html#Aliases
*/
  using   Exit_int = int           ;// system return code type
  using   Bulk_int = char          ;// serializing type can cast to std::string
  //
  using    Dim_int = uint_fast8_t  ;// space dim., hier. depth, poly. order,...
  using   Enum_int = int           ;// default enum native type
//  using   Elid_int = uint32_t      ;//TODO remove ?
  using  Align_int = uint_fast16_t ;
  using  Local_int = uint32_t      ;
  using Global_int = uint64_t      ;// element ID, node ID
  //
  using Perf_int   = uint_fast64_t ;// unit counters, time (ns) counters
  using Perf_float = float         ;// speed, aithmetic intensity, time (sec)
  //
  using Hash_int   = uint32_t      ;//TODO chck CRC32, CRC64 perf (cpu,gpu)
  using Team_int   = uintptr_t     ;// cast-compatible with MPI_comm from mpi.h
#if 0
  //
  // fmr:: floating point types are internal defaults?//TODO dynamic float types
  using Geom_float = double        ;// includes non-inverted jac & det
  using Phys_float = double        ;// includes inverses of jac & det
  using Solv_float = double        ;
  using Cond_float = float         ;// Preconditioning and scaling
  using Post_float = float         ;// Post-processing
  using Plot_float = float         ;// Visualization
#endif
  //
  using Line_size_int = uint_fast16_t;//TODO Remove.
  //
  using Vals_name_t = std::string;//WAS Data_id
  using File_name_t = std::string;
}//end fmr:: namespace
namespace femera { namespace test {
  int early_main (int* argc, char** argv);
} }// end femera::test:: namespace

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

//end FMR_HAS_FMR_HPP
#endif
