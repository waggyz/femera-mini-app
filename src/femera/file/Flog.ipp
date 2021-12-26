#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

namespace femera {
inline
file::Flog::Flog (femera::Work::Make_work_t W) noexcept {
  this->name ="file";
  std::tie(this->proc,this->file,this->data, this->test) = W;
}
inline
file::Flog::Flog () noexcept {
  this->name ="file";
}
inline
void file::Flog::task_exit () {
}

}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
