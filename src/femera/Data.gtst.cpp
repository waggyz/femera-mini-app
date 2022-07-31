#include "Data.hpp"

#include <gtest/gtest.h>

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
namespace femera { namespace test {
  TEST(EarlyData, IsOK) {
    EXPECT_EQ( 1, 1);
  } //
} }//end femerea::test:: namespace
