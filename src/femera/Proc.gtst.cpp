#include "Proc.hpp"

#include "gtest/gtest.h"

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

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
TEST(Proc, NodeHypeN) {
  EXPECT_GE( femera::Proc<femera::Work>::get_node_hype_n (),
    femera::Proc<femera::Work>::get_node_core_n ());
}
TEST(Proc, NodeNumaN) {
  EXPECT_GE( femera::Proc<femera::Work>::get_node_core_n (), 1);
}
TEST(Proc, NodeCoreIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_core_ix (),
    femera::Proc<femera::Work>::get_node_core_n ());
}
TEST(Proc, NodeHypeIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_hype_ix (),
    femera::Proc<femera::Work>::get_node_hype_n ());
}
TEST(Proc, NodeNumaIx) {
  EXPECT_LT( femera::Proc<femera::Work>::get_node_numa_ix (),
    femera::Proc<femera::Work>::get_node_numa_n ());
}
TEST(Proc, NodeUsedByte) {
  EXPECT_GT( femera::Proc<femera::Work>::get_node_used_byte (), 0);
}
