#include "Work.hpp"

#include "gtest/gtest.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST( Work, TrivialTest ) {
  EXPECT_EQ( 1, 1 );
}
#undef FMR_DEBUG
