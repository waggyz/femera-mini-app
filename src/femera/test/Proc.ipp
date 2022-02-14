#ifndef FEMERA_HAS_TEST_PROC_IPP
#define FEMERA_HAS_TEST_PROC_IPP

namespace femera {
  inline
  test::Proc::Proc (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="test Proc";
    this->abrv ="Proc";
    this->info_d = 2;
  }
  inline
  void test::Proc::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_PROC_IPP
#endif
