#include "../femera.hpp"
#include "Logs.hpp"

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return mini->exit (mini->init (&argc, argv));
FMR_WARN_INLINE_ON
}
namespace femera { namespace test {
//
TEST( Logs, TrivialTest) { EXPECT_EQ( 0, 0 ); }
TEST( Logs, Send) {
  EXPECT_TRUE (mini->data->can_send (fmr::log));
  EXPECT_GE( mini->data->send (fmr::log, "Logs","gtst","msg",
    "**** OK on thread %u", mini->proc->get_proc_id ()),
    mini->proc->is_main () ? 20 : 0);
}
TEST( Logs, CanSend) {
  EXPECT_TRUE (mini->data->can_send (fmr::err));
  EXPECT_TRUE (mini->data->can_send (fmr::out));
  EXPECT_TRUE (mini->data->can_send (fmr::log));
  #if 0
  EXPECT_TRUE (mini->data->can_send (fmr::plog));
  EXPECT_TRUE (mini->data->can_send (fmr::info));
  EXPECT_TRUE (mini->data->can_send (fmr::spam));
  EXPECT_TRUE (mini->data->can_send (fmr::dbug));
  EXPECT_TRUE (mini->data->can_send (fmr::none));
  #endif
  EXPECT_FALSE (mini->data->can_send ("fmr::yon"));
}
//
} }//end femerea::test:: namespace
