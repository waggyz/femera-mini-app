#ifndef FEMERA_HAS_TEST_FE1E_IPP
#define FEMERA_HAS_TEST_FE1E_IPP

namespace femera {
  inline
  test::FE1e::FE1e (const femera::Work::Core_ptrs_t core)
  noexcept : Test (core) {
    this->name      ="Femera single-element test stub";
    this->abrv      ="fe1e";
    this->task_type = task_cast (Task_type::FE1e);
  }
  inline
  void test::FE1e::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void test::FE1e::task_exit () {
  }
}//end femera namespace

#endif
