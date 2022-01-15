#include "proc.hpp"

#include "gtest/gtest.h"


TEST(FmrProc, NodeInfo) {
  EXPECT_GE(fmr::proc::get_node_core_n (), 1);
  EXPECT_GE(fmr::proc::get_node_hype_n (), 1);
  EXPECT_GE(fmr::proc::get_node_numa_n (), 0);
  EXPECT_GE(fmr::proc::get_node_core_ix (), 0);
  EXPECT_GE(fmr::proc::get_node_hype_ix (), 0);
  EXPECT_GE(fmr::proc::get_node_numa_ix (), 0);
  EXPECT_GT(fmr::proc::get_node_used_byte (), 1000);
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
