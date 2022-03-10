#include "../femera.hpp"

#include "gtest/gtest.h"

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  return mini->exit (mini->init (&argc,argv));
}

TEST( File, DataName ){
  EXPECT_EQ( mini->data->abrv, "file" );
}
