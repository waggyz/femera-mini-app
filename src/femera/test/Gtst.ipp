#ifndef FEMERA_GTST_IPP
#define FEMERA_GTST_IPP

namespace femera {
  inline
  test::Gtst::Gtst (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
  }
  inline
  test::Gtst::Gtst () noexcept {
    this->name ="GoogleTest";
    this->info_d = 3;
  }
  inline
  void test::Gtst::task_exit () {
  }
}//end femera namespace

//end FEMERA_GTST_IPP
#endif
