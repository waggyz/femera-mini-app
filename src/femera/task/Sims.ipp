#ifndef FEMERA_HAS_SIMS_IPP
#define FEMERA_HAS_SIMS_IPP

namespace femera {
  inline
  task::Sims::Sims (femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Femera simulations";
    this->abrv      ="sims";
    this->task_type = task_cast (Plug_type::Sims);
    this->info_d    = 3;
  }
  inline
  void task::Sims::task_init (int*, char**) {
  }
  inline
  void task::Sims::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_SIMS_IPP
#endif
