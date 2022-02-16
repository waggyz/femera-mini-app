#ifndef FEMERA_GTST_IPP
#define FEMERA_GTST_IPP

namespace femera {
  inline
  test::Gtst::Gtst (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="GoogleTest";
    this->abrv ="gtst";
    this->version = MAKESTR(FMR_GTEST_VERSION) ;
    this->task_type = task_cast (Plug_type::Gtst);
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_GTST_IPP
#endif
