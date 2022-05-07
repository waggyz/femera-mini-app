#include "Zomplex.hpp"

#include "gtest/gtest.h"

namespace zyc { namespace test {
  //
  auto tridual = zyc::Zomplex (zyc::Algebra::Dual, 3);
  auto tricomplex = zyc::Zomplex (zyc::Algebra::Complex, 3);
  //
  TEST( Zomplex, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
  TEST( Zomplex, TridualSizes ){
    EXPECT_EQ( tridual.get_order (),  3 );
    EXPECT_EQ( tridual.zval_size (),  8 );
    EXPECT_EQ( tridual.cr_size   (), 64 );
    EXPECT_EQ( tridual.cr_nnz    (), 27 );
  }
  TEST( Zomplex, TricomplexSizes ){
    EXPECT_EQ( tricomplex.get_order (),  3 );
    EXPECT_EQ( tricomplex.zval_size (),  8 );
    EXPECT_EQ( tricomplex.cr_size   (), 64 );
    EXPECT_EQ( tricomplex.cr_nnz    (), 64 );
  }

} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
