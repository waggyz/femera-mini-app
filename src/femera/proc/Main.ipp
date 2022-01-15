#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

namespace femera {
  inline
  proc::Main::Main (const femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="processing";
    this->info_d = 2;
  }
  inline
  proc::Main::Main () noexcept {
    this->name ="processing";
    this->info_d = 2;
  }
  inline
  void proc::Main::task_exit () {
  }
}//end femera:: namespace

//end FEMERA_MAIN_IPP
#endif
