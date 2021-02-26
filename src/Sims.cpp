#include "base.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Sims::Sims (Proc* P, Data* D) noexcept{ this->proc=P; this->data=D;
    this->work_type = work_cast (Base_type::Sims);
    this->task_name ="Sims";
    this->verblevel = 6;
    this->meter_unit="sim";
    //this-> parent = this;//TODO leave as nullptr?
//    this->part_dims.memory_state.can_read = true;//TODO remove
    this->         data_id = this->make_id ();
  }
  fmr::Data_id Sims::make_id (){
      std::vector<fmr::Local_int> path={};
      Sims* F=this;
      this->sims_lv = 0;
      while (F->work_type == work_cast(Base_type::Sims)
        && parent != this && parent != nullptr
        && this->sims_lv < this->data->get_hier_max()) {
        // Walk up the Sims hierarchy to the root Sims instance.
        this->sims_lv++;
        path.push_back (F->sims_ix);
        F = F->parent;
      }
      this->data_id
        = this->data->make_data_id (F->model_name, fmr::Tree_type::Sims, path);
    return this->data_id;
  }
  fmr::Data_id Sims::get_id (){
    return this->data_id;
  }
  fmr::Local_int Sims::get_sims_n (){
    return this->task.count (Base_type::Frun);// zero-indexed
  }
  fmr::Local_int Sims::get_part_n (){// zero-indexed
    this->part_dims.data[enum2val(fmr::Geom_info::Part_n)]
      = this->task.count (Base_type::Part);
    return this->part_dims.data[enum2val(fmr::Geom_info::Part_n)] ;
  }
#if 0
  fmr::Dim_int Sims::get_phys_d (std::string sim_name, fmr::Tree_path part){
    Sims* P=this;//TODO look up sim_name
    const auto n = part.size();
    for (size_t i=0; i<n; i++) {
      if (P) {
        if (part[i] < P->task.count()) {P = P->task.get<Sims> (part[i]);}
      }else{
      return 1;
    } }
    if (!fmr::data::is_loaded_ok (P->part_dims.memory_state)) {
      //TODO call overloaded P->read_part_dims();
    }
    if (!P->part_dims.memory_state.has_error){
      return fmr::Dim_int(
        P->part_dims.data[enum2val(fmr::Geom_info::Phys_d)]);
    }
    return 0;
  }
  fmr::Dim_int Sims::get_geom_d (std::string sim_name, fmr::Tree_path part){
    Sims* P=this;
    const auto n = part.size();
    for (size_t i=0; i<n; i++) {
      if (P) {
        if (part[i] < P->task.count()) {P = P->task.get<Sims> (part[i]);}
      }else{
      return 1;
    } }
    if (!fmr::data::is_loaded_ok (P->part_dims.memory_state)) {
      //TODO call overloaded P->read_part_dims();
    }
    if (!P->part_dims.memory_state.has_error){
      return fmr::Dim_int(
        P->part_dims.data[enum2val(fmr::Geom_info::Geom_d)]);
    }
    return 0;
  }
  fmr::Local_int Sims::get_part_n (std::string sim_name, fmr::Tree_path part){
    Sims* P=this;
    const auto n = part.size();
    for (size_t i=0; i<n; i++) {
      if (P) {
        if (part[i] < P->task.count()) {P = P->task.get<Sims> (part[i]);}
      }else{
      return 1;
    } }
    if (!fmr::data::is_loaded_ok (P->part_dims.memory_state)) {
      //TODO call overloaded P->read_part_dims();
    }
    if (!P->part_dims.memory_state.has_error){
      return fmr::Local_int(
        P->part_dims.data [enum2val(fmr::Geom_info::Part_n)]);
    }
    return 0;
  }
  fmr::Local_int Sims::get_mtrl_n (std::string sim_name, fmr::Tree_path part){
    Sims* P=this;
    const auto n = part.size();
    for (size_t i=0; i<n; i++) {
      if (P) {
        if (part[i] < P->task.count()) {P = P->task.get<Sims> (part[i]);}
      }else{
      return 1;
    } }
    if (!fmr::data::is_loaded_ok (P->part_dims.memory_state)) {
      //TODO call overloaded P->read_part_dims();
    }
    if (!P->part_dims.memory_state.has_error){
      return fmr::Local_int(
        P->part_dims.data [enum2val(fmr::Geom_info::Mtrl_n)]);
    }
    return 0;
  }
//#else
  fmr::Dim_int Sims::get_phys_d (std::string, fmr::Tree_path){
    return 0;
  }
  fmr::Dim_int Sims::get_geom_d (std::string, fmr::Tree_path){
    return 0;
  }
  fmr::Local_int Sims::get_part_n (std::string, fmr::Tree_path){
    return 0;
  }
  fmr::Local_int Sims::get_mtrl_n (std::string, fmr::Tree_path){
    return 0;
  }
#endif
  int Sims::chck (){
    return 0;
  }
  int Sims::prep (){int err=0;
    return err;
  }
  int Sims::init_task (int*, char**){int err=0;
#ifdef FMR_DEBUG
    printf ("*** Sims::init_task\n");
#endif
    return err;
  }
  int Sims::add (std::string name){int err=0;
    if (this->model_list.size() == 0){// Add the first Frun
      // New instances made by add_new_task will be deleted in this->exit (err),
      // or before.
      err= fmr::detail::main->add_new_task (Femera::Base_type::Frun,this);
    }
    this->model_list.push_back (name);
    return err;
  }
  int Sims::clear (){
    this->model_list={};
    return 0;
  }
  int Sims::run (){int err=0;
    if (this->proc->log->detail >= this->verblevel) {
      this->proc->log->print_heading ("Start");
    }
    //TODO prep?
    fmr::perf::timer_resume (& this->time);
    Proc* P = this->proc->hier[dist_to_hier_lv];
    if(P){
      fmr::Local_int m = fmr::Local_int (this->model_list.size ());
      if (this->proc->log->detail >= this->verblevel) {
        const int p = P->get_proc_n ();
        fmr::Local_int c = 0;
        switch(this->dist_to_cncr){
          case fmr::Concurrency::Once        :{ c = 1; break; }
          case fmr::Concurrency::Serial      :{ c = 1; break; }
          case fmr::Concurrency::Independent :{ c = p; break; }
          case fmr::Concurrency::Collective  :{ c = p; break; }
          default: {}
        }
        const std::string label = "Run "+std::to_string(m)+" sims";
        this->proc->log->label_fprintf (this->proc->log->fmrout, label.c_str(),
          "%i %s / %i %s %s, %s...\n",
          c, (c==1) ? "sim":"sims",
          p, P->task_name.c_str(),
#if 0
          fmr::Concurrency_name[this->dist_to_cncr].c_str(),//TODO
          fmr::Schedule_name[this->dist_to_plan].c_str());
#else
          fmr::Concurrency_name.at(this->dist_to_cncr).c_str(),//TODO
          fmr::Schedule_name.at(this->dist_to_plan).c_str());
#endif
      }
      fmr::perf::timer_pause  (& this->time, m);
      fmr::perf::timer_resume (& this->time);
      Sims* run0 = this->task.first<Sims> (Base_type::Frun);
      if (run0) {
        err= P->run (run0);//TODO fix return code handling
      }else{
        this->proc->log->label_fprintf (this->proc->log->fmrerr,"WARN""ING",
          "First Frun task in sims is null.\n");
    } }
    fmr::perf::timer_pause  (& this->time);
  //...
  if (this->proc->log->detail >= this->verblevel) {
    this->proc->log->print_heading ("Finished");
  }
  fmr::perf::timer_pause (& this->time, -err);//TODO fix return code handling
  //
  return this->exit (err);
}
  int Sims::exit_task (int err ){//TODO Why called twice?
#ifdef FMR_DEBUG
    printf ("*** Sims::exit_task\n");
#endif
#if 0
    if (this->proc->log->detail >= this->verblevel) {
      this->proc->log->print_heading ("D_one");
    }
#endif
    return err;
  }
}// end Femera namespace
#undef FMR_DEBUG
