#include "Proc.hpp"

#include "gtest/gtest.h"

TEST(EarlyProc, IsOK) {
  EXPECT_EQ( 1, 1);
}
TEST(Proc, NodeCoreN) {
  EXPECT_GE( femera::Proc<femera::Work>::get_node_core_n (), 1);
#ifdef FMR_CORE_N
  EXPECT_EQ( femera::Proc<femera::Work>::get_node_core_n (), FMR_CORE_N );
#else
  EXPECT_EQ( femera::Proc<femera::Work>::get_node_core_n (),
    femera::Proc<femera::Work>::get_node_hype_ix () );
#endif
}
TEST(ProcNode, NodeHypeN) {
  EXPECT_GE( femera::Proc<femera::Work>::get_node_hype_n (),
    femera::Proc<femera::Work>::get_node_core_n ());
}
TEST(ProcNode, NodeNumaN) {
  EXPECT_GE( femera::Proc<femera::Work>::get_node_numa_n (), 1);
}
TEST(ProcNode, NodeCoreIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_core_ix (),
    femera::Proc<femera::Work>::get_node_core_n ());
}
TEST(ProcNode, NodeHypeIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_hype_ix (),
    femera::Proc<femera::Work>::get_node_hype_n ());
}
TEST(ProcNode, NodeNumaIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_numa_ix (),
    femera::Proc<femera::Work>::get_node_numa_n ());
}
TEST(ProcNode, NodeUsedByte) {
  EXPECT_GT( femera::Proc<femera::Work>::get_node_used_byte (), 1000);
}
int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
