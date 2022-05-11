#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

//#ifdef ZYC_HAS_GTEST
//#include <gtest/gtest.h>
//#endif/

//#include <functional>        //bit_xor

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
template <typename T> inline constexpr
T zyc::cr_dual_elem (const T* ZYC_RESTRICT v,
  const zyc::Zsize_t row, const zyc::Zsize_t col)
noexcept {
  return ((row ^ col) == (row - col)) ? v [row - col] : T(0.0);
}
template <typename T> inline constexpr
T zyc::cr_dual_elem (const T* ZYC_RESTRICT v,
  const zyc::Zsize_t row, const zyc::Zsize_t col, const zyc::Zsize_t order)
noexcept {// for promoted or demoted Zorder access
  return (((row ^ col) == (row - col)) && ((row - col) < (Zsize_t(1) << order)))
    ? v [row - col] : T(0.0);
}

//end ZYC_HAS_ZYC_IPP
#endif
