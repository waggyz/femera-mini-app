#include "zyc.hpp"

#include "gtest/gtest.h"

#include <cstdio>

namespace zyc { namespace test {
  //
  static inline
  int print_dual (uint show_order, uint array_order ) {
    int nnz=0;
    const int n = 1 << show_order;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        const auto ix = zyc::dual_ix (i,j, array_order);
        nnz += (ix>=0) ? 1 : 0;
        printf ("%3i%s", ix, j==(n-1) ? "\n":" ");
    } }
    return nnz;
  }
  //
  TEST( Zmat, TrivialTest ){
    EXPECT_EQ( 1, 1 );
    EXPECT_EQ( print_dual (3,3), 27 );
    EXPECT_EQ( print_dual (3,2), 27-9 );
  }
} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
