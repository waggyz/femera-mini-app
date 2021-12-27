#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
inline
File::File (femera::Work::Make_work_t W) noexcept {
  this->name ="file";
  std::tie (this->proc, this->data, this->test) = W;
}
inline
File::File () noexcept {
  this->name ="file";
}
inline
void File::task_exit () {
}

}//end femera namespace

//end FEMERA_FILE_IPP
#endif
