#include "../core.h"
#include "Fcpu.hpp"

#include <gtest/gtest.h>

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_proc = femera::proc::Main ();

TEST( Fcpu, TaskName ){
#pragma GCC diagnostic ignored "-Winline"
  EXPECT_EQ( test_proc.init (nullptr,nullptr), 0 ); test_proc.proc = &test_proc;
#pragma GCC diagnostic warning "-Winline"
  EXPECT_EQ( test_proc.get_task (femera::Task_type::Fcpu)->get_abrv (), "cpu" );
  EXPECT_NE( test_proc.get_task (femera::Task_type::Fcpu)->get_name (), "" );
}
TEST( Node, Exit ){
  EXPECT_EQ( test_proc.exit (0), 0 );
}
