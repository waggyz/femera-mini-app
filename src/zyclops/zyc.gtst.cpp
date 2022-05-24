#include "zyc.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <cstdio>

namespace zyclops { namespace test {
  namespace zyc = ::zyclops;
  //
  TEST( Zyc, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
  TEST( Zyc, IntSizes ){
    EXPECT_GE( std::numeric_limits<Zorder_int>::max (), zyc::zorder_max );
#if 0
    EXPECT_EQ( sizeof (int_fast16_t), sizeof (int_fast32_t) );
#endif
    EXPECT_GT( std::numeric_limits<Zindex_int>::max (),
      zyc::upow (std::size_t (2), std::size_t (2)*zyc::zorder_max) - 1);
  }
  template <typename T> static inline constexpr
  T mdcr_elem_test (const T& ZYC_RESTRICT v,
    const zyc::Zindex_int row, const zyc::Zindex_int col)
  noexcept {
    return ((row ^ col) == (row - col)) ? (&v)[row - col] : T(0.0);
  }
  static inline
  double pass_by_reference (const zyc::Zindex_int row, const zyc::Zindex_int col)
  noexcept {
    std::vector<double> bidual = {1.0,0.0,0.0,0.0};
  return mdcr_elem_test (bidual [0], row, col);
  }
  TEST( Zyc, ByReference ){
    EXPECT_DOUBLE_EQ( pass_by_reference (0,0), 1.0 );
    EXPECT_DOUBLE_EQ( pass_by_reference (0,1), 0.0 );
    EXPECT_DOUBLE_EQ( pass_by_reference (1,1), 1.0 );
  }
  static inline constexpr
  int dual_ix (const int row, const int col)
  noexcept {//returns index of value, or -1 for a zero entry
  #if 0
    return ((row ^ col) + (row - col));// simplified below
  //  return ((row - col) - (row ^ col)) - col+row;
  //  return (row ^ col) - (row + col);
  #else
    return (row ^ col) == (row - col) ? row - col : -1;// this works
  #endif
  }
  static inline constexpr
  int dual_ix (const int row, const int col, const int array_order)
  noexcept {// safe for operations with different order operands
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
  TEST( Zyc, Multidual ){
    EXPECT_EQ( print_dual   (3,3), 27 );
    EXPECT_EQ( print_dual   (3,2), 27-9 );
  }
  TEST( Zyc, MultidualTransposed ){
    EXPECT_EQ( print_dual_t (3,3), 27 );
    EXPECT_EQ( print_dual_t (3,2), 27-9 );
    EXPECT_EQ( print_dual_t (4,4), zyc::upow (3,4));
  }
  static inline
  double real_f (double x, double y) {// f = x * y;
    return x * y;
  }
  static inline
  double real_dfdx (double, double y) {
    return y;
  }
  static inline
  double real_dfdy (double x, double) {
    return x;
  }
  static inline
  double real_d2fdxdy (double, double) {
    return 1.0;
  }
  TEST( Zyc, CheckF1Derivs ){
    EXPECT_DOUBLE_EQ( real_f       (3.0, 5.0), 15.0 );
    EXPECT_DOUBLE_EQ( real_dfdx    (3.0, 5.0),  5.0 );
    EXPECT_DOUBLE_EQ( real_dfdy    (3.0, 5.0),  3.0 );
    EXPECT_DOUBLE_EQ( real_d2fdxdy (3.0 ,5.0),  1.0 );
  }
  static inline
  double dual_aos_f1 (const double x, const double y,
    const Zindex_int zorder=0, const Zindex_int imag_part=0) {
    const size_t zsz = size_t (Zindex_int (1) << zorder);
    const std::vector<double> zx = {x,1.0,0.0,0.0, x,1.0,0.0,0.0};
    const std::vector<double> zy = {y,0.0,1.0,0.0, y,0.0,1.0,0.0};
    const uint n = 2;
    auto zf = std::vector<double> (zsz * n);
    zyc::mdas_madd (zf.data()[0], zx.data()[0], zy.data()[0], 2, n);
    const auto zf0 = zf [zsz*0 + std::size_t (imag_part)];
    const auto zf1 = zf [zsz*1 + std::size_t (imag_part)];
    if ((zf0<zf1) || (zf0>zf1)){ return NAN; }
    return zf0;
  }
  TEST( Zyc, BidualMultiplyAoS ){
    EXPECT_DOUBLE_EQ( dual_aos_f1 (3.0, 5.0, 2, 0), real_f       (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_aos_f1 (3.0, 5.0, 2, 1), real_dfdx    (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_aos_f1 (3.0, 5.0, 2, 2), real_dfdy    (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_aos_f1 (3.0, 5.0, 2, 3), real_d2fdxdy (3.0 ,5.0) );
  }
  static inline
  double dual_soa_f1 (const double x, const double y,
    const Zindex_int zorder=0, const Zindex_int imag_part=0) {
    const size_t zsz = size_t (Zindex_int (1) << zorder);
    const uint n = 2;
    const std::vector<double> zx = {x,x, 1.0,1.0, 0.0,0.0, 0.0,0.0, 0.0,0.0};
    const std::vector<double> zy = {y,y, 0.0,0.0, 1.0,1.0, 0.0,0.0, 0.0,0.0};
    auto zf = std::vector<double> (zsz * n);
    zyc::mdsa_madd (zf.data()[0], zx.data()[0], zy.data()[0], 2, n);
    const auto zf0 = zf [std::size_t (n* imag_part +0)];
    const auto zf1 = zf [std::size_t (n* imag_part +1)];
    if ((zf0<zf1) || (zf0>zf1)){ return NAN; }
    return zf0;
  }
  TEST( Zyc, BidualMultiplySoA ){
    EXPECT_DOUBLE_EQ( dual_soa_f1 (3.0, 5.0, 2, 0), real_f       (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_soa_f1 (3.0, 5.0, 2, 1), real_dfdx    (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_soa_f1 (3.0, 5.0, 2, 2), real_dfdy    (3.0, 5.0) );
    EXPECT_DOUBLE_EQ( dual_soa_f1 (3.0, 5.0, 2, 3), real_d2fdxdy (3.0 ,5.0) );
  }
#if 0
  static inline
  double dual_f2 (const double x, const double y,
    const Zindex_int pf=0, px=0, py=0, const Zindex_int imag_part=0) {
    const size_t zsz = size_t (Zindex_int (1) << pf);
    const std::vector<double> zx = {x,1.0};// porder 1
    const std::vector<double> zy = {y,0.0,1.0,0.0};// porder 2
    auto zf = std::vector<double> (zsz);
    zyc::mdas_madd (zf.data()[0], zx.data()[0], zy.data()[0], 2, 1, 2);
    return zf [std::size_t (imag_part)];
  }
#endif
#if 0
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
  TEST( Zyc, MultidualHammingWeight ){
    EXPECT_EQ( print_hamw (3,3), 27 );
  }
#endif
} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
