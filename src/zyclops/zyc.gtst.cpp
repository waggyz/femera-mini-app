#include "zyc.hpp"

#include <gtest/gtest.h>

#include <cstdio>

namespace zyc { namespace test {
  //
  static inline constexpr
  int dual_ix (const int row, const int col)
  noexcept {
    //returns index of value, or -1 for a zero entry
  #if 0
    return ((row ^ col) + (row - col));//TODO try to simplify below
  //  return ((row - col) - (row ^ col)) - col+row;
  //  return (row ^ col) - (row + col);
  #   else
    return (row ^ col) == (row - col) ? row - col : -1;// this works
  #endif
  }
  static inline constexpr
  int dual_ix (const int row, const int col, const int array_order)
  noexcept {
    // this one is safe for operations with different order operands
  #if 0
    const auto ix = row - col;
    return (ix < (1<<array_order)) && ((row ^ col) == ix) ? ix : -1;
  #else
    return ((row - col) < (1<<array_order)) && ((row ^ col) == (row - col))
      ? row - col : -1;// constexpr
  #endif
  }
  static inline
  int print_dual (int show_order, int array_order) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = dual_ix (i,j, array_order);
        nnz += (ix<0) ? 0 : 1;
        printf ("%2s%s",
          ix < 0 ? "" : std::to_string(ix).c_str(), j == (n-1) ? "\n":" ");
    } }
    return nnz;
  }
  static inline
  int print_dual_t (int show_order, int array_order) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = dual_ix (j,i, array_order);
        nnz += (ix<0) ? 0 : 1;
        printf ("%2s%s",
          ix < 0 ? "" : std::to_string(ix).c_str(), j == (n-1) ? "\n":" ");
    } }
    return nnz;
  }
  static inline
  int print_hamw (int show_order, int array_order) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        printf ("%2s%s",
          std::to_string (zyc::hamw (uint(n*i+j))).c_str(),
          j == (n-1) ? "\n":" ");
    } }
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = dual_ix (i,j, array_order);
        nnz += (ix<0) ? 0 : 1;
        printf ("%2s%s",
          ix < 0 ? "" : std::to_string (zyc::hamw (uint(n*i+j))).c_str(),
          j == (n-1) ? "\n":" ");
    } }
    return nnz;
  }
  TEST( Zyc, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
  TEST( Zyc, Multidual ){
    EXPECT_EQ( print_dual   (3,3), 27 );
    EXPECT_EQ( print_dual   (3,2), 27-9 );
  }
  TEST( Zyc, MultidualTransposed ){
    EXPECT_EQ( print_dual_t (3,3), 27 );
    EXPECT_EQ( print_dual_t (3,2), 27-9 );
    EXPECT_EQ( print_dual_t (4,4), zyc::upow (3,4));
  }
#if 0
  TEST( Zyc, MultidualHammingWeight ){
    EXPECT_EQ( print_hamw (3,3), 27 );
  }
#endif
} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
