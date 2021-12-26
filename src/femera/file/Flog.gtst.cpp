#include "Flog.hpp"

#include "gtest/gtest.h"


auto flog = femera::file::Flog();

TEST( Flog, TaskName ){
  EXPECT_EQ( flog.name, "file" );
}
TEST( Flog, SizeofFlogGE120 ){
  EXPECT_GE( sizeof(flog), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
