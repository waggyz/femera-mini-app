#ifndef FEMERA_HAS_TEST_PERF_IPP
#define FEMERA_HAS_TEST_PERF_IPP

namespace femera {
  inline
  test::Perf::Perf (const femera::Work::Core_ptrs_t core)
  noexcept : Test (core) {
    this->name      ="Femera performance tests";
    this->abrv      ="perf";
    this->task_type = task_cast (Task_type::Perf);
  }
  inline
  void test::Perf::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void test::Perf::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_PERF_IPP
#endif
