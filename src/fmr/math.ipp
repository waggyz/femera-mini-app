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

/*
## Copilot
Certainly! Below is an example of a C function that calculates the determinant of a 3x3 matrix using AVX intrinsics. This function assumes that the Intel AVX instruction set is available on the target machine. Please note that for brevity and clarity, the function does not include additional optimizations that could be implemented with AVX2 or AVX-512.

#include <immintrin.h>
#include <stdio.h>

float determinant_avx(float mat[3][3]) {
    __m256 row1 = _mm256_set_ps(0, 0, 0, mat[0][2], mat[0][1], mat[0][0], 0, 0);
    __m256 row2 = _mm256_set_ps(0, 0, 0, mat[1][0], mat[1][2], mat[1][1], 0, 0);
    __m256 row3 = _mm256_set_ps(0, 0, 0, mat[2][1], mat[2][0], mat[2][2], 0, 0);

    // Multiply across the diagonals
    __m256 diag1 = _mm256_mul_ps(row1, _mm256_mul_ps(row2, row3));

    // Shuffle the elements for the cross products
    row1 = _mm256_permute_ps(row1, _MM_SHUFFLE(0, 3, 2, 1));
    row2 = _mm256_permute_ps(row2, _MM_SHUFFLE(1, 0, 3, 2));
    row3 = _mm256_permute_ps(row3, _MM_SHUFFLE(2, 1, 0, 3));

    // Multiply across the diagonals for the cross products
    __m256 diag2 = _mm256_mul_ps(row1, _mm256_mul_ps(row2, row3));

    // Subtract the cross product from the main diagonal product
    __m256 det_vec = _mm256_sub_ps(diag1, diag2);

    // Horizontal add to get the determinant
    det_vec = _mm256_hadd_ps(det_vec, det_vec);
    det_vec = _mm256_hadd_ps(det_vec, det_vec);

    float det;
    _mm256_store_ss(&det, det_vec);

    return det;
}

int main() {
    float matrix[3][3] = {
        {1.0f, 2.0f, 3.0f},
        {4.0f, 5.0f, 6.0f},
        {7.0f, 8.0f, 9.0f}
    };

    float det = determinant_avx(matrix);
    printf("Determinant: %f\n", det);

    return 0;
}

Copilot
Using
the web
Certainly! Here’s how you can modify the function to work with double precision using AVX intrinsics:

#include <immintrin.h>
#include <stdio.h>

double determinant_avx(double mat[3][3]) {
    __m256d row1 = _mm256_set_pd(mat[0][2], mat[0][1], mat[0][0], 0);
    __m256d row2 = _mm256_set_pd(mat[1][0], mat[1][2], mat[1][1], 0);
    __m256d row3 = _mm256_set_pd(mat[2][1], mat[2][0], mat[2][2], 0);

    // Multiply across the diagonals
    __m256d diag1 = _mm256_mul_pd(row1, _mm256_mul_pd(row2, row3));

    // Shuffle the elements for the cross products
    row1 = _mm256_permute4x64_pd(row1, _MM_SHUFFLE(0, 3, 2, 1));
    row2 = _mm256_permute4x64_pd(row2, _MM_SHUFFLE(1, 0, 3, 2));
    row3 = _mm256_permute4x64_pd(row3, _MM_SHUFFLE(2, 1, 0, 3));
    // [Doesn't this shuffle zeros into the wrong places?]
#if 0
    // Multiply across the diagonals for the cross products
    __m256d diag2 = _mm256_mul_pd(row1, _mm256_mul_pd(row2, row3));

    // Subtract the cross product from the main diagonal product
    __m256d det_vec = _mm256_sub_pd(diag1, diag2);
#else
    // [FMA]
    _mm256_fmadd_pd ( -row1, _mm256_mul_pd(row2, row3) , diag1)
    //[TODO calc diag2 first, then shuffle for diag1, then _mm256_fmsub_pd]
#endif
    // Horizontal add to get the determinant
    det_vec = _mm256_hadd_pd(det_vec, det_vec);
    det_vec = _mm256_hadd_pd(det_vec, det_vec);
    //[I don't think this works]

    double det[4] = {0};
    _mm256_storeu_pd(det, det_vec);

    return det[0] - det[2];
}

int main() {
    double matrix[3][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    double det = determinant_avx(matrix);
    printf("Determinant: %lf\n", det);

    return 0;
}
[I think the next one is wrong, though it suggests an FMA optimization for above.]
Copilot
Using
the web
Yes, the function can be improved using Fused Multiply-Add (FMA) operations, which combine multiplication and addition in a single instruction. This can potentially reduce rounding errors and improve performance by decreasing the number of instructions required for the computation.

Here’s the updated function using FMA intrinsics for double precision:

#include <immintrin.h>
#include <stdio.h>

double determinant_avx_fma(double mat[3][3]) {
    __m256d row1 = _mm256_set_pd(mat[0][2], mat[0][1], mat[0][0], 0);
    __m256d row2 = _mm256_set_pd(mat[1][0], mat[1][2], mat[1][1], 0);
    __m256d row3 = _mm256_set_pd(mat[2][1], mat[2][0], mat[2][2], 0);

    // Multiply across the diagonals and add/subtract the cross products
    __m256d diag1 = _mm256_fmadd_pd(row1, _mm256_mul_pd(row2, row3), _mm256_set1_pd(0));// [This is silly, adding 0.0]
    __m256d diag2 = _mm256_fmsub_pd(row1, _mm256_mul_pd(row2, row3), _mm256_set1_pd(0));// [This is silly, subtracting 0.0]

    // Shuffle the elements for the cross products
    row1 = _mm256_permute4x64_pd(row1, _MM_SHUFFLE(0, 3, 2, 1));
    row2 = _mm256_permute4x64_pd(row2, _MM_SHUFFLE(1, 0, 3, 2));
    row3 = _mm256_permute4x64_pd(row3, _MM_SHUFFLE(2, 1, 0, 3));

    // Multiply across the diagonals for the cross products and subtract from the main diagonal product
    diag1 = _mm256_fmadd_pd(row1, _mm256_mul_pd(row2, row3), diag1);
    diag2 = _mm256_fmsub_pd(row1, _mm256_mul_pd(row2, row3), diag2);

    // Subtract the cross product from the main diagonal product
    __m256d det_vec = _mm256_sub_pd(diag1, diag2);

    // Horizontal add to get the determinant
    det_vec = _mm256_hadd_pd(det_vec, det_vec);
    det_vec = _mm256_hadd_pd(det_vec, det_vec);

    double det[4] = {0};
    _mm256_storeu_pd(det, det_vec);

    return det[0] - det[2];
}

int main() {
    double matrix[3][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    double det = determinant_avx_fma(matrix);
    printf("Determinant: %lf\n", det);

    return 0;
}
*/

#undef FMR_DEBUG
//end FMR_HAS_MATH_IPP
#endif
