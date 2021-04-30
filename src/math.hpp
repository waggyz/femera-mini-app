#ifndef FMR_HAS_MATH_HPP
#define FMR_HAS_MATH_HPP

#include <tuple>
#include <valarray>

#ifndef FMR_RESTRICT
#define FMR_RESTRICT __restrict
#endif
#define FMR_ARRAY_PTR auto* FMR_RESTRICT
#define FMR_CONST_PTR const auto* FMR_RESTRICT


namespace fmr {namespace data {
  enum class Precision   : int {Unknown=0, Bit = 1,//TODO unsigned Dim_int?
//    Bit        =  1, Int_hexadec  = -4, Int_byte     =-8,
    Nat_short  = -2, Nat          = -4, Nat_long     =-8,// unsigned
    Int_short  =  2, Int          =  4, Int_long     = 8,
    Float_half = 16, Float_single = 32, Float_double = 64,
    Fixed_half =-16, Fixed_single =-32, Fixed_double =-64
  };
  enum class Layout      : uint8_t { Unknown =0,//TODO Dim_int?
    Vector=1, Block=2, Native=3 // Native is for built-in complex type
  };
} }// end fmr::data namespace
namespace fmr {namespace math {
  enum class Poly : uint8_t {None=0, Error, Unknown,
    Full, Serendipity, Bipoly, Tripoly, Pyramid, Prism,
  end};
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
  typedef int8_t Zorder_int;
#ifdef FMR_HAS_ZYCLOPS
  enum class Algebra     : uint8_t {Unknown =0,
    Real=zyc::Real, Complex=zyc::Complex, Dual=zyc::Dual, Split=zyc::Split,
    Quat=zyc::Quat,     OTI=zyc::OTI    , User=zyc::User,
    Int, Nat
  };
#else
  enum class Algebra     : uint8_t {Unknown =0,
    Real, Complex, Dual, Split, Quat, OTI, User,
    Int,// signed
    Nat // unsigned
  };
#endif
#if 0
  enum class Complex_form:int8_t{ Unknown =0,
    Vector, CR, CRT,     // CRT: transposed
    CR_sparse, CRT_sparse// zero terms removed
    //CR_lower, CR_upper //TODO triangular storage?
    //...for Zomplex struct
    Algebra          algebra = Algebra::Real;
    Zorder_int         order = 0;
    Complex_form        form = Complex_form::Vector;
    fmr::data::Layout layout = fmr::data::Layout::Native;
  };
#endif
  struct Zomplex {
    Algebra          algebra = Algebra::Real;
    Zorder_int         order = 0;
    fmr::data::Layout layout = fmr::data::Layout::Native;
#if 0
    bool       is_cr_matrix  = false;// the rest for is_cr_matrix == true
    bool       is_transposed = false;
    bool       is_compressed = false;// true: zero terms removed
    bool            is_upper = false;
    bool            is_lower = false;
#endif
    // constructors
    Zomplex () {}
    Zomplex (Algebra z, Zorder_int p) : algebra (z), order(p) {
      if (p!=0) {layout = fmr::data::Layout::Block;}
    }
    Zomplex (Algebra z, Zorder_int p, fmr::data::Layout l) :
      algebra (z), order(p), layout(l) {}
  };
  const static Zomplex Real
    = Zomplex (Algebra::Real, 0, fmr::data::Layout::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, fmr::data::Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Quat, 2, fmr::data::Layout::Vector);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, fmr::data::Layout::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, fmr::data::Layout::Native);
} }// end fmr::math namespace

namespace fmr {namespace math {
  template <typename I> static inline I divide_ceil (const I x, const I y);
  // I : Integer type
  static inline unsigned int base2_digits (unsigned int x);
  static inline unsigned int count_digits (unsigned int x);
  static inline uint upow (uint base, uint exp);
  static inline uint nchoosek (uint n, uint k);
  static inline uint poly_terms (Poly ptype, uint nvars, uint pord);
} }// end fmr::math namespace

template <typename I> static inline// I : Integer type
I fmr::math::divide_ceil (const I x, const I y){
#if 0
  return x / y + (x % y != 0);
  return 1 + ((x - 1) / y); // if x != 0
  return x / y + (x % y > 0);// should be correct for negative result, too
#else
return (x % y) ? x / y + I(1) : x / y;
#endif
}
// https://stackoverflow.com/questions/25892665/
// performance-of-log10-function-returning-an-int
static inline unsigned int fmr::math::base2_digits (unsigned int x){
  return x ? 32 - __builtin_clz( x ) : 0;// count leading zeros
}
static inline unsigned int fmr::math::count_digits (unsigned int x){
  static const unsigned char guess [33]={
    0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
    6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
    9, 9, 9
  };
  static const unsigned int ten_to_the [] = {
    1, 10, 100, 1000, 10000, 100000,
    1000000, 10000000, 100000000, 1000000000
  };
  unsigned int digits = guess [base2_digits (x)];
  return digits + (x >= ten_to_the [digits]);
}
static inline uint fmr::math::upow (uint base, uint exponent) {
  //https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
  uint result = 1;
  for (;;) {
    if (exponent & 1) {result *= base;}
    exponent >>= 1;
    if (!exponent) {break;}
    base *= base;
  }
  return result;
}
static inline uint fmr::math::nchoosek (const uint n, const uint k) {
#if 0
  return (k==0) ? 1 : fmr::math::nchoosek (n - 1, k - 1) * n / k;
#else
  if (k == 0) {return 1;}
  //https://stackoverflow.com/questions/9330915/number-of-combinations-n-choose-r-in-c/53983114
  //large k: n choose k = n choose (n-k)
  if (k > n / 2) {return fmr::math::nchoosek (n, n - k);}
  uint res = 1;//TODO unsigned long ?
  for (uint r = 1; r <= k; ++r) {
    res *= n - r + 1;
    res /= r;
  }
  return res;
#endif
}
static inline uint fmr::math::poly_terms (
    const fmr::math::Poly ptype, const uint nvar, const uint pord) {
  if (nvar < 1) {return 0;}
  if (pord < 1) {return 1;}
  uint terms = 0;
  switch (ptype) {
    case fmr::math::Poly::Full : {
#if 0
      terms=1;
      for (uint p = 1; p<=pord+1; p++) {terms += fmr::math::upow (p, nvar-1);}
#else
      terms = fmr::math::nchoosek (pord + nvar, nvar);
#endif
      break;}
    case math::Poly::Serendipity : {
      terms = fmr::math::poly_terms (fmr::math::Poly::Bipoly, nvar, pord);
      terms-= (pord > 1 && nvar > 1)
        ? fmr::math::poly_terms (fmr::math::Poly::Full, nvar, pord-2) : 0;
      break;}
    case fmr::math::Poly::Bipoly :// Fall through.
    case fmr::math::Poly::Tripoly : {
      terms = fmr::math::upow (pord+1, nvar);
      break;}
    case fmr::math::Poly::Pyramid : {
      switch (pord) {
        case 1: {terms = 5; break;}
        case 2: {terms = 5 + 8; break;}
        case 3: {terms = 5 + 8*2; break;}
        default:{}
      }
      break;}
    case fmr::math::Poly::Prism : {
      switch (pord) {
        case 1: {terms = 6; break;}
        case 2: {terms = 6 + 9; break;}
        case 3: {terms = 6 + 9*2; break;}
        default:{}
      }
      break;}
    default : {}// do nothing
  }
  return terms;
}
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

//end FMR_HAS_MATH_HPP
#endif
