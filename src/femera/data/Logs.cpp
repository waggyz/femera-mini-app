#include "Logs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  data::Logs::Logs (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera logger";
    this->abrv      ="logs";
    this->task_type = task_cast (Task_type::Logs);
    this->info_d    = 3;
    this->out_NEW_name_list = {
      {fmr::NEW_log , {fmr::NEW_out }},
      {fmr::NEW_out , {fmr::NEW_out }},
      {fmr::NEW_err , {fmr::NEW_err }},
      {fmr::NEW_null, {fmr::NEW_null}},
      {fmr::NEW_none, {fmr::NEW_null}}
    };
  }
  void data::Logs::task_init (int*, char**) {//TODO opts -v<int>, -t<int>,...
    // set default logger (data->fmrlog) to stdout only from the main thread (0)
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      n = this->proc->get_proc_n (Task_type::Fomp);// OpenMP threads / mpi proc
      n = (n==0) ? 1 : n;
    }
    // default fmr:log destination is STDOUT from thread 0.
    this->out_NEW_name_list["fmr:log"] = Data::Data_list_NEW_t (n, "fmr:null");
    if (n > 0) { this->out_NEW_name_list["fmr:log"][0] = "fmr:out"; }
#if 1
    this->data->fmrlog = data::File_ptrs_t (n, nullptr);//TODO REMOVE
    if (n > 0) { this->data->fmrlog[0] = ::stdout; }
#endif
    this->data->set_logs_init (true);
  }
  void data::Logs::task_exit () {
  }
}//end femera:: namespace
