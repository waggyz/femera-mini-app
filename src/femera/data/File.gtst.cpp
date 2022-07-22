#include "../femera.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc,argv));
FMR_WARN_INLINE_ON
}

TEST( File, DataName ){
  EXPECT_EQ( mini->data->get_abrv (), "file" );
  EXPECT_GT( mini->data->NEW_send ("fmr:out","File","init","msg",
    "**** OK on thread %u", mini->proc->get_proc_id ()).size(), 25);
}
