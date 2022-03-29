#ifndef FEMERA_HAS_TEST_SELF_IPP
#define FEMERA_HAS_TEST_SELF_IPP

namespace femera {
  inline
  test::Self::Self (const femera::Work::Core_ptrs_t W)
  noexcept : Test (W) {
    this->name      ="Femera integration self-tests";
    this->abrv      ="self";
    this->task_type = task_cast (Plug_type::Self);
    this->info_d    = 3;
  }
  inline
  void test::Self::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_SELF_IPP
#endif
