#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

static inline
zyclops::Zindex_int zyclops::upow
(zyclops::Zindex_int base, zyclops::Zindex_int exponent)
noexcept {/* stackoverflow.com/questions/101439/the-most-efficient-way-to
  -implement-an-integer-based-power-function-powint-int */
  zyclops::Zindex_int result = 1;
  while (true) {//for (;;) {
    if (exponent & 1) { result *= base; }
    exponent >>= 1;
    if (! exponent) { break; }
    base *= base;
  }
  return result;
}
static inline constexpr
zyclops::Zindex_int zyclops::upow2 (zyclops::Zindex_int exponent)
noexcept {
  return zyclops::Zindex_int (1) << exponent;
}
#if 1
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
#endif
static inline constexpr
bool zyclops::is_mdcr_nz (const Zindex_int row, const Zindex_int col)
noexcept{
#if 0
  return (row ^ col) == (row - col);// works
  return ((row ^ col) & col) == 0;// works
#endif
  return ~((row ^ col) & col);// works
}
static inline constexpr
bool zyclops::is_mdcr_nz// for access to order > stored
(const Zindex_int row, const Zindex_int col, const Zorder_int o)
noexcept{
  return (~((row ^ col) & col)) && ((row ^ col) < (Zindex_int(1) << o));// works
#if 0
  // These also work.
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o));
  return (~((row ^ col) & col)) && ((row - col) < (Zindex_int(1) << o));// works
#endif
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
#if 0
  return (~((row ^ col) & col)) ? (&v)[row - col] : T(0.0); // works
  return T(~((row ^ col) & col)) * (&v)[row - col];         // works?
#endif
  return T(~((row ^ col) & col)) * (&v)[row ^ col];         // works
}
template <typename T> static inline constexpr// for access to order > stored
T zyclops::mdcr_elem (const T& ZYC_RESTRICT v, const zyclops::Zindex_int row,
  const zyclops::Zindex_int col, const zyclops::Zorder_int o)
noexcept {
#if 0
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o))
    ? (&v)[row - col] : T(0.0);// works
  return (~((row ^ col) & col)) && ((row - col) < (Zindex_int(1) << o))
    ? (&v)[row - col] : T(0.0);// works
#endif
  return (~((row ^ col) & col)) && ((row ^ col) < (Zindex_int(1) << o))
    ? (&v)[row ^ col] : T(0.0);// works
}
template <typename T> static inline constexpr
T zyclops::mdcr_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zindex_int row, const zyclops::Zindex_int col)
noexcept {
  return (~((row ^ col) & col)) ? (&a)[col] * (&b)[row ^ col] : T(0.0);
#if 0
  return (~((row ^ col) & col)) ? (&a)[col] * (&b)[row - col] : T(0.0);// works
  return T(1-((row ^ col) & col)) * (&a)[col] * (&b)[row - col];// works
  return ((row ^ col) == (row - col)) ? (&b)[row - col] * (&a)[col] : T(0.0);
#endif
}
#if 0
template <typename T> static inline constexpr// for heterogeneous order multiply
T zyclops::mdcr_mult_elem (const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zindex_int row, const zyclops::Zorder_int col,
  const Zorder_int o)
noexcept {
  return ((row ^ col) == (row - col)) && ((row - col) < (Zindex_int(1) << o))
    ? (&a)[col] * (&b)[row - col] : T(0.0);
}
#endif
template <typename T> static inline
T* zyclops::mdas_madd// array of structs storage, c += a * b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (zorder>=0 && n>0) {
    const auto zsz = std::size_t (1) << zorder;
    const auto nsz = n * zsz;
    for (std::size_t k=0; k<nsz; k+=zsz) {
      for (zyclops::Zindex_int j=0; j<zsz; j++) { // permuted loop, transp. calc
        for (zyclops::Zindex_int i=0; i<=j; i++) {// only lower triangle
          (&c)[k + j] += zyclops::mdcr_mult_elem ((&a)[k], (&b)[k], j, i);
  } } } }
  return &c;
}
template <typename T> static inline
T* zyclops::mdsa_madd// struct of arrays storage, c += a * b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
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
    for (zyclops::Zindex_int j=0; j<zsz; j++) { // permuted loop, transp. calc
      for (zyclops::Zindex_int i=0; i<=j; i++) {// only lower triangle
        if (zyclops::is_mdcr_nz (j, i)) {
          for (std::size_t k=0; k<n; k++) {
#if 0
            (&c)[n* j + k] += (&b)[n* i + k] * (&a)[n* (j-i) + k];
#endif
            (&c)[n* j + k] += (&b)[n* i + k] * (&a)[n* (j^i) + k];
  } } } } }
#endif
  return &c;
}
template <typename T> static inline
T* zyclops::mdas_div// array of structs storage, c = a / b, c zeroed first
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (n>0) {
    if (zorder<=0) {                                        // real, zsz = 1
      for (std::size_t k=0; k<n; k++) {(&c)[k] = (&a)[k] / (&b)[k];}
    } else {                                                // hypercomplex
      const auto zsz = std::size_t (1) << zorder;           // always >1
      const auto nsz = n * zsz;
      for (std::size_t k=0; k<nsz; k+=zsz) {
        const auto b0inv = T(1.0) / (&b)[k];
        (&c)[k] = (&a)[k] * b0inv;// real part
        for (zyclops::Zindex_int j=1; j<zsz; j++) {// permuted loop, transp calc
          for (zyclops::Zindex_int i=0; i<j; i++) {// lower triangle w/out diag.
            (&c)[k + j] -= zyclops::mdcr_mult_elem ((&c)[k], (&b)[k], j, i);
          }
          (&c)[k + j] += (&a)[k + j];
          (&c)[k + j] *= b0inv;
  } } } }
  return &c;
}
template <typename T> static inline
T* zyclops::mdsa_div// struct of arrays storage, c = a / b, c zeroed first
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (n>0) {
    if (zorder<=0) {                                        // real, zsz = 1
      for (std::size_t k=0; k<n; k++) {(&c)[k] = (&a)[k] / (&b)[k];}
    } else {
      const auto zsz = std::size_t (1) << zorder;           // always >1
      for (zyclops::Zindex_int j=0; j<zsz; j++) {// permuted loop, transp. calc
        if (j>0) {
          for (zyclops::Zindex_int i=0; i<j; i++) {// lower triangle, no diag.
            if (zyclops::is_mdcr_nz (j, i)) {
              for (std::size_t k=0; k<n; k++) {
#if 0
                (&c)[n* j + k] -= (&c)[n* i + k] * (&b)[n* (j-i) + k];
#endif
                (&c)[n* j + k] -= (&c)[n* i + k] * (&b)[n* (j^i) + k];
        } } } }
        for (std::size_t k=0; k<n; k++) {
          (&c)[n* j + k] += (&a)[n* j + k];
          (&c)[n* j + k] /= (&b)[k];
  } } } }
  return &c;
}
template <typename T> static inline
T* zyclops::hca_add// add, c = a + b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (zorder>=0 && n>0) {
    const auto nsz = (std::size_t (1) << zorder) * n;       // 2^zorder * n
    for (zyclops::Zindex_int i=0; i<nsz; i++) {
      (&c)[i] = (&a)[i] + (&b)[i];
  } }
  return &c;
}
template <typename T> static inline
T* zyclops::hca_adda// add-accumulate, c += a + b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (zorder>=0 && n>0) {
    const auto nsz = (std::size_t (1) << zorder) * n;       // 2^zorder * n
    for (zyclops::Zindex_int i=0; i<nsz; i++) {
      (&c)[i] += (&a)[i] + (&b)[i];
  } }
  return &c;
}
template <typename T> static inline
T* zyclops::hca_sub// subtract, c = a - b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (zorder>=0 && n>0) {
    const auto nsz = (std::size_t (1) << zorder) * n;       // 2^zorder * n
    for (zyclops::Zindex_int i=0; i<nsz; i++) {
      (&c)[i] = (&a)[i] - (&b)[i];
  } }
  return &c;
}
template <typename T> static inline
T* zyclops::hca_suba// subtract-accumulate, c += a - b
(T& ZYC_RESTRICT c, const T& ZYC_RESTRICT a, const T& ZYC_RESTRICT b,
  const zyclops::Zorder_int zorder, const std::size_t n)
noexcept {
  if (zorder>=0 && n>0) {
    const auto nsz = (std::size_t (1) << zorder) * n;       // 2^zorder * n
    for (zyclops::Zindex_int i=0; i<nsz; i++) {
      (&c)[i] += (&a)[i] - (&b)[i];
  } }
  return &c;
}
//end ZYC_HAS_ZYC_IPP
#endif
