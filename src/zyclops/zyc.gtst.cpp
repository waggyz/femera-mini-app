#include "zyc.hpp"

#include "gtest/gtest.h"

#include <cstdio>

namespace zyc { namespace test {
  //
  static inline
  int print_dual (int show_order, int array_order ) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = zyc::dual_ix (i,j, array_order);
        nnz += (ix>=0) ? 1 : 0;
        printf ("%2s%s",
          ix < 0 ? "" : std::to_string(ix).c_str(), j == (n-1) ? "\n":" ");
    } }
    return nnz;
  }
  static inline
  int print_dual_t (int show_order, int array_order ) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = zyc::dual_tix (i,j, array_order);
        nnz += (ix>=0) ? 1 : 0;
        printf ("%2s%s",
          ix < 0 ? "" : std::to_string(ix).c_str(), j == (n-1) ? "\n":" ");
    } }
    return nnz;
  }
  TEST( Zyc, TrivialTest ){
    EXPECT_EQ( 1, 1 );
    EXPECT_EQ( print_dual   (3,3), 27 );
    EXPECT_EQ( print_dual   (3,2), 27-9 );
    EXPECT_EQ( print_dual_t (3,3), 27 );
    EXPECT_EQ( print_dual_t (3,2), 27-9 );
    EXPECT_EQ( print_dual_t (4,4), zyc::upow (3,4));
  }
} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
