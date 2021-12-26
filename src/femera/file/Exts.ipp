#ifndef FEMERA_EXTS_IPP
#define FEMERA_EXTS_IPP

namespace femera {
inline
file::Exts::Exts (femera::Work::Make_work_t W) noexcept {
  this->name ="file";
  std::tie(this->proc,this->file,this->data, this->test) = W;
}
inline
file::Exts::Exts () noexcept {
  this->name ="file";
}
inline
void file::Exts::task_exit () {
}

}//end femera namespace

//end FEMERA_EXTS_IPP
#endif
