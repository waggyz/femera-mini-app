#include "../Main/Plug.hpp"

#include "omp.h"
#include <unistd.h>   // getopt, optarg

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf DEBUG
#endif

namespace Femera {
  Pomp::Pomp (Proc* P,Data* D) {this->proc=P; this->data=D; this->log=proc->log;
    this-> work_type = work_cast (Plug_type::Pomp);
//    this-> base_type = work_cast (Base_type::Proc);
    this-> task_name ="OpenMP";
    this-> verblevel = 2;
#ifdef FMR_HAS_MPI
    this-> hier_lv   = 2;
#else
    this-> hier_lv   = 1;
#endif
  }
  int Pomp::prep (){
    this-> version = std::to_string( _OPENMP );
    this-> set_proc_n (omp_get_num_procs ());
    //omp_get_max_threads () includes logical cores at this point
    return 0;
  }
  int Pomp::run (Sims* F) {int err=0;//TODO fix err/return code handling.
    auto flog = F->proc->log;
    auto parent_sims = F->parent;// parent sims is shared among OpenMP threads
    if (!this->is_in_parallel () && this->get_hier_lv () >= F->from.hier_lv) {
      bool do_start_parallel = false;
      switch (F->from.cncr) {
        case fmr::Concurrency::Independent :{}// Fall through.
        case fmr::Concurrency::Collective  :{do_start_parallel = true; break;}
        default: {}// Do nothing.
      }
      if (do_start_parallel) {
        if (flog->detail >= F->verblevel) {
          flog->label_fprintf (flog->fmrout, (F->task_name+" init").c_str(),
            "Starting %s parallel...\n", this->task_name.c_str());
        }
        FMR_PRAGMA_OMP(omp parallel reduction(+:err))
        { err+= this->run (F); }//TODO or run (this)?
    } }
    else{
      fmr::Local_int pn  = 1;
      fmr::Local_int pid = 0;
      switch (F->from.cncr) {
        case fmr::Concurrency::Independent :{}// Fall through.
        case fmr::Concurrency::Collective  :{
          pn = this->get_proc_n  ();
          pid= this->get_proc_id ();
          break;}
        default :{}// Do nothing.
      }
      const fmr::Local_int nmodel
        = fmr::Local_int (parent_sims->model_list.size ());
      std::string liststr = " block list";
      switch (F->from.plan) {
        case fmr::Schedule::Once : liststr=" one only"; [[fallthrough]];
        //nmodel=1;
        case fmr::Schedule::List : liststr=" preset list"; pn=1;//TODO Check.
          [[fallthrough]];
        case fmr::Schedule::Block :{
          for (fmr::Local_int i=pid+1; i<nmodel; i+=pn) {
            err= fmr::detail::main->add_new_task (F->work_type, parent_sims);
          }
          for (fmr::Local_int i=pid*pn; i<nmodel; i++) {
            auto runner = parent_sims->task.get<Sims>(i); if (runner) {
              runner->model_name = parent_sims->model_list[i];
              if (flog->verbosity >= F->verblevel) {
                flog->label_fprintf (flog->fmrout,
                (runner->task_name+liststr).c_str(),
                "%s...\n",runner->model_name.c_str());
              }
              err+= runner->run () ? 1 : 0;
          } }
          break; }
        case fmr::Schedule::Interleave :{
          for (fmr::Local_int i=pid+1; i<nmodel; i+=pn) {
            err= fmr::detail::main->add_new_task (F->work_type, parent_sims);
          }
          for (fmr::Local_int i=pid; i<nmodel; i+=pn) {
            auto runner = parent_sims->task.get<Sims>(i); if (runner) {
              runner->model_name = parent_sims->model_list[i];
              if (flog->verbosity >= F->verblevel) {
                flog->label_fprintf (flog->fmrout,
                (runner->task_name+" interleaved list").c_str(),
                "%s...\n",runner->model_name.c_str());
              }
              err+= runner->run () ? 1 : 0;
          } }
          break; }
        case fmr::Schedule::Fifo :{
          while (!parent_sims->model_list.empty()){
            Sims* runner=nullptr;
            FMR_PRAGMA_OMP(omp critical)//TODO May not work if not in_parallel
            if (!parent_sims->model_list.empty()){// shared among OpenMP threads
              //TODO accumulate local err so loop can continue.
              runner = parent_sims->task.get<Sims>(0);
              if (runner) {runner->model_name = parent_sims->model_list.front(); }
              else {//err+= 1;
                flog->label_fprintf (flog->fmrerr, "WARN""ING Pomp",
                "run %s added %s as nullptr\n",
                F->task_name.c_str(), parent_sims->model_list.front().c_str());
              }
              parent_sims->model_list.pop_front();
              if (!parent_sims->model_list.empty()){
                fmr::detail::main->add_new_task (F->work_type, parent_sims);
            } }//end critical shared model name queue region
            if (runner) {
              if (flog->verbosity >= F->verblevel) {
                flog->label_fprintf (flog->fmrout,
                (runner->task_name+" FIFO").c_str(),
                "%s...\n",runner->model_name.c_str());
              }
              err+= runner->run () ? 1 : 0;
          } }
          //
          break; }
        default:{
          flog->label_fprintf (flog->fmrerr, "* ERR""OR Pomp",
            "%s\n","run (Sims %s) unknown Schedule::%i",
            F->task_name.c_str(),int(F->from.plan) );
        }
      }//end Schedule switch
     //...
    }
    return err;
  }
bool Pomp::is_in_parallel (){
  return omp_in_parallel ();
}
int Pomp::get_proc_id (){
  return omp_get_thread_num ();
}
int Pomp::get_proc_n (){
  return this->proc_n;
}
int Pomp::set_proc_n (int n){
  omp_set_num_threads (n);// may not succeed...
  this-> proc_n = omp_get_max_threads ();//...set to actual
  return this->proc_n;
}
bool Pomp::is_master (){bool this_is_master=true;
#if 0
  for(uint i=0; i<this->proc_stack.size(); i++){
    if( this->proc_stack[i] != this){
      this_is_master &= this->proc_stack[i]->is_master(); }
  }
#endif
  this_is_master &= omp_get_thread_num () == this-> my_master;
  return this_is_master;
}
int Pomp::exit_task (int err){return err;
}
int Pomp::init_task (int* argc, char** argv){int err=0;
  fmr::perf::timer_resume (&this->time);
  err = this->prep ();
#ifdef FMR_DEBUG
  std::printf("Pomp::init_task start...\n");
#endif
  FMR_PRAGMA_OMP(omp critical) {//NOTE getopt is NOT thread safe.
    int argc2=argc[0];// Copy getopt variables.
    auto opterr2=opterr; auto optopt2=optopt;
    auto optind2=optind; auto optarg2=optarg;
    opterr = 0; int optchar;
    int omp_n = this->get_proc_n ();
    while ((optchar = getopt (argc[0], argv, "n:")) != -1){
      // n:  -n requires an argument
      switch (optchar){
        case 'n':{ omp_n = atoi (optarg); this->proc->opt_add ('n'); break; }
    } }
    this->set_proc_n (omp_n);
  #ifdef FMR_DEBUG
    std::printf("Pomp::init_task: set OpenMP threads to %i\n", omp_n );
  #endif
    // Restore getopt variables.
    argc[0]=argc2; opterr=opterr2; optopt=optopt2; optind=optind2; optarg=optarg2;
  }
  fmr::perf::timer_pause (&this->time);
  return err;
}
}// end Femera namespace
#undef FMR_DEBUG
