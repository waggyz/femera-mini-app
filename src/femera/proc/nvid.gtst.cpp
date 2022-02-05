#include "../core.h"
#include "gtest/gtest.h"

#ifdef FMR_HAS_NVIDIA
#include "nvid.hpp"

TEST( nvid, TrivialTest ){
  EXPECT_GT( femera::proc::nvid::get_node_card_n (), 0 );
  EXPECT_EQ( femera::proc::nvid::get_card_name (0), "" );
}
#endif

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
