#include "../core.h"
#include "gtest/gtest.h"

#include "nvid.hpp"

#ifdef FMR_HAS_NVIDIA
#if 0
TEST( NvidNamespace, InitFunctions ){
  EXPECT_GE( femera::proc::nvid::get_node_card_n (), 0 );
  EXPECT_NE( femera::proc::nvid::get_card_name (0), "" );
}
#endif
#endif

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}