#include "../femera.hpp"

#include "gtest/gtest.h"
#if 0
auto jobs = femera::task::Jobs
 ( femera::Work::Core_ptrs_t (nullptr,nullptr,nullptr) );
#else
  auto jobs = femera::task::Jobs ();
//  auto jobs = femera::new_jobs ();
#endif

int   dummy_argc = 1;
char  dummy [] ="dummy";
char* dummy_argv [] = {dummy};

TEST( Jobs, TaskAbrv ){
  EXPECT_EQ( jobs.get_abrv (), "jobs" );
}
TEST( Jobs, CoreNames ){
  EXPECT_EQ( jobs.init (&dummy_argc,&dummy_argv[0]), 0);
  EXPECT_EQ( jobs.proc->get_abrv (), "main" );
  EXPECT_EQ( jobs.data->get_abrv (), "file" );
  EXPECT_EQ( jobs.test->get_abrv (), "beds" );
}
TEST( Jobs, SizeofGtstGEWork ){
  EXPECT_GE( sizeof(jobs), sizeof(femera::Work) );
}
TEST( Jobs, Exit ){
  EXPECT_EQ( jobs.exit (), 0);
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
