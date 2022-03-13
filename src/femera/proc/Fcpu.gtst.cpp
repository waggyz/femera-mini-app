#include "../core.h"
#include "Fcpu.hpp"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_proc = femera::proc::Main ();

TEST( Fcpu, TaskName ){
  EXPECT_EQ( test_proc.init (nullptr,nullptr), 0 ); test_proc.proc = &test_proc;
  EXPECT_EQ( test_proc.get_task (femera::Plug_type::Fcpu)->abrv, "cpu" );
  EXPECT_NE( test_proc.get_task (femera::Plug_type::Fcpu)->name, "" );
}
TEST( Node, Exit ){
  EXPECT_EQ( test_proc.exit (0), 0 );
}
