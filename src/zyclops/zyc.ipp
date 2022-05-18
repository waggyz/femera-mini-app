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
    if (exponent & 1){ result *= base; }
    exponent >>= 1;
    if (!exponent){ break; }
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
bool zyc::cr_dual_nz (const Zarray_int row, const Zarray_int col)
noexcept{
  return (row ^ col) == (row - col);
}
template <typename T> static inline
T* zyc::aos_dual_mult
  (T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zorder_int zorder, const std::size_t n=1)
noexcept {
  if (n>0) {
    const auto zsz = std::size_t (1) << zorder;
    const auto sz = zsz * n;
    for (std::size_t k=0; k<sz; k+=zsz) {
      for (zyc::Zarray_int j=0; j<zsz; j++) {// permuted loop, transposed calc
        for (zyc::Zarray_int i=0; i<=j; i++) {
          (&c)[k + j] += zyc::cr_dual_mult_elem ((&a)[k], (&b)[k], j, i);
  } } } }
  return &c;
}
#if 1
template <typename T> static inline
T* zyc::soa_dual_mult
  (T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zorder_int zorder, const std::size_t n=1)
noexcept {
#if 0
  if (n>0 && zorder>=0) {
    const auto zsz = std::size_t (1) << zorder;
    for (zyc::Zarray_int j=0; j<zsz; j++) {// permuted loop
      for (zyc::Zarray_int i=j; i<zsz; i++) {
        if (zyc::cr_dual_nz (i, j)) {
          for (std::size_t k=0; k<n; k++) {
            (&c)[n* i + k] += (&b)[n* j + k] * (&a)[n* (i-j) + k];
  } } } } }
#else
  if (n>0 && zorder>=0) {
    const auto zsz = std::size_t (1) << zorder;
    for (zyc::Zarray_int j=0; j<zsz; j++) {// permuted loop, transposed calc
      for (zyc::Zarray_int i=0; i<=j; i++) {
        if (zyc::cr_dual_nz (j, i)) {
          for (std::size_t k=0; k<n; k++) {
            (&c)[n* j + k] += (&b)[n* i + k] * (&a)[n* (j-i) + k];
  } } } } }
#endif
  return &c;
}
#   endif
#if 0
template <typename T> static inline constexpr
void zyc::aos_dual_mult
  (T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zorder_int pf, const zyc::Zorder_int pa, const zyc::Zorder_int pb,
  const std::size_t n=1)
noexcept {
  const auto zsz = std::size_t (1) << zorder;
  const auto sz = zsz * n;
  for (std::size_t k=0; k<sz; k+=zsz) {
    for (zyc::Zarray_int j=0; j<zsz; j++) {// permuted loop, transposed calc
      for (zyc::Zarray_int i=0; i<zsz; i++) {
      (&c)[k + j] += zyc::cr_dual_mult_elem ((&a)[k], (&b)[k], j, i, pa);
} } } }
#   endif
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
static inline constexpr
bool zyc::cr_dual_nz
  (const Zarray_int row, const Zarray_int col, const Zarray_int p)
noexcept{
  return ((row ^ col) == (row - col)) && ((row - col) < (Zarray_int(1) << p));
}
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v,
  const zyc::Zarray_int row, const zyc::Zarray_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
}
template <typename T> static inline constexpr
T zyc::cr_dual_elem (const T& ZYC_RESTRICT v, const zyc::Zarray_int row,
  const zyc::Zarray_int col, const zyc::Zarray_int p)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zarray_int(1) << p))
    ? (&v)[row - col] : T(0.0);
}
template <typename T> static inline constexpr
T zyc::cr_dual_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zarray_int row, const zyc::Zarray_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&a)[col] * (&b)[row - col] : T(0.0);
}
#if 0
template <typename T> static inline constexpr
T zyc::cr_dual_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyc::Zarray_int row, const zyc::Zarray_int col, Zarray_int p)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zarray_int(1) << p)
    ? (&a)[col] * (&b)[row - col] : T(0.0);
}
#endif
#if 0
template <typename T> static inline constexpr
void zyc::aos_dual_mult (T* ZYC_RESTRICT a,
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
