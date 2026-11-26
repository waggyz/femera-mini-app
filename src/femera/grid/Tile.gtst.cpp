#include "../femera.hpp"

#include "Tile.hpp"
#include "tile/Bar1.hpp"

#include <gtest/gtest.h>
#include <omp.h>
#include <type_traits>

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

TEST( TileElem, HeaderCanPrecedeBar1 ){
  EXPECT_TRUE( (std::is_empty<femera::grid::tile::Tile<
    femera::grid::tile::Bar1>>::value) );
}


