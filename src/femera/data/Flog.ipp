#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

namespace femera {
inline
data::Flog::Flog (femera::Work::Make_work_t W) noexcept {
  this->name ="log";
  std::tie (this->proc, this->data, this->test) = W;
}
inline
data::Flog::Flog () noexcept {
  this->name ="log";
}
inline
void data::Flog::task_exit () {
}

}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
