#ifndef FEMERA_HAS_TEST_PTCH_IPP
#define FEMERA_HAS_TEST_PTCH_IPP

namespace femera {
  inline
  test::Ptch::Ptch (const femera::Work::Core_ptrs_t core)
  noexcept : Test (core) {
    this->name      ="Femera finite element patch tests";
    this->abrv      ="ptch";
    this->task_type = task_cast (Task_type::Ptch);
  }
  inline
  void test::Ptch::task_init (int*, char**) {
    // Keep the task lifecycle stub-only until numerical patch cases are ready.
    this->set_init (true);
  }
  inline
  void test::Ptch::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_PTCH_IPP
#endif
