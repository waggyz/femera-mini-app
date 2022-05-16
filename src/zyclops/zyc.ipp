#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

static inline
uint zyc::upow (uint base, uint exponent)
noexcept {
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
static inline constexpr
uint zyc::hamw (const uint64_t i)
noexcept {
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u64() (i));
#else
  return uint(__builtin_popcountll (i));
#endif
}
static inline constexpr
uint zyc::hamw (const uint32_t i)
noexcept {
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u32() (i));
#else
  return uint(__builtin_popcount (i));
#endif
}
static inline constexpr
bool zyc::is_dual_nz (Zarray_int row, Zarray_int col)
noexcept{
  return (row ^ col) == (row - col);
}
template <typename T> static inline constexpr
void zyc::dual_mult_aos
  (T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zorder_int zorder, const std::size_t n=1)
noexcept {
  const zyc::Zarray_int zsz = zyc::Zarray_int (1) << zorder;
  for (std::size_t k=0; k<n; k++) {
    auto ak =& ((&a)[zsz * k]);
    auto bk =& ((&b)[zsz * k]);
    auto ck =& ((&c)[zsz * k]);
    for (zyc::Zarray_int j=0; j<zsz; j++) {// permuted loop, transposed calc
      for (zyc::Zarray_int i=0; i<zsz; i++) {
#if 0
      ck[i] += zyc::cr_dual_elem (a, i, j) * (&b)[j];
#endif
#if 0
      ck [j] += zyc::cr_dual_elem (ak[0], j, i) * bk [i];
#endif
      ck [j] += zyc::cr_dual_mult_elem (ak[0], bk[0], j, i);
} } } }
# if 0
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,//DONE change all ptr to reference
  const zyc::Zarray_int row, const zyc::Zarray_int col)
/*
 * The difference between pass-by-reference and pass-by-pointer is that pointers
 * can be NULL or reassigned whereas references cannot. Use pass-by-pointer
 * if NULL is a valid parameter value or if you want to reassign the pointer.
 * Otherwise, use constant or non-constant references to pass arguments.
 *
 * https://www.ibm.com/docs/en/zos/2.4.0?topic=calls-pass-by-reference-c-only
 */
noexcept {
  return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
}
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T* ZYC_RESTRICT v,
  const zyc::Zarray_int row, const zyc::Zarray_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? v [row - col] : T(0.0);
}
#   endif
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,
  const zyc::Zarray_int row, const zyc::Zarray_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
}
static inline constexpr
bool zyc::is_dual_nz (Zarray_int row, Zarray_int col, Zarray_int p)
noexcept{
  return ((row ^ col) == (row - col)) && ((row - col) < (Zarray_int(1) << p));
}
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v, const zyc::Zarray_int row,
  const zyc::Zarray_int col, const zyc::Zarray_int p)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zarray_int(1) << p))
    ? (&v)[row - col] : T(0.0);
}
#if 1
template <typename T> static inline constexpr
T zyc::cr_dual_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zarray_int row, const zyc::Zarray_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&a)[col] * (&b)[row - col] : T(0.0);
}
#endif
#if 0
template <typename T> static inline constexpr
void zyc::dual_mult_aos (T* ZYC_RESTRICT a,
  const T* ZYC_RESTRICT b, const T* ZYC_RESTRICT c, const zyc::Zarray_int order)
noexcept {
  const zyc::Zarray_int zsz = zyc::Zarray_int (1) << order;
  for (zyc::Zarray_int i=0; i<zsz; i++) {
    for (zyc::Zarray_int j=0; j<zsz; j++) {
      a [j] += zyc::cr_dual_elem (b, i, j) * c [j];
} } }
#endif
//end ZYC_HAS_ZYC_IPP
#endif
