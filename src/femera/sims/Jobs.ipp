#ifndef FEMERA_HAS_JOBS_IPP
#define FEMERA_HAS_JOBS_IPP

namespace femera {
  inline
  sims::Jobs::Jobs (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="simulation job handler";
    this->info_d = 3;
  }
  inline
  sims::Jobs::Jobs () noexcept {
    this->name ="simulation job handler";
    this->info_d = 3;
  }
  inline
  void sims::Jobs::task_init (int*, char**){
//    if (this->proc == nullptr) { this->proc = Proc::new_task (argc, argv); }
  }
  inline
  void sims::Jobs::task_exit () {
  }
}//end femera namespace

//end FEMERA_HAS_JOBS_IPP
#endif
