#include "Beds.hpp"

#ifdef FMR_HAS_GTEST
#include "Gtst.hpp"
#endif

namespace femera {
  void test::Beds::task_init (int*, char**) {
#ifdef FMR_HAS_GTEST
    this->add_task (Test<test::Gtst>::new_task (this->get_core()));
#endif
  }
}//end femera namespace
