#include "../femera.hpp"

#include <gtest/gtest.h>


auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  //sims_abrv  = mini.get_task   (femera::Task_type::Sims)->get_abrv ();
  //
  return mini.exit ();
}
TEST( AlgoSlvrPCGs, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

