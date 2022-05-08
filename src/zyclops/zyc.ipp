#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

//#ifdef ZYC_HAS_GTEST
#include "gtest/gtest.h"
//#endif/

//#include <functional>        //bit_xor

#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

static inline
uint zyc::hamw (const uint64_t i) {
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u64() (i));
#else
  return uint(__builtin_popcountll (i));
#endif
}
static inline
uint zyc::hamw (const uint32_t i) {
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
static inline
uint zyc::dual_nz (const uint32_t row, const uint32_t col) {// returns 0 or 1
  return (row ^ col) == (row - col);
}
static inline
int zyc::dual_ix (const int row, const int col) {
  //returns index of value, or -1 for a zero entry
#if 0
  return ((row ^ col) + (row - col));//TODO try to simplify below
//  return ((row - col) - (row ^ col)) - col+row;
//  return (row ^ col) - (row + col);
#   else
  return (row ^ col) == (row - col) ? row - col : -1;// this works
#endif
}
static inline
int zyc::dual_ix (const int row, const int col, const int array_order) {
  // this one is safe for operations with different order operands
  const auto ix = row - col;
  return (ix < (1<<array_order)) && ((row ^ col) == ix) ? ix : -1;
}
// Transpose versions
static inline
uint zyc::dual_tnz (const uint32_t col, const uint32_t row) {// returns 0 or 1
  return (row ^ col) == (row - col);
}
static inline
int zyc::dual_tix (const int col, const int row) {
  //returns index of value, or -1 for a zero entry
  return (row ^ col) == (row - col) ? row - col : -1;// this works
}
static inline
int zyc::dual_tix (const int col, const int row, const int array_order) {
  // this one is safe for operations with different order operands
  const auto ix = row - col;
  return (ix < (1<<array_order)) && ((row ^ col) == ix) ? ix : -1;
}
//end ZYC_HAS_ZYC_IPP
#endif
