#include "../femera.hpp"
#include "Bank.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <valarray>
#include <cstring>           // std::memcmp
#include <cstdio>            // std::snprintf
#include <cmath>             // isnan, std::signbit


namespace femera { namespace test {
  const auto mini = fmr::new_jobs ();

  TEST(Mini, IsOK) {
    EXPECT_EQ( 1, 1);
  }
  TEST(Bank, Existence) {
    ASSERT_NE(mini->data, nullptr);

    const auto bank = mini->data->get_task(femera::Task_type::Bank);
    ASSERT_NE(bank, nullptr);

    EXPECT_EQ(bank->get_base_abrv(), "data");
    EXPECT_EQ(bank->get_abrv(), "bank");
  }
#if 0
  TEST(MiniVals, Add) {
    EXPECT_EQ( static_cast<femera::data::Bank>
    (mini->data->get_task(Task_type::Bank)) ->add<int>("test",10,1)[0], 1);
  }
#endif
} }//end femerea::test:: namespace

fmr::Exit_int main (int argc, char** argv) {
FMR_WARN_INLINE_OFF
  return femera::test::mini->exit (femera::test::mini->init (&argc, argv));
FMR_WARN_INLINE_ON
}
