#include "../core.h"
#include "Node.hpp"

#include <gtest/gtest.h>

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
auto test_proc = femera::proc::Main ();

TEST( Node, TaskName ){
#pragma GCC diagnostic ignored "-Winline"
  EXPECT_EQ( test_proc.init (nullptr,nullptr), 0 ); test_proc.proc = &test_proc;
#pragma GCC diagnostic warning "-Winline"
  EXPECT_NE( test_proc.get_task (femera::Task_type::Node)->get_name(), "node" );
}
TEST(Node, CoreN) {
  EXPECT_GE( femera::proc::Node::get_core_n (), uint(1));
#ifdef FMR_CORE_N
  EXPECT_EQ( femera::proc::Node::get_core_n (), uint(FMR_CORE_N) );
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
  EXPECT_GE( femera::proc::Node::get_numa_n (), uint(1));
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
  EXPECT_GT( femera::proc::Node::get_used_byte (), uint(1000));
}
TEST( Node, Exit ){
  EXPECT_EQ( test_proc.exit (0), 0 );
}
