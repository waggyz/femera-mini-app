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

  TEST( Logs, TrivialTest) { EXPECT_EQ( 0, 0 ); }
#if 0
  TEST( Logs, SendLogLine) {
    EXPECT_EQ( mini->data->send (fmr:log, Vals_type::Logs_line, this->info_d,
      "data","logs","gtst","send logs line test level %i", 1), 43 );
    EXPECT_EQ( mini->data->send (fmr:log, Vals_type::Logs_line, FMR_VERBMAX+1,
      "data","logs","gtst","send logs line test level %i", FMR_VERBMAX+1), 0 );
  }
  TEST( Logs, SendLogNameLine) {
    EXPECT_EQ( mini->data->send (fmr:log, Vals_type::Name_line, this->info_d,
      " data logs gtst send ","  info line test %i", 1), 37 );// trims white space
  }
  TEST( Logs, SendLogInfoLine) {
    EXPECT_EQ( mini->data->send (fmr:log, Vals_type::Info_line, this->info_d,
      "data logs gtst send info line test %i", 1), 37 );
  }
#endif
} }//end femerea::test:: namespace
