#include "../femera.hpp"

#include <gtest/gtest.h>


fmr::Exit_int main (int argc, char** argv) {

  auto  mini_ptr = fmr::new_jobs (& argc, argv);
  auto& mini = *mini_ptr;

//mini.add_task(mini.this_cast(femera::Task_type::Mtrl));

//FMR_WARN_INLINE_OFF
  return mini.exit ();
//FMR_WARN_INLINE_ON
}

TEST( Mtrl, TrivialTest ){
  EXPECT_EQ( 1, 1 );
#if 0
  EXPECT_EQ( mini->get_task(femera::Task_type::Mtrl)->get_abrv (), "mtrl" );
#endif
}


#if 0

      const auto mini_ptr = std::move (fmr::new_jobs (& argc, argv));
      // --or-- auto mini_ptr = fmr::new_jobs (& argc, argv);
      auto& mini = *mini_ptr;
      // Do mini.* things after parsing the arguments
      return mini.exit ();
#endif
