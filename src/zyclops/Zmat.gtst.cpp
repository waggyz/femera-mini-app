#include "Zmat.hpp"

#include <gtest/gtest.h>

namespace zyc { namespace test {
  //
  TEST( Zmat, TrivialTest ){
    EXPECT_EQ( 1, 1 );
  }
} }//end zyc::test:: namespace

int main (int argc, char** argv) {
  ::testing::InitGoogleTest (& argc,argv);
  return RUN_ALL_TESTS();
}
