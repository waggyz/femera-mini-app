#include "../../femera.hpp"
#include "../Elem.test.hpp"
#include "Wdge.hpp"

#include <gtest/gtest.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}

TEST( GridElemWdge, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

