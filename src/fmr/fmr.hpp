#ifndef FMR_HAS_FMR_HPP
#define FMR_HAS_FMR_HPP

#include "macros.h"

#include <string>
#include <cstdint>
#include <array>

namespace fmr {
/* prefer e.g. int_fast8_t, uint_least8_t, ...,  uintmax_t, uintptr_t
 *
 * This "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
 * https://google.github.io/styleguide/cppguide.html#Aliases
 */
  // internal ints
  using   Exit_int = int           ;// system return code type
  using   Team_int = uintptr_t     ;// cast-compatible with MPI_comm from mpi.h
  using  Align_int = uint_fast16_t ;// in-memory data alignment (bytes)
  //
  // storable data types
  using   Char_int = char          ;// native char type in strings
  using   Bulk_int = Char_int      ;// serializing type can cast to std::string
  //
  // sizeof: Dim_int <= Enum_int <= Local_int <= Global_int
  using    Dim_int = uint_fast8_t  ;// space dim., hier. depth, poly. order,...
  using   Enum_int = int           ;// default enum native type
  using  Local_int = FMR_LOCAL_INT ;// local partition indices
  using Global_int = FMR_GLOBAL_INT;// element ID, node ID
  //
  using   Hash_int = uint32_t      ;//TODO check CRC32, CRC64 perf (cpu,gpu)
  //
  using Perf_int   = uint_fast64_t ;// unit counters, time (ns) counters
  using Perf_float = float         ;// speed, arithmetic intensity, time (sec)
  //
  //NOTE fmr:: floating point types are defaults?
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
    end// The last item (end) is for indicating the number of enumerated values.
  };
  enum class Data_type : Enum_int { None=0, Error, Unknown,
#include "data-type-enum.inc"
    end
  };
  template <typename E> constexpr typename
  std::underlying_type<E>::type enum2val (E e) {/* Cast enum to underlying type
    * to get the last enum index number for size, sync., etc., use, e.g.,
    * const auto data_type_n = fmr::enum2val (fmr::Data_type::end);
    */
    return static_cast <typename std::underlying_type<E>::type> (e);
  }
  static const std::array <Vals_type, enum2val (Data_type::end)+1>
  vals_type = {
    Vals_type::None,
    Vals_type::Error,
    Vals_type::Unknown,
#include "data-vals-type.inc"
    Vals_type::None
  };
#if 1
  static const std::array <std::string, enum2val (Data_type::end)+1>
  vals_name = {
    "data_none",
    "data_err",
    "data_unk",
#include "data-type-name.inc"
    "END"
  };
  static const std::array <std::string, enum2val (Data_type::end)+1>
  vals_info = {
    "no data",
    "data error",
    "unknown data",
#include "data-type-info.inc"
    "Data_type enum end marker"
  };
#endif
  //
}//end fmr:: namespace
//
namespace fmr {
  //
  using Vals_name_t = std::string;
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
  * (output) data destination.
  */
  // built-in data sources and destinations
  static const Data_name_t   log ="fmr:log" ;// default stdout main thread only
  static const Data_name_t   out ="fmr:out" ;// default stdout from all threads
  static const Data_name_t   err ="fmr:err" ;// default stderr from all threads
  static const Data_name_t    in ="fmr:in"  ;// default stdin to each MPI thread
  static const Data_name_t  none ="fmr:none";
  static const Data_name_t  null ="fmr:null";// convenient synonym of fmr::none
  //
  static const Data_name_t  info ="fmr:info";
  static const Data_name_t  plog ="fmr:perf";// performance log
  static const Data_name_t  spam ="fmr:spam";
  static const Data_name_t debug ="fmr:debug";
#if 0
  static const Data_name_t perf="fmr:perf";//NOTE collides with namespace perf::
#endif
  // formats: fmr::Vals_type::Info_line, fmr::Vals_type::Text_line, ...
}//end fmr:: namespace
//
namespace femera { namespace test {
  int early_main (int* argc, char** argv);
} }// end femera::test:: namespace

//end FMR_HAS_FMR_HPP
#endif
