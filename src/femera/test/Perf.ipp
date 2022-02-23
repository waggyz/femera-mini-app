#ifndef FEMERA_HAS_TEST_PERF_IPP
#define FEMERA_HAS_TEST_PERF_IPP

namespace femera {
  inline
  test::Perf::Perf (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="performance tests";
    this->abrv ="perf";
    this->info_d = 2;
    this->task_type = task_cast (Plug_type::Perf);
  }
  inline
  void test::Perf::task_init (int*, char**) {
  }
  inline
  void test::Perf::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_PERF_IPP
#endif
