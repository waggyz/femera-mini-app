#ifndef FEMERA_BEDS_IPP
#define FEMERA_BEDS_IPP

namespace femera {
inline
Beds::Beds (femera::Work::Make_work_t W) noexcept {
  this->name ="testbeds";
  std::tie (this->proc, this->data, this->test) = W;
}
inline
Beds::Beds () noexcept {
  this->name ="testbeds";
}
inline
void Beds::task_exit () {
}

}//end femera namespace

//end FEMERA_BEDS_IPP
#endif
