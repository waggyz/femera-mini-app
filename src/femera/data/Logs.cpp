#include "Logs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  data::Logs::Logs (const femera::Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->time.set_unit_name ("line");
    this->name      ="Femera logger";
    this->abrv      ="logs";
    this->task_type = task_cast (Task_type::Logs);
    this->info_d    = 3;
    this->out_name_list = {
      {fmr::none, {}},         // suppress output
      {fmr::null, {}},         // convenient synonym
      {fmr::log , {fmr::out }},// task_init sets ::stdout to main only
      {fmr::out , {fmr::out }},// default all threads to ::stdout
      {fmr::err , {fmr::err }} // default all threads to ::stderr
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
    this->out_name_list [fmr::log ] = Data::Data_list_t (n, fmr::out);
    if (n > 0) { this->out_name_list [fmr::log][0] = fmr::out; }
    this-> did_init_tf = true;
  }
  void data::Logs::task_exit () {
  }
  std::size_t data::Logs::task_send (const fmr::Data_name_t& file,
    const std::string& text, const fmr::Dim_int out_d)
  noexcept { std::size_t byte = 0;
#ifdef FMR_DEBUG
    printf ("logs task_send (%s, %s, %u)\n",
      file.c_str(), text.c_str(), out_d);
#endif
    if (! this->do_log (out_d)) { return byte; }
    //
    fmr::Exit_int err = 0;
    Data_list_t outs  ={};
    try { outs = this->out_name_list.at (file); }
    catch (std::out_of_range& e) { err = 1; }
    catch (const Warn& e)    { err =-1; e.print (); }
    catch (const Errs& e)    { err = 1; e.print (); }
    catch (std::exception& e){ err = 2;
      Errs::print (this->get_abrv ()+" task_send", e); }
    catch (...)              { err = 3;
      Errs::print (this->get_abrv ()+" task_send"); }
    if (err > 0) { return byte; }
    //
    const auto n = outs.size ();
    if (n <= 0) { return byte; }
    //
    FILE* f = nullptr;
    const auto dest = outs [this->proc->get_proc_ix () % n];
    if (dest == fmr::log) { f = ::stdout; };//TODO use unordered_map ?
    if (dest == fmr::out) { f = ::stdout; };
    if (dest == fmr::err) { f = ::stderr; };
    if (f == nullptr) { return byte; }
    //
    this->time.add_idle_time_now ();
    const auto b = fprintf (f, "%s\n", text.c_str ());
    this->time.add_busy_time_now ();
    //TODO warn if negative (fail)?
    byte += std::size_t ((b>0) ? b : 0);
    this->time.add_count (1, 0, 0, byte);
    return byte;
  }
  //
}//end femera:: namespace
