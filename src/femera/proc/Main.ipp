#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

namespace femera {
inline
proc::Main::Main (femera::Work::Make_work_t W) noexcept {
  this->name ="Femera";
  std::tie (this->proc, this->data, this->test) = W;
}
inline
proc::Main::Main () noexcept {
  this->name ="Femera";
}
inline
void proc::Main::task_exit () {
}

}//end femera namespace

//end FEMERA_MAIN_IPP
#endif
