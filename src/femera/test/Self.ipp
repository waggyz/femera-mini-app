#ifndef FEMERA_HAS_TEST_SELF_IPP
#define FEMERA_HAS_TEST_SELF_IPP

namespace femera {
  inline
  test::Self::Self (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="self test";
    this->abrv ="self";
    this->info_d = 2;
  }
  inline
  void test::Self::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_TEST_SELF_IPP
#endif
