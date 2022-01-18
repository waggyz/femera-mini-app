#include "Node.hpp"

#include "gtest/gtest.h"


auto test_node = femera::proc::Node
  (femera::Work::Core_t (nullptr,nullptr,nullptr));

TEST( Node, TaskName ){
  EXPECT_NE( test_node.name, "node" );
}
TEST(Node, CoreN) {
  EXPECT_GE( femera::proc::Node::get_core_n (), 1);
#ifdef FMR_CORE_N
  EXPECT_EQ( femera::proc::Node::get_core_n (), FMR_CORE_N );
#else
  EXPECT_EQ( femera::proc::Node::get_core_n (),
    femera::proc::Node::get_hype_ix () );
#endif
}
TEST(Node, HypeN) {
  EXPECT_GE( femera::proc::Node::get_hype_n (),
    femera::proc::Node::get_core_n ());
}
TEST(Node, NumaN) {
  EXPECT_GE( femera::proc::Node::get_numa_n (), 1);
}
TEST(Node, CoreIx) {
  EXPECT_LT( femera::proc::Node::get_core_ix (),
    femera::proc::Node::get_core_n ());
}
TEST(Node, HypeIx) {
  EXPECT_LT( femera::proc::Node::get_hype_ix (),
    femera::proc::Node::get_hype_n ());
}
TEST(Node, NumaIx) {
  EXPECT_LT( femera::proc::Node::get_numa_ix (),
    femera::proc::Node::get_numa_n ());
}
TEST(Node, UsedByte) {
  EXPECT_GT( femera::proc::Node::get_used_byte (), 1000);
}
fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
