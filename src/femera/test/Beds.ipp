#ifndef FEMERA_BEDS_IPP
#define FEMERA_BEDS_IPP

namespace femera {
  inline
  Beds::Beds (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="testbeds";
    this->info_d = 2;
  }
  inline
  Beds::Beds () noexcept {
    this->name ="testbeds";
    this->info_d = 2;
  }
  inline
  void Beds::task_exit () {
  }
}//end femera namespace

//end FEMERA_BEDS_IPP
#endif
