#include "Task.hpp"

#include <gtest/gtest.h>

TEST(Task, IsOK) {
  EXPECT_EQ( 1, 1);
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
