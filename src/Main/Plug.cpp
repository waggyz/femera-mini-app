//TODO Rename to Libs.cpp?
#include "Plug.hpp"
#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
  Plug::~Plug(){ }
  Plug:: Plug(Proc* P,Data* D){this->proc=P; this->data=D;
    this->work_type = work_cast (Base_type::Plug);
//    this->base_type = work_cast (Base_type::Main);
    this->task_name ="Core";
    this->verblevel = 1;
    this->task.add (this);
  }
  int Plug::prep (){//Register processing environment and file handling drivers.
    // The new Work:: objects below will be destroyed by proc->exit(err).
#ifdef FMR_HAS_MPI
    this->add<Proc>(new Pmpi(this->proc,this->data), this->proc);
#endif
#ifdef _OPENMP
    this->add<Proc>(new Pomp(this->proc,this->data), this->proc);
#endif
#ifdef FMR_HAS_GTEST
    this->add<Proc>(new Gtst(this->proc,this->data), this->proc);
#endif
    // below will be destroyed by data->exit(err).
    this->add<Data>(new Fake(this->proc,this->data), this->data);
#ifdef FMR_HAS_GMSH
    this->add<Data>(new Dmsh(this->proc,this->data), this->data);
#endif
#ifdef FMR_HAS_CGNS
    this->add<Data>(new Dcgn(this->proc,this->data), this->data);
#endif
#if 0
#ifdef FMR_HAS_HDF5
    this->loaded_names.push_back("HDF5");
#endif
#endif
    return 0;
  }
  int Plug::init_task (int* argc, char** argv){int err=0;
#ifdef FMR_DEBUG
    std::printf("***Plug::init_task()...\n");
#endif
    err= this->prep ();
    fmr::perf::timer_pause (&this->time);
    fmr::perf::timer_pause (&this->proc->time);
    // Initialize processing environment.
    err= proc->init (argc,argv);
    if (err){
      if (err<0){ this->proc->log->verbosity = 0; }
      proc->exit (err);
      return err;
    }
    fmr::perf::timer_resume(&this->time);
    fmr::perf::timer_pause (&this->time);
    // Print available lib names.
    std::string info("");
    size_t i=0;
    for (auto name : loaded_names){i++;
      info += name;
      if (i < this->loaded_names.size()){
        info += std::string(" ");
    } }
    info += std::string("\n");
    this->proc->log->label_printf ((this->task_name+" mods").c_str(),
      info.c_str());
    return err;
  }
  int Plug::exit_task (int err){
#ifdef FMR_DEBUG
    std::printf("Plug::exit(err)...\n");
#endif
    fmr::perf::timer_pause (& this->time);
    fmr::perf::timer_resume(& this->time);
    int exit_err = (err<0) ? 0 : err;// err<0: no error, but exit immediately
    if (this->proc){
      if (err) {this->proc->log->verbosity=0; this->proc->log->detail=0; }
      const int iprint = this->proc->is_master ();// for use after proc->exit()
      int proc_err = this->proc->exit (err);
      if (err<=0 && proc_err && iprint){
        fprintf (stderr,"ERROR %i returned by Main->proc->exit(%i)\n",
          proc_err, err);
        if (!exit_err){ exit_err=proc_err; }
    } }
#ifdef FMR_HAS_MPI
    exit_err=0;// Exit from mpi normally when Femera exits on error.
#endif
    fmr::perf::timer_pause (& this->time);
    return err;
  }
  int Plug::add_new_task (Femera::Base_type type, Work* add_to){
    return this->add_new_task (work_cast (type), add_to);
  }
  int Plug::add_new_task (Femera::Plug_type type, Work* add_to){
    return this->add_new_task (work_cast (type), add_to);
  }
  int Plug::add_new_task (Femera::Work_type type, Work* add_to){
    Work* W = nullptr;
    switch (int(type)) {
      case int(Femera::Base_type::Frun) :{
        W = new Frun (static_cast<Sims*>(add_to));
        break;}
//      case int(Femera::Base_type::Part) :{
//        Sims* F = static_cast<Sims*>(add_to);
//        if (F) { W = new Part (F, F->get_part_n()); }
//        break;}
      case int(work_cast(Femera::Plug_type::Mesh)) :{
        W = new Mesh (static_cast<Sims*>(add_to));
        break;}
      default: {
        this->proc->log->label_fprintf(this->proc->log->fmrerr,"WARNING Plug",
          "add_new_task: unknown Work_type::%i\n", int(type));
        return 1;}
    }
#ifdef FMR_DEBUG
    if (W){this->proc->log->label_fprintf (this->proc->log->fmrout,"* ADD Plug",
      "%s to %s\n",W->task_name.c_str(), add_to->task_name.c_str());}
#endif
    if (W){ this->add<Work> (W, add_to); }else{ return 1; }
    return 0;
  }
}// end Femera namespace
#ifdef FMR_HAS_GTEST
int fmr::run_all_tests(){int err=0;
  auto P=fmr::detail::main->proc->task.first<Femera::Proc>
    (Femera::Plug_type::Gtst);
  if (P){err=P->chck(); }
  return err;
}
#else
int fmr::run_all_tests(){ return 0; }//TODO Warn about no tests?
#endif
#if 0
//int fmr::plug:: init (int* argc, char** argv){int err=0;
int fmr::plug:: init (int* , char** ){int err=0;
  //fmr::detail::main->plug
  //  = new Femera::Plug(fmr::detail::main->proc,fmr::detail::main->data);
 // err=fmr::detail::main->plug->prep();
  if(err){return(err); }
//  err=fmr::detail::main->plug->init(argc,argv);
  return err;
}
int fmr::plug:: exit (int err ){
//  err=fmr::detail::main->plug->exit(err);
#if 0
  if(fmr::detail::main->plug){
    delete fmr::detail::main->plug; fmr::detail::main->plug=nullptr;
  }else{
    err=1;
  }
#endif
  return err;
}
#endif
#undef FMR_DEBUG
