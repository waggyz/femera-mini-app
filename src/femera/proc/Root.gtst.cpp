#include "Root.hpp"

#include "gtest/gtest.h"


auto test_top = femera::proc::Root
  (femera::Work::Core_t (nullptr,nullptr,nullptr));

TEST( Root, TaskName ){
  EXPECT_EQ( test_top.name, "top" );
}
TEST( Root, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(test_top), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
