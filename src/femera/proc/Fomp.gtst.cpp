#include "../core.h"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (& argc, argv);
}
auto test_main = femera::proc::Main ();

TEST( Fomp, Init ){// Initialize Fmpi with MPI already init by early_main (..)
  EXPECT_EQ( 0, test_main.init (nullptr,nullptr) );
  test_main.proc = & test_main;
}
TEST( Fomp, TaskAbrv ){
  //TODO get_task by enum?
  EXPECT_EQ( test_main.get_task (femera::Plug_type::Fomp)->abrv, "omp" );
}
TEST( Fomp, Exit ){// should NOT finalize MPI: not initialized by test_main
  EXPECT_EQ( test_main.exit (0), 0 );
}
