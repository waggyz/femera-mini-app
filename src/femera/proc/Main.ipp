#ifndef FEMERA_MAIN_IPP
#define FEMERA_MAIN_IPP

namespace femera {
  inline
  proc::Main::Main (const femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="processing";
    this->info_d = 2;
    this->abrv ="main";
  }
  inline
  proc::Main::Main () noexcept {
    this->name ="processing";
    this->info_d = 2;
    this->abrv ="main";
  }
  inline
  void proc::Main::task_exit () {
    this->proc =nullptr;
  }
  inline
  fmr::Local_int proc::Main::task_proc_ix () {
    return this->proc_ix;
  }
}//end femera:: namespace

//end FEMERA_MAIN_IPP
#endif
