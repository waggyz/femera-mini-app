#include "Beds.hpp"

#ifdef FMR_HAS_GTEST
#include "Gtst.hpp"
#endif
#include "Self.hpp"

namespace femera {
  void test::Beds::task_init (int*, char**) {
#ifdef FMR_HAS_GTEST
    this->add_task (Test<test::Gtst>::new_task (this->get_core()));
#endif
    if (true) {//TODO if -T option in args
      this->add_task (Test<test::Self>::new_task (this->get_core()));
  } }
}//end femera namespace
