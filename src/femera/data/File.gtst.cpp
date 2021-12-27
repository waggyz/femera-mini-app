#include "File.hpp"

#include "gtest/gtest.h"


auto file = femera::File();

TEST( File, TaskName ){
  EXPECT_EQ( file.name, "file" );
}
TEST( File, SizeofFileGE120 ){
  EXPECT_GE( sizeof(file), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
