#ifndef FEMERA_HAS_TEST_SELF_IPP
#define FEMERA_HAS_TEST_SELF_IPP

namespace femera {
  inline
  test::Self::Self (const femera::Work::Core_ptrs_t W)
  noexcept : Test (W) {
    this->name      ="Femera integration self-tests";
    this->abrv      ="self";
    this->task_type = task_cast (Task_type::Self);
  }
  inline
  void test::Self::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_SELF_IPP
#endif
