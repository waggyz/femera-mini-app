#ifndef FMR_HAS_MATH_IPP
#define FMR_HAS_MATH_IPP

template <typename I> static inline// I : Integer type
I fmr::math::divide_ceil (const I x, const I y) {
#if 0
  return x / y + (x % y != 0);
  return 1 + ((x - 1) / y); // if x != 0
  return x / y + (x % y > 0);// should be correct for negative result, too
#else
return (x % y) ? x / y + I(1) : x / y;
#endif
}
/* https://stackoverflow.com/questions/25892665/
   performance-of-log10-function-returning-an-int */
static inline
uint fmr::math::base2_digits (uint x) {
  return x ? 32 - uint(__builtin_clz( x )) : 0;// count leading zeros
}
static inline
uint fmr::math::count_digits (uint x) {
  static const unsigned char guess [33]={
    0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
    6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
    9, 9, 9
  };
  static const uint ten_to_the [] = {
    1, 10, 100, 1000, 10000, 100000,
    1000000, 10000000, 100000000, 1000000000
  };
  uint digits = guess [base2_digits (x)];
  return digits + (x >= ten_to_the [digits]);
}
static inline
uint fmr::math::upow (uint base, uint exponent) {
  /* stackoverflow.com/questions/101439/the-most-efficient-way-to-implement
     -an-integer-based-power-function-powint-int */
  uint result = 1;
  for (;;) {
    if (exponent & 1) {result *= base;}
    exponent >>= 1;
    if (!exponent) {break;}
    base *= base;
  }
  return result;
}
static inline
uint fmr::math::nchoosek (const uint n, const uint k) {
#if 0
  return (k==0) ? 1 : fmr::math::nchoosek (n - 1, k - 1) * n / k;
#else
  if (k == 0) {return 1;}
  /* stackoverflow.com/questions/9330915/number-of-combinations-n-choose-r-in-c
     /53983114 */
  //large k: n choose k = n choose (n-k)
  if (k > n / 2) {return fmr::math::nchoosek (n, n - k);}
  uint res = 1;
  for (uint r = 1; r <= k; ++r) {
    res *= n - r + 1;
    res /= r;
  }
  return res;
#endif
}
static inline
uint fmr::math::poly_terms (
    const fmr::math::Poly ptype, const uint nvar, const uint pord) {
  if (nvar < 1) {return 0;}
  if (pord < 1) {return 1;}
  uint terms = 0;
  switch (ptype) {
    case fmr::math::Poly::Full : {
#if 0
      terms=1;
      for (uint p = 1; p<=pord+1; ++p) {terms += fmr::math::upow (p, nvar-1);}
#else
      terms = fmr::math::nchoosek (pord + nvar, nvar);
#endif
      break;}
    case math::Poly::Serendipity : {
      terms = fmr::math::poly_terms (fmr::math::Poly::Bipoly, nvar, pord);
      terms-= (pord > 1 && nvar > 1)
        ? fmr::math::poly_terms (fmr::math::Poly::Full, nvar, pord - 2) : 0;
      break;}
    case fmr::math::Poly::Bipoly :// Fall through.
    case fmr::math::Poly::Tripoly : {
      terms = fmr::math::upow (pord + 1, nvar);
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
// "Comparing floating point numbers" by Bruce Dawson
template <typename F> static inline// F : Floating-point type
bool fmr::math::are_close (const F a, const F b) {
  return std::abs(a - b) <= ( (std::abs(a) < std::abs(b)
    ? std::abs(b) : std::abs(a)) * std::numeric_limits<F>::epsilon());
}
template <typename F> static inline// F : Floating-point type
bool fmr::math::are_equal (const F a, const F b){
  return std::abs(a - b) <= ( (std::abs(a) > std::abs(b)
    ? std::abs(b) : std::abs(a)) * std::numeric_limits<F>::epsilon());
}
template <typename F> static inline// F : Floating-point type
bool fmr::math::is_less_than (const F a, const F b) {
  return (b - a) > ( (std::abs(a) < std::abs(b)
    ? std::abs(b) : std::abs(a)) * std::numeric_limits<F>::epsilon());
}
template <typename F> static inline// F : Floating-point type
bool fmr::math::is_more_than (const F a, const F b) {
  return (a - b) > ( (std::abs(a) < std::abs(b)
    ? std::abs(b) : std::abs(a)) * std::numeric_limits<F>::epsilon());
}
template <typename F> static inline// F : Floating-point type
bool fmr::math::is_greater_than (const F a, const F b) {
  return (a - b) > ( (std::abs(a) < std::abs(b)
    ? std::abs(b) : std::abs(a)) * std::numeric_limits<F>::epsilon());
}

#undef FMR_DEBUG

//end FMR_HAS_MATH_IPP
#endif
