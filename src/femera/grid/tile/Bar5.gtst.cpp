#include "../../femera.hpp"
#include "Bar5.hpp"
#include "../elem/Bars.hpp"// Used for quality tests.

#include <gtest/gtest.h>

#include <type_traits>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  //
  return mini.exit ();
}
TEST( GridTileBar5, TrivialTest ){
  EXPECT_TRUE( true );
}
namespace femera { namespace grid { namespace tile {



} } } // end namespace femera::grid::tile
