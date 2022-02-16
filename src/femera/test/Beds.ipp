#ifndef FEMERA_HAS_BEDS_IPP
#define FEMERA_HAS_BEDS_IPP

namespace femera {
  inline
  test::Beds::Beds (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="testbeds";
    this->abrv ="beds";
    this->task_type = task_cast (Plug_type::Beds);
    this->base_type = task_cast (Base_type::Test);
    this->info_d = 2;
  }
  inline
  void test::Beds::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_BEDS_IPP
#endif
