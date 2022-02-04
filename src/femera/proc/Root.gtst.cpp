#include "../core.h"
#include "Root.hpp"

#include "gtest/gtest.h"

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_proc = femera::proc::Main ();

TEST( Root, TaskName ){
  EXPECT_EQ( test_proc.init (nullptr,nullptr), 0 ); test_proc.proc = &test_proc;
  EXPECT_EQ( test_proc.get_task_raw ({0})->name, "root process" );
}
TEST( Root, ProcNisOne ){
  EXPECT_EQ( test_proc.get_task_raw ({0})->get_proc_n (), uint(1) );
}
TEST( Root, ProcIxIsZero ){
  EXPECT_EQ( test_proc.get_task_raw ({0})->get_proc_ix (), uint(0) );
}
/*
TEST( Root, ProcIdIsZero ){
  EXPECT_EQ( test_proc.get_task_raw ({0})->get_proc_id (), 0 );
}
*/
TEST( Root, Exit ){
  EXPECT_EQ( test_proc.exit (0), 0 );
}