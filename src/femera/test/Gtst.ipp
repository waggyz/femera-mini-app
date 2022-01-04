#ifndef FEMERA_GTST_IPP
#define FEMERA_GTST_IPP

namespace femera {
  inline
  test::Gtst::Gtst (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="GoogleTest";
    this->info_d = 3;
  }
#if 0
  inline
  test::Gtst::Gtst () noexcept {
    this->name ="GoogleTest";
    this->info_d = 3;
  }
#endif
}//end femera namespace

//end FEMERA_GTST_IPP
#endif
