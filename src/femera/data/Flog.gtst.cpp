#include "Flog.hpp"

#include "gtest/gtest.h"


auto flog = femera::data::Flog
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );

TEST( Flog, TaskName ){
  EXPECT_EQ( flog.name, "log" );
}
TEST( Flog, SizeofFlogGE120 ){
  EXPECT_GE( sizeof(flog), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
