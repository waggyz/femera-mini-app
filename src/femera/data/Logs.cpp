#include "Logs.hpp"

#ifdef FMR_HAS_GTEST
//#include "gtest/gtest.h"
#endif

namespace femera {
  void data::Logs::task_init (int*, char**) {
    fmr::Local_int n=0;
    if (this->proc->is_main ()) {
      n = 2;//FIXME number of OpenMP threads / mpi process
    }
    this->data->fmrlog = Data::File_ptrs_t (n, nullptr);
    if (n > 0) { this->data->fmrlog[0] = ::stdout; }
    this->data->did_logs_init = true;
  }
  void data::Logs::task_exit () {}
}//end femera:: namespace


