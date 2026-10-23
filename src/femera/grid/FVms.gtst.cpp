#include "../femera.hpp"
#include "FVms.hpp"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  return mini.exit ();
}
TEST( GridCellFVms, TaskMetadata ){
  auto grid = femera::grid::FVms::new_task (mini.get_core ());
  EXPECT_EQ( grid->get_name (), "Femera finite volume grid" );
  EXPECT_EQ( grid->get_abrv (), "FV" );
  EXPECT_EQ( grid->get_task_type (), femera::task_cast (femera::Task_type::FVms) );
}
