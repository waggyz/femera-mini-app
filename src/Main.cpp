#include "Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
  Main::~Main (){
    if (this->data){ delete this->data;this->data=nullptr; }
    if (this->proc){ delete this->proc;this->proc=nullptr; }
//    if (this->sims){ delete this->sims;this->sims=nullptr; }
  }
  int Main:: prep (){int err=0;
#ifdef FMR_DEBUG
    std::printf("*** Main::prep()...\n");
#endif
    this-> work_type = work_cast(Base_type::Main);
    this-> task_name ="Main";
    this-> verblevel = 5;
    //TODO change to unique_ptr or shared_ptr
    // For now, make new Proc and Data, and delete them in destructor.
    this-> proc =new Proc ();
    this-> data =new Data (proc);
    // The rest are deleted in task.free_stack().
    this-> task.add (new Plug (proc, data));//TODO need one/core?
    this-> task.add (new Sims (proc, data));
    this-> task.add (this);
    return err;
  }
  int Main::add_model_name (std::string model){
    return this->task.first<Sims>(Base_type::Sims)->add (model);
  }
  int Main::clear (){
    return this->task.first<Sims>(Base_type::Sims)->clear ();
  }
#if 0
  int Main::run (const std::deque<std::string> models){
    auto F = this->task.first<Sims> (Base_type::Sims);
#if 0
    if (F) {return F.run (models); }
#else
    if (F) {this->proc->log->printf ("Main::run (%i models) in main\n",
      int(models.size()));
    return 0;
    }
#endif
    this->proc->log->printf_err ("ERR""OR Base sims module not loaded.\n");
    return 1;
  }
  int Main::run (){
    return this->run (this->model_list);
  }
#endif
  int Main::run (){
    return this->task.first<Sims>(Base_type::Sims)->run ();
  }
  int Main::add_new_task (Femera::Base_type type, Work* add_to){
#ifdef FMR_DEBUG
    this->proc->log->label_fprintf (this->proc->log->fmrout, "** ADD Main",
      "Base_type::%i to %s\n",int(type),add_to->task_name.c_str());
#endif
    return this->task.first<Main> (Base_type::Plug)
      ->add_new_task (type, add_to);
  }
  int Main::add_new_task (Femera::Plug_type type, Work* add_to){
    return this->task.first<Main> (Base_type::Plug)
      ->add_new_task (type, add_to);
  }
  int Main::add_new_task (Femera::Work_type type, Work* add_to){
    return this->task.first<Main> (Base_type::Plug)
      ->add_new_task (type, add_to);
  }
  int Main::init_task (int* argc, char** argv){int err=0;
#ifdef FMR_DEBUG
    std::printf("Main::init_task (..)\n");
#endif
    fmr::perf::timer_pause (&this->time);
    if (!this->task.first<Main> (Base_type::Plug)){
      err= proc->init (argc,argv);
      if (err){
        if (err<0){ this->proc->log->verbosity = 0; }
        proc->exit (err);
        return err;
    } }
#if 0
    if (this->proc->log->detail >= this->verblevel) {
      this->proc->log->print_heading ("Start");
    }
#endif
//    err= this->sims->init (argc,argv);
    return err;
  }
  int Main::exit_task (int err){
    fmr::perf::timer_pause (&this->time);
    int exit_err = (err<0) ? 0 : err;// err<0: no error, but exit immediately
#ifdef FMR_DEBUG
    std::printf("Main::exit (%i)...\n",err);
#endif
#if 0
    if (this->proc->log->detail >= this->verblevel) {
      this->proc->log->print_heading ("Exit");
    }
#endif
#if 0
    if( this->proc->log-> timing > 2 ){
      fmr::perf::timer_pause (& this->time, proc-> redo_n * sims_n );
      //proc-> barrier ();
      if( this->proc->log-> verbosity > 1 ){
        if( this->proc->log-> detail > 1 ){
          this->proc->log-> printf ("\n");
          this->proc->log-> print_heading ("E_xit");
        }
#if 1
        std::string label = std::to_string( this->time.count ) +" Sims time";
        this->proc->log-> label_printf( label.c_str(),
          "%.3f sc on master thread\n",
          double( this->time.last_busy_ns )* 1e-9 );
#endif
        double solv_per_sec = fmr::perf::overall_speed ( this->time );
        double sec_per_solv = 1.0 / solv_per_sec;
        double t=sec_per_solv;
        std::string f="";
        if     ( sec_per_solv <   1.0){f="%.0f ms/sim (%.1f sim/sc)\n"; t*=1e3; }
        else if( sec_per_solv < 120.0){f="%.1f sc/sim\n"; }
        else if( sec_per_solv <7200.0){f="%.1f mn/sim\n"; t/=   60.0; }
        else                          {f="%.1f hr/sim\n"; t/= 3600.0;
        }
        if( sec_per_solv < 1 ){
          this->proc->log-> label_printf ("Overall perf", f.c_str(), t, solv_per_sec );
        }else{
          this->proc->log-> label_printf ("Overall perf", f.c_str(), t );
      } }
    }
#endif
    //int verb = 0, iprint = 0;// std::basic_string<char> label_format = "";
    if (this->proc){
      if (proc->log){ this->proc->log->print_log_time ("time"); }
      if (err){ this->proc->log->verbosity=0; this->proc->log->detail=0; }
#if 1
      if (this->proc->log->detail > 0){this->proc->log->print_heading ("Exit");}
#endif
//      if (this->sims){ err=this->sims->exit (err); }
      // Keep these for use after proc->exit(err) when
      // proc->log-> printf() is no longer available.
      //verb = this->proc->log->verbosity;
      int iprint = this->proc->is_master ();
      //label_format = "%"+ std::to_string(this->proc->log->label_width) +"s";
      int proc_err=this->proc->exit (err);
      if (err<=0 && proc_err && iprint){
        fprintf (stderr,"ER""ROR %i returned by Main->proc->exit(%i)\n",
          proc_err,err);
        if (!exit_err){ exit_err = proc_err; }
    } }
    fmr::perf::timer_pause (&this->time);
    return exit_err;
  }
}// end Femera namespace

Femera::Main* fmr::detail::main=nullptr;// singleton instance
// If fmr::detail::main made new here, delete it in fmr::detail::main->exit(err)

int fmr::init (int* argc, char** argv){int err=0;
  fmr::detail::main = new Femera::Main();// delete is automatic on program exit.
  err= fmr::detail::main->init (argc,argv);
  if(!err){ fmr::detail::main->data->init (argc,argv); }
#if 0
  if(!err){ fmr::sims::run (); }
  fmr::sims::clear ();
  fmr::detail::main->data->clear ();
#endif
  return err;
}
int fmr::exit (int err){
  int test_err= fmr::run_all_tests();
  err = err || test_err;
  if (fmr::detail::main->data){
    int data_err= fmr::detail::main->data->exit (err);
    if (data_err){ err= data_err; }
  }
  return fmr::detail::main->exit (err);
  //NOTE Do NOT: delete fmr::detail::main; fmr::detail::main=nullptr;
  // when it is made new in fmr::detail::init(..), because the
  // fmr::detail::main destructor is called automatically before program exit.
}
int fmr::sims::add (const std::string model){
  return fmr::detail::main->add_model_name (model);
}
int fmr::sims::add (const std::deque<std::string> model_names){int err=0;
  for (auto model : model_names){
    err= fmr::sims::add (model);// Copy model names to the main queue
    if (err) {return err;}
  }
  return err;
}
int fmr::sims::clear (){
  fmr::detail::main->clear ();
  return 0;
}
int fmr::sims::run (){
  return fmr::detail::main->run ();
}
#undef FMR_DEBUG
