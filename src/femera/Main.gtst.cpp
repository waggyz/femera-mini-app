#include "Main.hpp"

#include "main-early.gtst.ipp"
using namespace femera;

auto fmr = Main();

TEST( fmr, TaskName ){
  EXPECT_EQ( work->task_name, "Main" );
}
TEST( fmr, SizeofWorkGE120 ){
  EXPECT_GE( sizeof(fmr), 120 );
}
