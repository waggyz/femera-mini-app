#include "../femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}

namespace femera { namespace test {
//
TEST( File, DataName ){
  EXPECT_EQ( mini->data->get_abrv (), "file" );
}
TEST( File, SendOut ){
  EXPECT_GT( mini->data->send (fmr::out,"File","gtst","out",
    "**** OK on thread %u", mini->proc->get_proc_id ()), 25);
}
TEST( File, GetSetVerbosity ){
  EXPECT_EQ( mini->data->set_verb (1), 1);
  EXPECT_EQ( mini->data->get_verb (), 1);
  EXPECT_EQ( mini->data->set_verb (3), 3);
}
TEST( File, IsInFilesystem ){
  EXPECT_TRUE( mini->data->in_filesystem (".") );// Current directory should exist
  EXPECT_FALSE( mini->data->in_filesystem ("this_file_should_not_exist.txt") );
}
//
} }//end femera::test:: namespace
