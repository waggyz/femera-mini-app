#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Prmd.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

TEST( GridElemPrmd, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

