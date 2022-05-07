#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

//#ifdef ZYC_HAS_GTEST
#include "gtest/gtest.h"
//#endif/

#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

static inline
uint zyc::hamw (uint64_t i){
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u64() (i));
#else
  return uint(__builtin_popcountll (i));
#endif
}
static inline
uint zyc::hamw (uint32_t i){
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u32() (i));
#else
  return uint(__builtin_popcount (i));
#endif
}
static inline
uint zyc::upow (uint base, uint exponent) {
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

//end ZYC_HAS_ZYC_IPP
#endif
