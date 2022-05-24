#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

static inline
zyclops::Zindex_int zyclops::upow
(zyclops::Zindex_int base, zyclops::Zindex_int exponent)
noexcept {
  /* stackoverflow.com/questions/101439/the-most-efficient-way-to-implement
     -an-integer-based-power-function-powint-int */
  zyclops::Zindex_int result = 1;
  while (true) {//for (;;) {
    if (exponent & 1){ result *= base; }
    exponent >>= 1;
    if (!exponent){ break; }
    base *= base;
  }
  return result;
}
static inline constexpr
uint zyclops::hamw (const uint64_t i)
noexcept {
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u64 (i));// intrinsic returns __int64
#else
  return uint(__builtin_popcountll (i));// gcc builtin returns int
#endif
}
static inline constexpr
uint zyclops::hamw (const uint32_t i)
noexcept {
#ifdef __INTEL_COMPILER
  return uint(_mm_popcnt_u32 (i));// intrinsic returns int
#else
  return uint(__builtin_popcount (i));// gcc builtin returns int
#endif
}
static inline constexpr
bool zyclops::is_mdcr_nz (const Zindex_int row, const Zindex_int col)
noexcept{
  return (row ^ col) == (row - col);
}
static inline constexpr
bool zyclops::is_mdcr_nz// for access to order > stored
(const Zindex_int row, const Zindex_int col, const Zorder_int o)
noexcept{
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o));
}
/* The difference between pass-by-reference and pass-by-pointer is that pointers
 * can be NULL or reassigned whereas references cannot. Use pass-by-pointer
 * if NULL is a valid parameter value or if you want to reassign the pointer.
 * Otherwise, use constant or non-constant references to pass arguments.
 *
 * https://www.ibm.com/docs/en/zos/2.4.0?topic=calls-pass-by-reference-c-only
 */
template <typename T> static inline constexpr
T zyclops::mdcr_elem (const T& ZYC_RESTRICT v,
  const zyclops::Zindex_int row, const zyclops::Zindex_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
}
template <typename T> static inline constexpr
T zyclops::mdcr_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zindex_int row, const zyclops::Zindex_int col)
noexcept {
  return ((row ^ col) == (row - col)) ? (&a)[col] * (&b)[row - col] : T(0.0);
}
template <typename T> static inline constexpr// for access to order > stored
T zyclops::mdcr_elem (const T& ZYC_RESTRICT v, const zyclops::Zindex_int row,
  const zyclops::Zindex_int col, const zyclops::Zorder_int o)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o))
    ? (&v)[row - col] : T(0.0);
}
template <typename T> static inline constexpr// for heterogeneous order multiply
T zyclops::mdcr_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zindex_int row, const zyclops::Zorder_int col,
  const Zorder_int o)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o))
    ? (&a)[col] * (&b)[row - col] : T(0.0);
}
template <typename T> static inline
T* zyclops::mdas_madd// array of structs storage
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n=1)
noexcept {
  if (zorder>=0 && n>0) {
    const auto zsz = std::size_t (1) << zorder;
    const auto nsz = n * zsz;
    for (std::size_t k=0; k<nsz; k+=zsz) {
      for (zyclops::Zindex_int j=0; j<zsz; j++) {// permuted loop, transp. calc
        for (zyclops::Zindex_int i=0; i<=j; i++) {
          (&c)[k + j] += zyclops::mdcr_mult_elem ((&a)[k], (&b)[k], j, i);
  } } } }
  return &c;
}
template <typename T> static inline
T* zyclops::mdsa_madd// struct of arrays storage
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n=1)
noexcept {
#if 0
  if (n>0 && zorder>=0) {
    const auto zsz = std::size_t (1) << zorder;
    for (zyclops::Zindex_int j=0; j<zsz; j++) {// permuted loop
      for (zyclops::Zindex_int i=j; i<zsz; i++) {
        if (zyclops::is_mdcr_nz (i, j)) {
          for (std::size_t k=0; k<n; k++) {
            (&c)[n* i + k] += (&b)[n* j + k] * (&a)[n* (i-j) + k];
  } } } } }
#else
  if (zorder>=0 && n>0) {
    const auto zsz = std::size_t (1) << zorder;
    for (zyclops::Zindex_int j=0; j<zsz; j++) {// permuted loop, transp. calc
      for (zyclops::Zindex_int i=0; i<=j; i++) {
        if (zyclops::is_mdcr_nz (j, i)) {
          for (std::size_t k=0; k<n; k++) {
            (&c)[n* j + k] += (&b)[n* i + k] * (&a)[n* (j-i) + k];
  } } } } }
#endif
  return &c;
}
//end ZYC_HAS_ZYC_IPP
#endif
