#ifndef FMR_HAS_MATH_HPP
#define FMR_HAS_MATH_HPP

#define FMR_ALIGN_INTS alignof(std::size_t)
#define FMR_ALIGN_VALS alignof(__m256d)

#ifdef FMR_HAS_ZYCLOPS
#define ZYC_ALIGN FMR_ALIGN_VALS
#include "../zyclops/zyclops.hpp"
#endif

#include <tuple>
#include <valarray>
#include <map>
#include <immintrin.h>

/* CRC-32C (iSCSI) polynomial in reversed bit order. */
#define FMR_CRC32_POLY 0x82f63b78

/* CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order. */
/* #define FMR_CRC32_POLY 0xedb88320 */

 // ECMA-182
#define FMR_CRC64_POLY 0xc96c5795d7870f42L

#ifndef FMR_RESTRICT
#define FMR_RESTRICT ZYC_RESTRICT
#endif
#define FMR_ARRAY_PTR ZYC_ARRAY_PTR
#define FMR_CONST_PTR ZYC_CONST_PTR
#define FMR_ALIGN_PTR ZYC_ALIGN_PTR

namespace fmr { namespace math {
  enum class Poly : int8_t {None=0, Error, Unknown,
    Full, Serendipity, Bipoly, Tripoly, Pyramid, Prism,
    end };
  static const std::map<Poly,std::pair<std::string,std::string>>
  poly_letter_name {
    {Poly::        None,std::make_pair("-","no polynomial")},//TODO makes sense?
    {Poly::       Error,std::make_pair("!","polynomial error")},
    {Poly::     Unknown,std::make_pair("?","unknown polynomial")},
    {Poly::        Full,std::make_pair("P","Full Taylor polynomial")},
    {Poly:: Serendipity,std::make_pair("S","Serendipity")},
    {Poly::      Bipoly,std::make_pair("B","Bipolynomial")},
    {Poly::     Tripoly,std::make_pair("T","Tripolynomial")},
    {Poly::     Pyramid,std::make_pair("Y","Pyramid polynomial")},
    {Poly::       Prism,std::make_pair("M","Prism polynomial")},
    {Poly::         end,std::make_pair("*","Poly enum end marker")}
  };
} }// end fmr::math namespace

namespace fmr { namespace math {
  template <typename I> static inline
  I divide_ceil (const I x, const I y);
  // I : Integer type
  static inline
  uint base2_digits (uint x);
  static inline
  uint count_digits (uint x);
  static inline
  uint upow (uint base, uint exp);
  static inline
  uint nchoosek (uint n, uint k);
  static inline
  uint poly_terms (Poly ptype, uint nvars, uint pord);
  template <typename F> static inline
  bool are_close (const F a, const F b);
  template <typename F> static inline
  bool are_equal (const F a, const F b);
  template <typename F> static inline
  bool is_less_than (const F a, const F b);
  template <typename F> static inline
  bool is_more_than (const F a, const F b);
  template <typename F> static inline
  bool is_greater_than (const F a, const F b);
} }// end fmr::math namespace

#if 0
namespace fmr {namespace data {
  enum class Do :int{ Error=-4, New=-3, Check=-2, Find=-1,
    Read=0, Write=1, Modify=2, Close=3, Free=4, Default=5
  };// or data::Go:: data::Task:: data::Get:: data::Try:: data::Access::
  // or data::Set:: or data::Set:: or data::For::, data::As:: ?
  struct State {// Validity
    bool has_error   = false;// !is_ok
    bool can_read    = false;// !is_write_only
    bool can_write   = false;// !is_read_only, can_modify
    bool was_found   = false;// !is_new, is_stored, is_on_disk
    bool was_checked = false;// !is_unknown, has_been_checked
    bool was_read    = false;// !is_freed, is_loaded, is_in_memory
    bool was_changed = false;// !is_same_on_disk, is_different_from_stored
#if 0
    bool is_open     = false;// !is_closed
    bool is_virtual  = false;// !must_be_read, can_be_computed
    bool is_default  = false;// !(default_data==Default::None);
    bool do_save     = false;// do_save_before_exit (if is_modified)
#endif
  };
} }//end fmr::data namespace
#endif

#include "math.ipp"

//end FMR_HAS_MATH_HPP
#endif
