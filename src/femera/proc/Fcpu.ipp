#ifndef FEMERA_FCPU_IPP
#define FEMERA_FCPU_IPP

namespace femera {

inline
proc::Fcpu::Fcpu (femera::Work::Make_work_t W) noexcept {
  this->name ="CPU";
  std::tie (this->proc, this->data, this->test) = W;
}
inline
proc::Fcpu::Fcpu () noexcept {
  this->name ="CPU";
}
inline
void proc::Fcpu::task_exit () {
}

}//end femera namespace

//end FEMERA_FCPU_IPP
#endif
