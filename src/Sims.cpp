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
    this->data_id = this->make_id ();
  }
  fmr::Data_id Sims::make_id (){
      std::vector<fmr::Local_int> path={};
      Sims* F=this;
      this->tree_lv = 0;
        // Walk up the Sims hierarchy to the root Sims instance.
      while (F->work_type == work_cast(Base_type::Sims)
        && parent != this && parent != nullptr
        && this->tree_lv < this->data->get_hier_max()) {
        this->tree_lv++;
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
  fmr::Local_int Sims::get_sims_n (){// zero-indexed
    return this->task.count (Base_type::Sims);
  }
  fmr::Local_int Sims::get_part_n (){//TODO Remove.
    return this->task.count (Base_type::Part);
  }
#if 0
  fmr::Local_int Sims::get_sims_n () {// number of models in this collection
    // Valid after this->model_list has been populated.
    bool is_new=false;
    //TODO was: fmr::Local_int n = this->task.count (Base_type::Frun);
    if (this->locals.count(fmr::Data::Sims_n)) {// already in map
      auto item = & this->locals.at(fmr::Data::Sims_n);
      if (item->data.size() < 1) {
        is_new = true;
        item->data.resize (1,fmr::Local_int (this->model_list.size ()));
      }else{
        if (item->data[0] <1) {
          is_new = true;
          item->data[0] = fmr::Local_int (this->model_list.size ());
    } } }
    else {// Add to map and initialize data.
      is_new = true;
      this->locals[fmr::Data::Sims_n].data.resize (1,//TODO allow >1 collection
        fmr::Local_int (this->model_list.size ()));
    }
    const fmr::Local_int n = this->locals.at(fmr::Data::Sims_n).data[0];
    if (is_new) {
      auto store = & this->locals.at(fmr::Data::Sims_n).stored_state;
      // number of sims in a collection not stored directly in a file
      store->is_default  = true;
      store->can_write   = false;
      store->can_read    = false;
      store->was_read    = false;
      store->was_checked = true;
      store->has_changed = false;
      store->do_save     = false;
      store->has_error   = n < 1;
      auto inmem = & this->locals.at(fmr::Data::Sims_n).memory_state;
      inmem->is_default  = true;
      inmem->can_write   = false;//TODO change # sims dynamically?
      inmem->can_read    = true;
      inmem->was_read    = true;
      inmem->was_checked = true;
      inmem->has_changed = false;
      inmem->do_save     = false;
      inmem->has_error   = n < 1;
    }
    return n;// zero-indexed
  }
  fmr::Local_int Sims::get_part_n (){// zero-indexed
    this->part_dims.data[enum2val(fmr::Geom_info::Part_n)]
      = this->task.count (Base_type::Part);
    return this->part_dims.data[enum2val(fmr::Geom_info::Part_n)] ;
  }
#endif
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
  int Sims::chck () {
    return 0;
  }
  int Sims::prep () {int err=0;
    return err;
  }
  int Sims::init_task (int*, char**) {int err=0;
#ifdef FMR_DEBUG
    printf ("*** Sims::init_task\n");
#endif
    return err;
  }
  int Sims::add (std::string name) {int err=0;
    if (this->model_list.size() == 0) {// Add the first Frun.//TODO Remove.
      // New instances made by add_new_task will be deleted in this->exit(err),
      // or before.
      err= fmr::detail::main->add_new_task (Femera::Base_type::Frun, this);
    }
    this->model_list.push_back (name);
    return err;
  }
  int Sims::clear () {
    this->model_list ={};
    this->globals ={};
    this->locals ={};
    this->enums ={};
    this->dims ={};
#if 0
    if (this->locals.count(fmr::Data::Sims_n)) {// present in map
      this->locals.at(fmr::Data::Sims_n)
        = fmr::Local_int_vals(fmr::Data::Sims_n);
    }
#endif
    return 0;
  }
  int Sims::run () {int err=0;
    auto log = this->proc->log;
    if (log->detail >= this->verblevel) {log->print_heading ("Start");}
    //TODO this->prep() ?
    fmr::perf::timer_resume (& this->time);
    Proc* P = this->proc->hier[this->send.hier_lv];
    if (P) {
#if 0
      const fmr::Local_int m = this->get_sims_n ();
#else
      fmr::Local_int m = fmr::Local_int (this->model_list.size ());
#endif
      if (log->detail >= this->verblevel) {
        const int p = P->get_proc_n ();
        fmr::Local_int c = 0;
        switch (this->send.cncr) {
          case fmr::Concurrency::Once        :// Fall through.
          case fmr::Concurrency::Serial      : c = 1; break;
          case fmr::Concurrency::Independent :// Fall through.
          case fmr::Concurrency::Collective  : c = p; break;
          default: {}// Do nothing.
        }
        const std::string label = "Run "+std::to_string(m)+" sims";
        log->label_fprintf (log->fmrout, label.c_str(),
          "%i %s / %i %s %s, %s...\n",
          c, (c==1) ? "sim":"sims", p, P->task_name.c_str(),
          fmr::Concurrency_name.at(this->send.cncr).c_str(),
          fmr::Schedule_name.at(this->send.plan).c_str());
      }
      fmr::perf::timer_pause  (& this->time, m);
      fmr::perf::timer_resume (& this->time);
      Sims* run0 = this->task.first<Sims> (Base_type::Frun);
      if (run0) {
        err= P->run (run0);//TODO fix return code handling
      }else{
        log->label_fprintf (log->fmrerr,"WARN""ING",
          "First Frun task in sims is null.\n");
    } }
    fmr::perf::timer_pause  (& this->time);
  //...
  if (log->detail >= this->verblevel) {log->print_heading ("Finished"); }
  fmr::perf::timer_pause (& this->time, -err);//TODO Fix return code handling.
  //
  return this->exit (err);
}
  int Sims::exit_task (int err) {//TODO Why called twice?
#ifdef FMR_DEBUG
    printf ("*** Sims::exit_task\n");
#endif
#if 0
    if (log->detail >= this->verblevel) {
      log->print_heading ("D_one");
    }
#endif
    return err;
  }
}// end Femera namespace
#undef FMR_DEBUG
