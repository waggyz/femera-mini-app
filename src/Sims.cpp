#include "base.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Sims::Sims (Proc* P, Data* D) noexcept{ this->proc=P; this->data=D;
    this->work_type = work_cast (Base_type::Sims);
    this->task_name ="Sims";
    this->verblevel = 2;
    this->meter_unit="sim";
    //this-> parent = this;//TODO leave as nullptr or set to this?
  }
  fmr::Local_int Sims::get_sims_n (){// number of collections loaded
    return this->task.count (Base_type::Sims);
  }
  fmr::Local_int Sims::get_frun_n (){// number of collections loaded
    return this->task.count (Base_type::Frun);
  }
  fmr::Local_int Sims::get_part_n (){//TODO Remove.
    return this->task.count (Base_type::Part);
  }
  int Sims::chck () {
    return 0;
  }
  int Sims::init_task (int*, char**) {int err=0;
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    if (log->verbosity >= this->verblevel) {
      std::string label = this->task_name+" init";
      log->label_fprintf (log->fmrout, label.c_str(), "***\n");
    }
#endif
    return err;
  }
  int Sims::add (std::string name) {int err=0;
    this->model_list.push_back (name);
#if 0
    if (this->model_list.size() == 1) {// Add the first Frun.
      err= fmr::detail::main->add_new_task (Femera::Base_type::Frun, this);
      // New instances made by add_new_task will be deleted in this->exit(err),
      // or before.
    }
#endif
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
  int Sims::prep () {int err=0;
    const auto log = this->proc->log;
    const auto Pfrom = this->proc->hier[this->from.hier_lv];
    const auto Psend = this->proc->hier[this->send.hier_lv];
    if (Pfrom && Psend) {
      fmr::perf::timer_resume (& this->time);
      if (this->model_name.size() == 0) {
      this->model_name = "fmr:Sims:"+ Pfrom->task_name +"_"
        + std::to_string (Pfrom->get_proc_id ());
      }
      const auto sim_n = fmr::Local_int (this->model_list.size());
      //
      this->from.bats_sz = sim_n;//TODO for XS sims only?
      //
      const auto sendp = (this->send.hier_lv > this->from.hier_lv)
        ? Psend->get_proc_n () : 1;       // # threads sent to
      const auto bsz = this->send.bats_sz;// # items sent in each batch
      fmr::Local_int sendn = 0;           // # items to run concurrently
      switch (this->send.cncr) {
        case fmr::Concurrency::Once        : sendn = 1;     break;
        case fmr::Concurrency::Serial      : sendn = sendp; break;
        case fmr::Concurrency::Independent : sendn = sendp; break;
        case fmr::Concurrency::Collective  : sendn = 1;     break;
        default :{}// Do nothing.
      }
      const auto send_type
        = work_cast ((bsz>1) ? Base_type::Sims : Base_type::Frun);
      for (fmr::Global_int i=0; i < (sendp*bsz); i++) {// add initial tasks
        err= fmr::detail::main->add_new_task (send_type, this);
      }
      if (log->verbosity >= this->verblevel) {
        fmr::perf::timer_pause (& this->time);
        if (log->detail >= this->verblevel) {
          std::string label ="";
#if 0
          label = this->task_name+" prep send";
          log->label_fprintf (log->fmrout, label.c_str(),
            "init %i sets, %i runs\n",this->get_sims_n (), this->get_frun_n ());
#endif
          const auto fromp = Pfrom->get_proc_n ();
          fmr::Local_int fromn = 0;
          switch (this->from.cncr) {
            case fmr::Concurrency::Once        : fromn = 1;     break;
            case fmr::Concurrency::Serial      : fromn = fromp; break;
            case fmr::Concurrency::Independent : fromn = fromp; break;
            case fmr::Concurrency::Collective  : fromn = 1;     break;
            default :{}// Do nothing.
          }
          const fmr::Local_int bn = fmr::math::divide_ceil (sim_n, bsz);
          label = this->task_name+" prep from";
          const auto fbsz = this->from.bats_sz;
          const fmr::Local_int fbn = fmr::math::divide_ceil (sim_n, fbsz);
          log->label_fprintf (log->fmrout, label.c_str(),
            "%i set%s /  %i %s %s, %s\n",
            fromn, (fromn==1)?" ":"s", fromp, Pfrom->task_name.c_str(),
            fmr::get_enum_string (fmr::Concurrency_name, from.cncr).c_str(),
            fmr::get_enum_string (fmr::Schedule_name, this->from.plan).c_str());
          const auto szshort = fmr::get_enum_string (fmr::Sim_size_short,
            this->sims_size);
          if (fbsz>1) {
            log->label_fprintf (log->fmrout, label.c_str(),
              "%u set%s of %u %s sim%s in%s batch%s\n",
              fbn, (fbn==1)?" ":"s", fbsz, szshort.c_str(),
              (fbsz==1)?"":"s", (fbn<=1)?"":" each",
              (fbn<=1)?(" "+this->model_name).c_str():"");
          }else{
            log->label_fprintf (log->fmrout, label.c_str(),
              "%u %s sim%s in batch %s\n",
              sim_n, szshort.c_str(), (sim_n==1)?" ":"s",
              this->model_name.c_str());
          }
          label = this->task_name+" prep send";
          if (bsz>1) {
            log->label_fprintf (log->fmrout, label.c_str(),
              "%u set%s of %u sim%s in each batch\n",
              bn, (bn==1)?" ":"s", bsz, (bsz==1)?"":"s");
          }
          log->label_fprintf (log->fmrout, label.c_str(),
            "%i %s%s /  %i %s %s, %s\n",
            sendn, (bsz>1)?"set":"sim", (sendn==1)?" ":"s",
            sendp, Psend->task_name.c_str(),
            fmr::get_enum_string (fmr::Concurrency_name, send.cncr).c_str(),
            fmr::get_enum_string (fmr::Schedule_name, this->send.plan).c_str());
        }
        fmr::perf::timer_resume (& this->time);
      }
#if 0
      this->enums  [fmr::Data::Sims_type] = fmr::Enum_int_vals (//  Collection
        fmr::Data::Sims_type, sims_n, enum2val (fmr::Sims_type::TODO));// type
      //TODO XS Data sizing
      this->enums [fmr::Data::Sims_size] = fmr::Enum_int_vals (
        fmr::Data::Sims_size, sim_n, fmr::enum2val (this->sims_size));
      this->locals [fmr::Data::Part_halo_n] = fmr::Local_int_vals (
        fmr::Data::Part_halo_n, sim_n,0);
#endif
      this->dims [fmr::Data::Geom_d]
        = fmr::Dim_int_vals (fmr::Data::Geom_d, sim_n);
      this->enums [fmr::Data::Part_type] = fmr::Enum_int_vals
        (fmr::Data::Part_type, sim_n, fmr::enum2val (this->part_type));
      this->locals [fmr::Data::Gset_n]
        = fmr::Local_int_vals (fmr::Data::Gset_n, sim_n);
      this->locals [fmr::Data::Part_n]
        = fmr::Local_int_vals (fmr::Data::Part_n, sim_n);
      this->locals [fmr::Data::Mesh_n]
        = fmr::Local_int_vals (fmr::Data::Mesh_n, sim_n);
      this->locals [fmr::Data::Grid_n]
        = fmr::Local_int_vals (fmr::Data::Grid_n, sim_n);
      //
      this->dims [fmr::Data::Phys_d]
        = fmr::Dim_int_vals (fmr::Data::Phys_d, sim_n);
      this->enums [fmr::Data::Time_type] = fmr::Enum_int_vals
        (fmr::Data::Time_type, sim_n, fmr::enum2val (fmr::Sim_time::Implicit));
      this->locals [fmr::Data::Mtrl_n]
        = fmr::Local_int_vals (fmr::Data::Mtrl_n, sim_n);
      fmr::perf::timer_pause (& this->time);
    }
    return err;
  }
  int Sims::run () {int err=0;
    auto log = this->proc->log;
    if (log->detail >= this->verblevel) {log->print_heading ("Start");}
    this->prep ();
    fmr::perf::timer_resume (& this->time);
    const auto Psend = this->proc->hier [this->send.hier_lv];
    const auto sim_n = fmr::Local_int (this->model_list.size());
    const std::string name = this->model_name;
    if (sim_n < 1) {
      log->fprintf (log->fmrerr,"ERR""OR no sims found in collection %s\n",
        name.c_str());
      return 1;
    }
    this->data->get_dim_vals   (name, this->dims.at   (fmr::Data::Geom_d));
//    this->data->get_enum_vals  (name, this->enums.at  (fmr::Data::Part_type));
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Gset_n));
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Part_n));
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Mesh_n));
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Grid_n));
    //
    FMR_PRAGMA_OMP(omp parallel reduction(+:err)) {
      //TODO Handle other sim sizes.
      fmr::Local_int sim_i=0;
      const auto R = this->task.get<Sims>(Psend->get_proc_id());
      bool do_abort = false;
      while (!this->model_list.empty() && !do_abort) {// Run XS sims, FIFO.
        do_abort = false;
        FMR_PRAGMA_OMP(omp critical) {
          switch (this->send.plan) {
            case fmr::Schedule::Fifo :// first in/first out
              sim_i = sim_n - fmr::Local_int (this->model_list.size());
              R->model_name = this->model_list.front();
              this->model_list.pop_front();
              break;
            case fmr::Schedule::Filo :// first in/last out
              R->model_name = this->model_list.back();
              this->model_list.pop_back();
              sim_i = fmr::Local_int (this->model_list.size());
              break;
            default : do_abort = true;
              log->printf_err (
                "ERROR Sims distribution plan %s not yet implemented.\n",
                get_enum_string(fmr::Schedule_name, this->send.plan).c_str());
        } }//end critical region
        if (!do_abort) {
          R->sims_ix = sim_i;
          if (log->detail >= this->verblevel) {
            fmr::perf::timer_pause (& this->time);
            const auto geom_d = this->dims.at   (fmr::Data::Geom_d).data[sim_i];
            const auto gset_n = this->locals.at (fmr::Data::Gset_n).data[sim_i];
            const auto part_n = this->locals.at (fmr::Data::Part_n).data[sim_i];
            const auto mesh_n = this->locals.at (fmr::Data::Mesh_n).data[sim_i];
            const auto grid_n = this->locals.at (fmr::Data::Grid_n).data[sim_i];
            std::string label = std::to_string(geom_d)+ "D "
              + this->task_name+" "+R->task_name;
            log->label_fprintf (log->fmrout, label.c_str(),
              "%u:%s %u gset%s, %u part%s, %u grid%s, %u mesh%s\n",
              sim_i, R->model_name.c_str(),
              gset_n, (gset_n==1) ? "":"s",
              part_n, (part_n==1) ? "":"s",
              grid_n, (grid_n==1) ? "":"s",
              mesh_n, (mesh_n==1) ? "":"es");
            fmr::perf::timer_resume (& this->time);
          }
        //TODO Should this timing include lower-level runtime?
        err+= (R->run() > 0) ? 1 : 0;//TODO handle warnings (err<0)?
    } } }//end parallel region
    fmr::perf::timer_pause (& this->time, sim_n - err);
    if (log->detail >= this->verblevel) {log->print_heading ("Finished");}
#if 0
    return err;
#else
    return this->exit (err);//TODO barrier?
#endif
  }
  int Sims::exit_task (int err) {
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

