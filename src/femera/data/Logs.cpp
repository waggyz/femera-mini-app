#include "Logs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void data::Logs::task_init (int*, char**) {
    // set default logger (data->fmrlog) to stdout only from the main thread (0)
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      n = this->proc->get_proc_n (Task_type::Fomp);// OpenMP threads / mpi proc
      n = (n==0) ? 1 : n;
    }
    this->data->fmrlog = data::File_ptrs_t (n, nullptr);
    if (n > 0) { this->data->fmrlog[0] = ::stdout; }
    this->data->set_logs_init (true);
  }
  void data::Logs::task_exit () {
  }
}//end femera:: namespace


