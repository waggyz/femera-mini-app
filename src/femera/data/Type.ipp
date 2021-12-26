#ifndef FEMERA_TYPE_IPP
#define FEMERA_TYPE_IPP

namespace femera {
inline
data::Type::Type (femera::Work::Make_work_t W) noexcept {
  this->name ="data";
  std::tie(this->proc,this->file,this->data, this->test) = W;
}
inline
data::Type::Type () noexcept {
  this->name ="data";
}
inline
void data::Type::task_exit () {
}

}//end femera namespace

//end FEMERA_TYPE_IPP
#endif
