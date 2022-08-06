#include "Logs.hpp"
#include "File.hpp"
#include "../proc/Main.hpp"

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
    this->out_name_list = {     //NOTE task_init calls set_verb to reset these
      {fmr::none , {}},         // suppress output
      {fmr::null , {}},         // convenient synonym to fmr::none
      {fmr::err  , {fmr::err }},// default all threads to ::stderr verb_d >= 0
      {fmr::out  , {fmr::out }},// default all threads to ::stdout verb_d >= 1
      {fmr::log  , {fmr::out }},// default main thread to ::stdout verb_d >= 1
//      {fmr::perf , {fmr::out }},// default main thread to ::stdout verb_d >= 2
      {fmr::info , {fmr::out }},// default main thread to ::stdout verb_d >= 3
      {fmr::spam , {fmr::out }},// default all threads to ::stdout verb_d >= 4
      {fmr::debug, {fmr::out }} // default all threads to ::stdout verb_d >= 5
    };
  }
  fmr::Dim_int data::Logs::set_verb (const int v)
  noexcept {
    bool did_reduce = false;
    if (v < 0) {
      if (this->did_init ()) {// print warning
        this->data->send (fmr::err, "data","logs","WARN","Verbosity remains "
          "(%i) because requested verbosity (%i) is negative.",
          int (this->verb_d), int (v));
      } else {
        fprintf (::stderr, "data logs WARN Verbosity remains "
          "(%i) because requested verbosity (%i) is negative.\n",
          int (this->verb_d), int (v));
      }
      return this->verb_d;
    }
    if (v > FMR_VERBMAX) {
      this->verb_d = FMR_VERBMAX;
      if (this->did_init ()) {// print warning
        this->data->send (fmr::err, "data","logs","WARN","Verbosity set "
          "to (%i) because requested (%i) exceeds maximum (%i).",
          int (this->verb_d), int (v), int (FMR_VERBMAX));
      } else {
        fprintf (::stderr, "data logs WARN Verbosity set "
          "to (%i) because requested (%i) exceeds maximum (%i).\n",
          int (this->verb_d), int (v), int (FMR_VERBMAX));
    } }
    else {
      did_reduce = v < this->verb_d;
      this->verb_d = fmr::Dim_int (v);
    }
    if (did_reduce) {// verbosity reduced
    this->name      ="Femera logger";
      const auto sv = (this->verb_d > 5) ? 5 : this->verb_d;
      switch (sv) {
        case 0 : this->out_name_list [fmr::out  ] = {};// all cases fall through
                       out_name_list [fmr::log  ] = {};
//        case 1 : this->out_name_list [fmr::perf ] = {};
        case 2 : this->out_name_list [fmr::info ] = {};
        case 3 : this->out_name_list [fmr::spam ] = {};
        case 4 : this->out_name_list [fmr::debug] = {};
    } }
    if (this->did_init ()) {// print info
      this->data->send (fmr::info,
        "data","logs","verb","%4i    /%4i maximum verbosity",
        int (this->verb_d), int (FMR_VERBMAX));
    }
    return this->verb_d;
  }
  void data::Logs::task_init (int*, char**) {//TODO opts -v<int>, -t<int>,...
    // set default logger (data->fmrlog) to stdout only from the main thread (0)
    fmr::Local_int n = 0;
    if (this->proc->is_main ()) {
      n = this->proc->get_race_n ();// OpenMP threads / mpi proc
      n = (n==0) ? 1 : n;
    }
    // default fmr:log, fmr:info destinations are ::stdout from thread 0 only.
    this->out_name_list [fmr::log ] = Data::Data_list_t (n, fmr::out);
    this->out_name_list [fmr::info] = Data::Data_list_t (n, fmr::out);
//    this->out_name_list [fmr::perf] = Data::Data_list_t (n, fmr::out);
    if (n > 0) {
      this->out_name_list [fmr::log ][0] = fmr::out;
      this->out_name_list [fmr::info][0] = fmr::out;
//      this->out_name_list [fmr::perf][0] = fmr::out;
    }
    this->name += " (main to stdout, all to stderr)";
#ifdef FMR_DEBUG
    this->set_verb (11);
    this->set_verb (0);
#endif
    this->set_init (true);
    this->set_verb (this->verb_d);// prints info
  }
  void data::Logs::task_exit () {
  }
  fmr::Global_int data::Logs::task_send
  (const fmr::Data_name_t& file, const std::string& text)
  noexcept { fmr::Global_int byte = 0;
#ifdef FMR_DEBUG
    printf ("logs task_send (%s, %s, %u)\n",
      file.c_str(), text.c_str());
#endif
    fmr::Exit_int err = 0;
    Data_list_t outs  ={};
    try { outs = this->out_name_list.at (file); }
    catch (std::out_of_range& e) { err = 1; }
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
    try { f = this->open_file_map.at (dest); }
    catch (std::out_of_range& e) { err = 1; }
    catch (std::exception& e){ err = 2;
      Errs::print (this->get_abrv ()+" task_send", e); }
    catch (...)              { err = 3;
      Errs::print (this->get_abrv ()+" task_send"); }
    if ((err > 0) || (f == nullptr)) { return byte; }
    //
    FMR_PRAGMA_OMP(omp MAIN)
    { this->time.add_idle_time_now (); }// TODO Handle other threads safely.
    const auto b = fprintf (f, "%s\n", text.c_str ());
    FMR_PRAGMA_OMP(omp MAIN)
    { this->time.add_busy_time_now (); }
    if ((b < 0) && (this->did_init ())) {// Warn fprintf negative return value.
      fprintf (::stderr, "data logs WARN "
        "task_send fprintf ([to \"%s\"], \"%%s\\n\", \"%s\") "
        "returned (%i), negative bytes written.\n",
        file.c_str(), text.c_str(), int (b));
    }
    byte += fmr::Global_int ((b>0) ? b : 0);
    FMR_PRAGMA_OMP(omp MAIN)
    { this->time.add_count (1, 0, 0, byte); }
    return byte;
  }//
  //
}//end femera:: namespace
//
