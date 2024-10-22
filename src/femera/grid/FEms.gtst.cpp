#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  return mini.exit ();
}
TEST( GridCellFEms, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

