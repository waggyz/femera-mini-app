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
      {fmr::NEW_none, {}},             // suppress output
      {fmr::NEW_null, {}},             // convenient synonym
      {fmr::NEW_log , {fmr::NEW_out }},// task_init sets ::stdout to main only
      {fmr::NEW_out , {fmr::NEW_out }},// default all threads to ::stdout
      {fmr::NEW_err , {fmr::NEW_err }} // default all threads to ::stderr
    };
  }
  void data::Logs::task_init (int*, char**) {//TODO opts -v<int>, -t<int>,...
    // set default logger (data->fmrlog) to stdout only from the main thread (0)
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      n = this->proc->get_proc_n (Task_type::Fomp);// OpenMP threads / mpi proc
      n = (n==0) ? 1 : n;
    }
    // default fmr:log destination is ::stdout from thread 0.
    this->out_NEW_name_list [fmr::NEW_log ] = Data::Data_list_NEW_t (n, fmr::NEW_out);
    if (n > 0) { this->out_NEW_name_list [fmr::NEW_log][0] = fmr::NEW_out; }
#if 1
    this->data->fmrlog = data::File_ptrs_t (n, nullptr);//TODO REMOVE
    if (n > 0) { this->data->fmrlog [0] = ::stdout; }
#endif
    this->data->set_logs_init (true);
  }
  void data::Logs::task_exit () {
  }
  std::size_t data::Logs::task_NEW_send (const fmr::Data_name_NEW_t& file,
    const std::string& text, const fmr::Dim_int out_d)
  noexcept { std::size_t byte = 0;
#ifdef FMR_DEBUG
    printf ("logs task_NEW_send (%s, %s, %u)\n",
      file.c_str(), text.c_str(), out_d);
#endif
    if (this->do_log (out_d)) {
      if (this->out_NEW_name_list.find (file) != out_NEW_name_list.end ()) {
        const auto outs = this->out_NEW_name_list.at (file);//TODO double-lookup
        const auto n = outs.size ();
        if (n > 0) {
          FILE* f = nullptr;
          const auto dest = outs [this->proc->get_proc_ix () % n];
          if (dest == fmr::NEW_log) { f = ::stdout; };//TODO use unordered_map
          if (dest == fmr::NEW_out) { f = ::stdout; };
          if (dest == fmr::NEW_err) { f = ::stderr; };
          if (f != nullptr) {
            const auto b = fprintf (f, "%s\n", text.c_str ());
            //TODO warn if negative (fail)?
            byte += std::size_t ((b>0) ? b : 0);
    } } } }
    return byte;
  }
  //
}//end femera:: namespace
