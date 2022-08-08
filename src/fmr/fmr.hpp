#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include <string>
#include <cstdint>
#include <array>

namespace fmr {
/* prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
 *
 * sizeof (Dim_int <= Enum_int <= Local_int <= Global_int)
 * sizeof (Elid_int >= Enum_int + fmr::math::Poly + Dim_int) TODO remove
 *
 * This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
 * https://google.github.io/styleguide/cppguide.html#Aliases
*/
  // internal ints
  using   Exit_int = int           ;// system return code type
  using  Align_int = uint_fast16_t ;
  using   Team_int = uintptr_t     ;// cast-compatible with MPI_comm from mpi.h
  //
  // storable data types
  using   Char_int = char          ;// char type in strings
  using   Bulk_int = Char_int      ;// serializing type can cast to std::string
  //
  using    Dim_int = uint_fast8_t  ;// space dim., hier. depth, poly. order,...
  using   Enum_int = int           ;// default enum native type
  using  Local_int = uint32_t      ;
  using Global_int = uint64_t      ;// element ID, node ID
  //
  using   Hash_int = uint32_t      ;//TODO check CRC32, CRC64 perf (cpu,gpu)
  //
  using Perf_int   = uint_fast64_t ;// unit counters, time (ns) counters
  using Perf_float = float         ;// speed, aithmetic intensity, time (sec)
  //
  // fmr:: floating point types are defaults?
  using Geom_float = double        ;// includes non-inverted jac & det
  using Phys_float = double        ;// includes inverses of jac & det
  using Solv_float = double        ;
  using Cond_float = float         ;// Preconditioning and scaling
  using Post_float = float         ;// Post-processing
  using Plot_float = float         ;// Visualization
  //
  enum class Vals_type : Enum_int { None=0, Error, Unknown,
    Bulk, String, Dim, Enum, Local, Global, Hash,
    Count, Perf,
    Geom, Phys, Solv, Cond, Post, Plot,
    end
  };
  enum class Data_type : Enum_int { None=0, Error, Unknown,
#include "vals-enum.inc"
  end// The last item is "end" to indicate the number of enumerated values.
  };
  template <typename E>// Cast enum to number: for enum index#, size, sync.
  constexpr typename std::underlying_type<E>::type enum2val (E e) {
    // use: const auto part_type_n = fmr::enum2val (fmr::Partition::end);
    return static_cast<typename std::underlying_type<E>::type>(e);
  }
  static const std::array <Vals_type, enum2val (Data_type::end)+1>
  vals_type = {
    Vals_type::None,
    Vals_type::Error,
    Vals_type::Unknown,
#include "vals-type.inc"
    Vals_type::None
  };
#if 1
  static const std::array <std::string, enum2val (Data_type::end)+1>
  vals_name = {
    "data_none",
    "data_err",
    "data_unk",
#include "vals-name.inc"
    "END"
  };
  static const std::array <std::string, enum2val (Data_type::end)+1>
  vals_info = {
    "no data",
    "data error",
    "unknown data",
#include "vals-info.inc"
    "Data enum end marker"
  };
#endif
  //
}//end fmr:: namespace
//
namespace fmr {
  //
  using Vals_name_t = std::string;//WAS Data_id
  using File_name_t = std::string;
  // a file name is interpreted as a collection of vals or strings
  using Data_name_t = std::string;// variable (vals) or file name
 /*
  * Data_name_t: sim, part, file, directory, or other data collection path and
  * (base) name.
  * The base name, Vals_type (or vals name), and an integer index (e.g.
  * partition number or thread ID), together identify a data item (and a format
  * handler if appropriate) for its source, destination(s), and storage in
  * memory. Each identifier can map to an initial (input) source and final
  * (output) data destinations.
  */
  // built-in data sources and destinations
  static const Data_name_t log ="fmr:log" ;// default stdout from main thrd only
  static const Data_name_t out ="fmr:out" ;// default stdout from all threads
  static const Data_name_t err ="fmr:err" ;// default stderr from all threads
  static const Data_name_t in  ="fmr:in"  ;// default stdin  to   each MPI thrd
  static const Data_name_t none="fmr:none";
  static const Data_name_t null="fmr:null";// convenient synonym of fmr::none
  //
  static const Data_name_t info="fmr:info";
//TODO collides with namespace perf::  static const Data_name_t perf="fmr:perf";
  static const Data_name_t spam="fmr:spam";
  static const Data_name_t debug="fmr:debug";
  // formatters: fmr::Vals_type::Info_line, fmr::Vals_type::Text_line, ...
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
