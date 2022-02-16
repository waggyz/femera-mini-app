#ifndef FEMERA_HAS_JOBS_IPP
#define FEMERA_HAS_JOBS_IPP

namespace femera {
  inline
  sims::Jobs::Jobs (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="simulation job handler";
    this->abrv ="jobs";
    this->task_type = task_cast (Plug_type::Jobs);
    this->base_type = task_cast (Base_type::Sims);
    this->info_d = 3;
  }
  inline
  sims::Jobs::Jobs () noexcept {
    this->name ="simulation job handler";
    this->abrv ="jobs";
    this->task_type = task_cast (Plug_type::Jobs);
    this->base_type = task_cast (Base_type::Sims);
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_HAS_JOBS_IPP
#endif
