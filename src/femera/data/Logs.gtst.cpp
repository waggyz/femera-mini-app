#include "Logs.hpp"

#include "gtest/gtest.h"


auto flog = femera::data::Logs
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );

TEST( Logs, TaskName ){
  EXPECT_EQ( flog.name, "log" );
}
TEST( Logs, SizeofLogsGE120 ){
  EXPECT_GE( sizeof(flog), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
