#include "../femera.hpp"
#include "FDms.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (& argc, argv);
  //
  return mini->exit ();
}
TEST( GridCellFEms, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
