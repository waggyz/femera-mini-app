#include "../femera.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Local_int sims_count =0;
std::string sims_abrv;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  sims_count = mini.get_task_n (femera::Task_type::Sims);
  sims_abrv  = mini.get_task   (femera::Task_type::Sims)->get_abrv ();
  //
  return mini.exit ();
}

TEST( PhysMtrl, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
TEST( PhysMtrl, TaskStackHasSims ){
  EXPECT_GT( mini.get_task_n (), 0 );
  EXPECT_GT( sims_count, 0 );
  EXPECT_EQ( sims_abrv , "sims" );
}

