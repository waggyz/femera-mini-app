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
bool zyc::is_dual_nz (Zix_int row, Zix_int col)
noexcept{
  return (row ^ col) == (row - col);
}
# if 0
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,//DONE change all ptr to reference
  const zyc::Zix_int row, const zyc::Zix_int col)
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
  const zyc::Zix_int row, const zyc::Zix_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? v [row - col] : T(0.0);
}
#   endif
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,
  const zyc::Zix_int row, const zyc::Zix_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
}
static inline constexpr
bool zyc::is_dual_nz (Zix_int row, Zix_int col, Zix_int order)
noexcept{
  return ((row ^ col) == (row - col)) && ((row - col) < (Zix_int(1) << order));
}
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,
  const zyc::Zix_int row, const zyc::Zix_int col, const zyc::Zix_int order)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zix_int(1) << order))
    ? (&v)[row - col] : T(0.0);
}
#if 0
template <typename T> static inline constexpr
void zyc::dual_mult_aos (T* ZYC_RESTRICT a,
  const T* ZYC_RESTRICT b, const T* ZYC_RESTRICT c, const zyc::Zix_int order)
noexcept {
  const zyc::Zix_int zsz = zyc::Zix_int (1) << order;
  for (zyc::Zix_int i=0; i<zsz; i++) {
    for (zyc::Zix_int j=0; j<zsz; j++) {
      a [j] += zyc::cr_dual_elem (b, i, j) * c [j];
} } }
#endif
//end ZYC_HAS_ZYC_IPP
#endif