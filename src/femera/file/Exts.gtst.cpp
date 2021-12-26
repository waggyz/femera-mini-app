#include "Exts.hpp"

#include "gtest/gtest.h"


auto exts = femera::file::Exts();

TEST( Exts, TaskName ){
  EXPECT_EQ( exts.name, "file" );
}
TEST( Exts, SizeofExtsGE120 ){
  EXPECT_GE( sizeof(exts), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
