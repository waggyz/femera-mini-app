#include "../femera.hpp"

#include "gtest/gtest.h"
#if 1
auto jobs = femera::sims::Jobs
 ( femera::Work::Core_t (nullptr,nullptr,nullptr) );
#else
  auto jobs = femera::new_sims ();
#endif

int   dummy_argc = 1;
char  dummy [] ="dummy";
char* dummy_argv [] = {dummy};

TEST( Jobs, TaskName ){
  EXPECT_EQ( jobs.name, "simulation job handler" );
}
TEST( Jobs, CoreNames ){
  EXPECT_EQ( jobs.init (&dummy_argc,&dummy_argv[0]), 0);
  EXPECT_EQ( jobs.proc->name, "processing environment" );
  EXPECT_EQ( jobs.data->name, "file handler" );
  EXPECT_EQ( jobs.test->name, "testbeds" );
}
TEST( Jobs, SizeofGtstGEWork ){
  EXPECT_GE( sizeof(jobs), sizeof(femera::Work) );
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
