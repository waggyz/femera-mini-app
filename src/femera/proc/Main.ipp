#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

namespace femera {
  inline
  Main::Main (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
  }
  inline
  Main::Main () noexcept {
    this->name ="Femera";
    this->info_d = 2;
  }
  inline
  void Main::task_exit () {
  }
}//end femera namespace

//end FEMERA_MAIN_IPP
#endif
