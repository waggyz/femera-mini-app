#include "../femera.hpp"
//#include "Jobs.hpp"

#include "gtest/gtest.h"
#if 1
auto test_jobs = femera::sims::Jobs
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );
#else
  auto test_jobs = femera::new_sims ();
#endif

int   dummy_argc = 1;
char  dummy [] ="dummy";
char* dummy_argv [] = {dummy};

TEST( Jobs, TaskName ){
  EXPECT_EQ( test_jobs.name, "simulation job handler" );
}
TEST( Jobs, CoreNames ){
  EXPECT_EQ( test_jobs.init (&dummy_argc,&dummy_argv[0]), 0);
  EXPECT_EQ( test_jobs.proc->name, "processing environment" );
  EXPECT_EQ( test_jobs.data->name, "file handler" );
  EXPECT_EQ( test_jobs.test->name, "testbeds" );
}
TEST( Jobs, SizeofGtstGE120 ){
  EXPECT_GE( sizeof(test_jobs), 120 );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
