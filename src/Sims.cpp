#include "base.h"

#include <unistd.h>   // getopt, optarg

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
    this->data_list = {
      fmr::Data::Geom_d,
      fmr::Data::Gset_n,    fmr::Data::Part_n,    fmr::Data::Mesh_n,
      fmr::Data::Grid_n,    fmr::Data::Gcad_n,
      fmr::Data::Node_sysn, fmr::Data::Elem_sysn//,
//      fmr::Data::Phys_d,    fmr::Data::Mtrl_n,
//      fmr::Data::Dofs_sysn
    };
  }
  fmr::Local_int Sims::get_sims_n (){// number of collections loaded
    return this->task.count (Base_type::Sims);
  }
  fmr::Local_int Sims::get_frun_n (){// number of sims loaded
    return this->task.count (Base_type::Frun);
  }
  fmr::Local_int Sims::get_part_n (){//TODO Remove.
    return this->task.count (Base_type::Part);
  }
  int Sims::chck () {
    return 0;
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
      this->ini_data_vals (this->data_list, sim_n);
      this->enums [fmr::Data::Part_type] = fmr::Enum_int_vals
        (fmr::Data::Part_type, sim_n, fmr::enum2val (this->part_type));
      //this->data_list.push_back(fmr::Data::Part_type);
      this->enums [fmr::Data::Time_type] = fmr::Enum_int_vals
        (fmr::Data::Time_type, sim_n, fmr::enum2val (fmr::Sim_time::Implicit));
      //this->data_list.push_back(fmr::Data::Time_type);
#if 0
      this->enums  [fmr::Data::Sims_type] = fmr::Enum_int_vals (// Collection
        fmr::Data::Sims_type, sims_n, enum2val (fmr::Sims_type::TODO));// type
      this->enums [fmr::Data::Sims_size] = fmr::Enum_int_vals (//      XS sims
        fmr::Data::Sims_size, sim_n, fmr::enum2val (this->sims_size));
      this->locals [fmr::Data::Part_halo_n] = fmr::Local_int_vals (
        fmr::Data::Part_halo_n, sim_n,0);
#endif
      //
      fmr::perf::timer_pause (& this->time);
    }
    return err;
  }
  int Sims::iter () {int err=0;
    return err;
  }
  int Sims::post () {int err=0;
    return err;
  }
  int Sims::ini_data_vals (const Data_list list, const size_t n) {int err=0;
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Dim :
          this->dims [type] = fmr::Dim_int_vals (type, n); break;
        case fmr::Vals_type::Enum :
          this->enums [type] = fmr::Enum_int_vals (type, n); break;
        case fmr::Vals_type::Local :
          this->locals [type] = fmr::Local_int_vals (type, n); break;
        case fmr::Vals_type::Global :
          this->globals [type] = fmr::Global_int_vals (type, n); break;
        default : {err= 1;
          const auto log = this->proc->log;
          const std::string namestr = fmr::get_enum_string (fmr::vals_name,type);
          log->label_fprintf (log->fmrerr, "WARN""ING Sims",
            "ini_data_vals (..) type of %s not handled.\n", namestr.c_str());
    } } }
    return err;
  }
  int Sims::get_data_vals (const fmr::Data_id name, const Data_list list) {
    int err=0;
    const auto log = this->proc->log;
#ifdef FMR_DEBUG
    auto vals = this->locals [fmr::Data::Elem_conn];
    const std::string tstr = fmr::get_enum_string (fmr::vals_name, vals.type);
    log->label_fprintf (log->fmrerr, "****    1 Sims",
      "%lu %s:%s local vals...\n",vals.data.size(),name.c_str(), tstr.c_str());
#endif
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Dim :
          err= this->data->get_dim_vals (name, this->dims.at (type)); break;
        case fmr::Vals_type::Enum :
          err= this->data->get_enum_vals (name, this->enums.at (type)); break;
        case fmr::Vals_type::Local :
          err= this->data->get_local_vals (name, this->locals.at (type)); break;
        case fmr::Vals_type::Global :
          err= this->data->get_global_vals (name, this->globals.at(type)); break;
        default : {err= 1;
          const std::string namestr = fmr::get_enum_string (fmr::vals_name,type);
          log->label_fprintf (log->fmrerr, "WARN""ING Sims",
            "get_data_vals (..) Vals_type of %s not handled.\n",namestr.c_str());
    } } }
#ifdef FMR_DEBUG
    auto val2 = this->locals [fmr::Data::Elem_conn];
    const std::string vstr = fmr::get_enum_string (fmr::vals_name, val2.type);
    log->label_fprintf (log->fmrerr, "****    2 Sims",
      "%lu %s:%s local vals...\n",val2.data.size(),name.c_str(), vstr.c_str());
#endif
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
      log->fprintf (log->fmrerr,
        "ERR""OR Sims::run() no sims found in collection %s\n", name.c_str());
      return 1;
    }
    // *** HERE: get gcad_n for each sim in this batch and mesh them?
    this->get_data_vals (name, this->data_list);
    //
    if (this->send.hier_lv > this->from.hier_lv
      && Psend->task_name =="OpenMP") {
      FMR_PRAGMA_OMP(omp parallel reduction(+:err)) {
        //TODO Handle other sim sizes.
        fmr::Local_int sim_i=0;
        const auto R = this->task.get<Sims>(Psend->get_proc_id());
        bool do_abort = false;
        while (!this->model_list.empty() && !do_abort) {// Run XS sims
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
              default : do_abort = true;//TODO Set err ?
                log->printf_err (
                  "ERR""OR Sims distribution plan %s not yet implemented.\n",
                  get_enum_string(fmr::Schedule_name, this->send.plan).c_str());
          } }//end critical region
          if (!do_abort) {
            R->sims_ix = sim_i;
            const auto  geo_d = this->get_dim_val (fmr::Data::Geom_d, sim_i);
            const auto gcad_n = this->get_local_val (fmr::Data::Gcad_n,sim_i);
            if (gcad_n > 0) {//TODO move to *** HERE?
              for (fmr::Local_int i=0; i<gcad_n; i++) {
                err+= this->data->make_mesh (R->model_name, i);
                //TODO add R->model_name+"-"+std::to_str(i) to model_list ?
              }
              this->get_data_vals (name, this->data_list);//TODO only new vals
            }
            if (log->detail >= this->verblevel) {
              fmr::perf::timer_pause (& this->time);
              const auto gset_n = this->get_local_val(fmr::Data::Gset_n, sim_i);
              const auto part_n = this->get_local_val(fmr::Data::Part_n, sim_i);
              const auto grid_n = this->get_local_val(fmr::Data::Grid_n, sim_i);
              const auto mesh_n = this->get_local_val(fmr::Data::Mesh_n, sim_i);
              const auto node_n = get_global_val (fmr::Data::Node_sysn, sim_i);
              const auto elem_n = get_global_val (fmr::Data::Elem_sysn, sim_i);
              if (false) {
                for (fmr::Local_int i=0; i<gcad_n; i++) {
                  err+= this->data->make_part (R->model_name, i, part_n);//TODO
              } }
#if 0
              const auto dofs_n = get_global_val (fmr::Data::Dofs_sysn, sim_i);
#else
              const fmr::Global_int dofs_n = 0;//TODO
#endif
              const auto szshort = fmr::get_enum_string (fmr::Sim_size_short,
                this->sims_size);
              const std::string label = szshort+std::to_string(geo_d)+ "D "
                + this->task_name+" " + R->task_name;
              log->label_fprintf (log->fmrout, label.c_str(),
                "sim_%u: %u gset%s, %u part%s, %u CAD%s, %u grid%s, %u mesh%s\n",
                sim_i,
                gset_n, (gset_n==1)?"":"s", part_n, (part_n==1)?"":"s",
                gcad_n, (gcad_n==1)?"":"s", grid_n, (grid_n==1)?"":"s",
                mesh_n, (mesh_n==1)?"":"es");
              log->label_fprintf (log->fmrout, label.c_str(),
                "sim_%u: %lu elem%s, %lu node%s, %lu DOF%s total in %s\n",
                sim_i,
                elem_n, (elem_n==1)?"":"s", node_n, (node_n==1)?"":"s",
                dofs_n, (dofs_n==1)?"":"s", R->model_name.c_str());
              fmr::perf::timer_resume (& this->time);
            }
            //TODO Should this timing include lower-level runtime?
            err+= (R->run() > 0) ? 1 : 0;//TODO handle warnings (err<0)?
      } } }//end parallel region
    }else{
      const auto Pfrom = this->proc->hier [this->from.hier_lv];
      log->printf_err (
        "ERR""OR %s distribute from level %u:%s to %u:%s not yet implemented.\n",
        this->task_name.c_str(),
        this->from.hier_lv, Pfrom->task_name.c_str(),
        this->send.hier_lv, Psend->task_name.c_str());
      return 1;
    }
    fmr::perf::timer_pause (& this->time, sim_n - err);
    if (log->detail >= this->verblevel) {log->print_heading ("Finished");}
    if (err > 0) {
      const std::string label = "WARN""ING "+this->task_name;;
      log->label_fprintf (log->fmrerr, label.c_str(), "%i/%u ran with %s.\n",
        err, sim_n, (err==1)?"an err""or":"err""ors");
    }
    return this->exit (err);//TODO barrier?
  }
  fmr::Dim_int Sims::get_dim_val (fmr::Data dt, size_t ix) {
    if (this->dims.count(dt) > 0) {
      if (this->dims.at(dt).data.size() > ix) {
        return this->dims.at(dt).data[ix];
    } }
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,dt);
    const std::string label = "WARN""ING "+this->task_name;;
    this->proc->log->label_fprintf (this->proc->log->fmrerr, label.c_str(),
      "%s[%lu] dim val not found.\n", namestr.c_str(), ix);
    return 0;
  }
  fmr::Enum_int Sims::get_enum_val (fmr::Data dt, size_t ix) {
    if (this->enums.count(dt) > 0) {
      if (this->enums.at(dt).data.size() > ix) {
        return this->enums.at(dt).data[ix];
    } }
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,dt);
    const std::string label = "WARN""ING "+this->task_name;;
    this->proc->log->label_fprintf (this->proc->log->fmrerr, label.c_str(),
      "%s[%lu] enum val not found.\n", namestr.c_str(), ix);
    return 0;
  }
  fmr::Local_int Sims::get_local_val (fmr::Data dt, size_t ix) {
    if (this->locals.count(dt) > 0) {
      if (this->locals.at(dt).data.size() > ix) {
        return this->locals.at(dt).data[ix];
    } }
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,dt);
    const std::string label = "WARN""ING "+this->task_name;;
    this->proc->log->label_fprintf (this->proc->log->fmrerr, label.c_str(),
      "%s[%lu] local val not found.\n", namestr.c_str(), ix);
    return 0;
  }
  fmr::Global_int Sims::get_global_val (fmr::Data dt, size_t ix) {
    if (this->globals.count(dt) > 0) {
      if (this->globals.at(dt).data.size() > ix) {
        return this->globals.at(dt).data[ix];
    } }
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,dt);
    const std::string label = "WARN""ING "+this->task_name;;
    this->proc->log->label_fprintf (this->proc->log->fmrerr, label.c_str(),
      "%s[%lu] global val not found.\n", namestr.c_str(), ix);
    return 0;
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
  int Sims::init_task (int* argc, char** argv) {int err=0;
    auto log = this->proc->log;
#ifdef FMR_DEBUG
    if (log->verbosity >= this->verblevel) {
      const auto label = this->task_name+" init_task";
      log->label_fprintf (log->fmrout, label.c_str(), "***\n");
    }
#endif
    fmr::perf::timer_resume (& this->time);
    FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
      int argc2=argc[0];// Copy getopt variables.
      auto opterr2=opterr; auto optopt2=optopt;
      auto optind2=optind; auto optarg2=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, ":S::M::L::X::")) != -1){
        // x:  requires an argument
        // x:: optional argument (Gnu compiler)
        switch (optchar){
          case 'S' : {this->proc->opt_add (optchar);
            this->sims_size = fmr::Sim_size::SM;
            break;}
          case 'M' : {this->proc->opt_add (optchar);
            this->sims_size = fmr::Sim_size::MD;
            break;}
          case 'L' : {this->proc->opt_add (optchar);
            this->sims_size = fmr::Sim_size::LG;
            break;}
          case 'X' : {this->proc->opt_add (optchar);
            if (optarg) {switch (optarg[0]) {
              case 'S' : {this->sims_size = fmr::Sim_size::XS; break;}
              case 'L' : {this->sims_size = fmr::Sim_size::XL; break;}
              default : {}// Do nothing.
            } }
            break;}
          default :{}// Do nothing.
      } }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
    }//end non-threadsafe section
    if (log->verbosity >= this->verblevel) {
      const auto szname = fmr::get_enum_string (fmr::Sim_size_name,
        this->sims_size);
      const auto szshort = fmr::get_enum_string (fmr::Sim_size_short,
        this->sims_size);
      const auto label = szshort+this->task_name +" init";
      log->label_printf (label.c_str(),"%s assumed sim size\n", szname.c_str());
    }
    fmr::perf::timer_pause (&this->time);
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

