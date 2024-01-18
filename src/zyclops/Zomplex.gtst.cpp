#include "Zomplex.hpp"

#include <gtest/gtest.h>

namespace zyclops { namespace test {
  namespace zyc = ::zyclops;
  //
  auto tridual = zyc::Zomplex (zyc::Algebra::Dual, 3, zyc::Layout::Inset);
  auto tricomplex = zyc::Zomplex (zyc::Algebra::Complex, 3, zyc::Layout::Inset);
  //
  TEST( Zomplex, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
  TEST( Zomplex, TridualSizes ){
    EXPECT_EQ( tridual.get_order (),  3 );
    EXPECT_EQ( tridual.hc_size (),  8 );
    EXPECT_EQ( tridual.cr_size   (), 64 );
    EXPECT_EQ( tridual.cr_nnz    (), 27 );
  }
  TEST( Zomplex, TricomplexSizes ){
    EXPECT_EQ( tricomplex.get_order (),  3 );
    EXPECT_EQ( tricomplex.hc_size (),  8 );
    EXPECT_EQ( tricomplex.cr_size   (), 64 );
    EXPECT_EQ( tricomplex.cr_nnz    (), 64 );
  }
} }//end zyclops::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
