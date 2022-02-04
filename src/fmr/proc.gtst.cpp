#include "proc.hpp"

#include "gtest/gtest.h"

TEST(FmrProc, NodeInfo) {
  EXPECT_GE(fmr::proc::get_node_core_n (), uint(1));
  EXPECT_GE(fmr::proc::get_node_hype_n (), uint(1));
  EXPECT_GE(fmr::proc::get_node_numa_n (), uint(0));
  EXPECT_GE(fmr::proc::get_node_core_ix (), uint(0));
  EXPECT_GE(fmr::proc::get_node_hype_ix (), uint(0));
  EXPECT_GE(fmr::proc::get_node_numa_ix (), uint(0));
  EXPECT_LT(fmr::proc::get_node_core_ix (), fmr::proc::get_node_core_n ());
  EXPECT_LT(fmr::proc::get_node_hype_ix (), fmr::proc::get_node_hype_n ());
  EXPECT_LT(fmr::proc::get_node_numa_ix (), fmr::proc::get_node_numa_n ());
  EXPECT_GT(fmr::proc::get_used_byte (), uint(1000));
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
