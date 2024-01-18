#include "Zmat.hpp"

#include <gtest/gtest.h>

namespace zyclops { namespace test {
  namespace zyc = ::zyclops;
  //
  TEST( Zmat, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
} }//end zyclops::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
