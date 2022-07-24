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
EXPECT_GE( mini->data->NEW_send (fmr::NEW_log, "Logs","gtst","msg",
  "**** OK on thread %u", mini->proc->get_proc_id ()),
  mini->proc->is_main () ? 20 : 0);
}
//
} }//end femerea::test:: namespace
