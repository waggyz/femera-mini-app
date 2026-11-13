#include "elem-poly.hpp"

#include <gtest/gtest.h>

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST(FmrElemPoly, TrivialTest) {
  EXPECT_EQ( 1, 1 );
}
//TODO some tests here. Currently some tests are in bars.gtst.cpp
