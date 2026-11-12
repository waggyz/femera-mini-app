#include "../femera.hpp"

#include "Tile.hpp"

#include <gtest/gtest.h>
#include <omp.h>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}
TEST( TileElem, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}



