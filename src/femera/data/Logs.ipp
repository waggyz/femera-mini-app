#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  data::Logs::Logs (const femera::Work::Core_ptrs_t W)
  noexcept : Data (W) {
    this->name      ="Femera logger";
    this->abrv      ="logs";
    this->task_type = task_cast (Plug_type::Logs);
    this->info_d    = 3;
  }
  inline
  void data::Logs::task_init (int*, char**) {
    // set default logger (data->fmrlog) to stdout only from the main thread (0)
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      const auto P = this->proc->get_task (Plug_type::Fomp);
      if (P == nullptr) {n = 1;}
      else {n = P->get_proc_n ();}// number of OpenMP threads / mpi process
    }
    this->data->fmrlog = data::File_ptrs_t (n, nullptr);
    if (n > 0) { this->data->fmrlog[0] = ::stdout; }
    this->data->set_logs_init (true);
  }
  inline
  void data::Logs::task_exit () {
  }
}//end femera namespace

//end FEMERA_DATA_LOGS_IPP
#endif
