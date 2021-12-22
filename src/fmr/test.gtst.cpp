#include "test.hpp"

#include "gtest/gtest.h"

TEST(EarlyMain, IsOK) {
  EXPECT_EQ( 1, 1);
}
int main (int argc, char** argv) {
  return fmr::detail::test:: early_main (&argc, argv);
}
