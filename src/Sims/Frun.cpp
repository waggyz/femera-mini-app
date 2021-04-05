#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#include <unistd.h> //TODO Remove usleep.

namespace Femera {
  Frun::Frun (Sims* F) noexcept {//TODO Change derived class name. Fsim? Sim1?
    this->parent = F; this->proc=F->proc; this->data=F->data;
    this->from = F->send;
    this->send = {from.hier_lv, fmr::Schedule::Once, fmr::Concurrency::Once};
    this->sims_size = F->sims_size;
    //
    this->  work_type = work_cast (Base_type::Frun);
//    this-> base_type = work_cast (Base_type::Sims);// TODO Remove?
    this->  task_name ="Run!";
    this-> model_name ="(sim runner)";
    this->  verblevel = 3;
    this->  part_type = fmr::Partition::Join;
    this-> meter_unit ="dof";
    //this->data_list = {};
  }
  int Frun::chck (){
    return 0;
  }
  int Frun::prep () {int err=0;
    fmr::perf::timer_resume (& this->time);
    auto log = this->proc->log;
    //
    this->geom_d = parent->get_dim_val (fmr::Data::Geom_d, this->sims_ix);
    const std::string name = this->model_name;
    const auto time_type = fmr::Sim_time (
      parent->get_enum_val (fmr::Data::Time_type, this->sims_ix));
    if (log->detail >= this->verblevel) {
      fmr::perf::timer_pause (& this->time);
      const auto timestr = fmr::get_enum_string (fmr::Sim_time_name, time_type);
      const auto szshort = fmr::get_enum_string (fmr::Sim_size_short,
        this->sims_size);
      std::string label = szshort +" "+std::to_string(geom_d)+"D "
        + this->task_name +" prep";
      log->label_fprintf (log->fmrout, label.c_str(),
        "%s time, %s:%s_%u is %s\n", timestr.c_str(),
        parent->model_name.c_str(), this->task_name.c_str(), this->sims_ix,
        this->model_name.c_str());
      fmr::perf::timer_resume (& this->time);
    }
#if 0
    const auto gcad_n
      = parent->locals.at (fmr::Data::Gcad_n).data [this->sims_ix];
    const auto grid_n
      = parent->locals.at (fmr::Data::Grid_n).data [this->sims_ix];
    const auto mesh_n
      = parent->locals.at (fmr::Data::Mesh_n).data [this->sims_ix];
#else
    const auto gcad_n = parent->get_local_val (fmr::Data::Gcad_n,this->sims_ix);
    const auto grid_n = parent->get_local_val (fmr::Data::Grid_n,this->sims_ix);
    const auto mesh_n = parent->get_local_val (fmr::Data::Mesh_n,this->sims_ix);
#endif
    // add tasks: new Gcad/Grid/Mesh //TODO only first batch ?
    if (grid_n > 0) {
      //TODO cell_dims
      const auto send_type = work_cast (Plug_type::Grid);
      for (fmr::Local_int i=0; i < grid_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
    } }
    if (gcad_n > 0) {
      const auto send_type = work_cast (Plug_type::Gcad);
      for (fmr::Local_int i=0; i < gcad_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
    } }
    if (mesh_n > 0) {
      for (auto type : {fmr::Data::Elem_type}) {
        this->enums [type] = fmr::Enum_int_vals (type, mesh_n);
        this->data->get_enum_vals (name, this->enums.at (type));
      }
      for (auto type : {fmr::Data::Elem_n}) {
        this->locals [type] = fmr::Local_int_vals (type, mesh_n);
        this->data->get_local_vals (name, this->locals.at (type));
      }
#if 0
      if (submesh_n > 0) {
        for (auto type : {fmr::Data::Node_sysn, fmr::Data::Elem_sysn}) {
          this->globals [type] = fmr::Global_int_vals (type, mesh_n);
          this->data->get_global_vals (name, this->globals.at (type));
      } }
#endif
      const auto send_type = work_cast (Plug_type::Mesh);
      for (fmr::Local_int i=0; i < mesh_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
    } }
    const auto geom_n = this->task.count ();// grid_n + gcad_n + mesh_n
    if (geom_n > 0) {
      for (int i=0; i < geom_n; i++) {// Run serially.
        const auto G = this->task.get<Sims>(i);
        if (G) {
          G->sims_ix = i;
          G->model_name
            = this->model_name + ":" + G->task_name+"_"+std::to_string(i);
          err+= (G->prep () > 0) ? 1 : 0 ;
        }else{
          err++;
          log->printf_err ("ERROR %s %s mesh/grid/gcad %i is NULL.\n",
            this->task_name.c_str(), this->model_name.c_str(), i);
    } } }
#if 0
    this->data_id
      = this->data->make_data_id (this->model_name, fmr::Tree_type::Sims,{});
    const auto ginfo_id = this->data->make_data_id (this->data_id,
      fmr::Tree_type::Sims,{},fmr::Data::Geom_info);
    this->data->get_local_vals (ginfo_id, this->part_dims);
    const auto ginfo =& this->part_dims.data[0];
    const auto gisok =& this->part_dims.data[0];
    fmr::Local_int geom_d=0, gset_n=0, part_n=0, conn_n=0, mesh_n=0, grid_n=0;
    bool geod_ok=false, gset_ok=false, conn_ok=false, part_ok=false,
      mesh_ok=false, grid_ok=false;
    //
    auto ix = fmr::enum2val(fmr::Geom_info::Geom_d);
    if (ginfo[ix] && gisok[ix]) {
      geod_ok=true; geom_d = fmr::Dim_int   (ginfo[ix]); }
    ix = fmr::enum2val(fmr::Geom_info::Gset_n);
    if (ginfo[ix] && gisok[ix]) {
      gset_ok=true; gset_n = fmr::Local_int (ginfo[ix]); }
    ix = fmr::enum2val(fmr::Geom_info::Part_n);
    if (ginfo[ix] && gisok[ix]) {
      part_ok=true; part_n = fmr::Local_int (ginfo[ix]); }
    ix = fmr::enum2val(fmr::Geom_info::Part_halo_n);
    if (ginfo[ix] && gisok[ix]) {
      conn_ok=true; conn_n = fmr::Local_int (ginfo[ix]); }
    ix = fmr::enum2val(fmr::Geom_info::Mesh_n);
    if (ginfo[ix] && gisok[ix]) {
      mesh_ok=true; mesh_n = fmr::Local_int (ginfo[ix]); }
    ix = fmr::enum2val(fmr::Geom_info::Grid_n);
    if (ginfo[ix] && gisok[ix]) {
      grid_ok=true; grid_n = fmr::Local_int (ginfo[ix]); }
    //
    fmr::Local_int phys_d=0, mtrl_n=0;
    fmr::Sim_time sim_time = fmr::Sim_time::Unknown;
    const auto phys_id = this->data->make_data_id (this->data_id,
      fmr::Tree_type::Sims,{},fmr::Data::Phys_info);
    this-> data->get_local_vals (phys_id, this->phys_dims);
    const auto yinfo =& this->phys_dims.data[0];
    const auto yisok =& this->part_dims.data[0];
    bool phyd_ok=false, mtrl_ok=false, time_ok=false;
    ix = fmr::enum2val(fmr::Phys_info::Phys_d);
    if (yinfo[ix] && yisok[ix]) {
      phyd_ok=true; phys_d = fmr::Dim_int   (yinfo[ix]); }
    ix = fmr::enum2val(fmr::Phys_info::Mtrl_n);
    if (yinfo[ix] && yisok[ix]) {
      mtrl_ok=true; mtrl_n = fmr::Local_int (yinfo[ix]); }
    ix = fmr::enum2val(fmr::Phys_info::Sim_time);
    if (yinfo[ix] && yisok[ix]) {
      time_ok=true; sim_time = fmr::Sim_time (yinfo[ix]); }
    //
    auto log = this->proc->log;
    if (log->detail >= this->verblevel) {
      std::string label = geod_ok ? std::to_string(geom_d)+"D " : "";
      label += "geom";
      std::string txt = this->model_name, c=": ";
      if (gset_ok) {
        txt += c+std::to_string(gset_n)+" group"+(gset_n==1?"":"s"); c=", "; }
      if (part_ok) {
        txt += c+std::to_string(part_n)+" part"+(part_n==1?"":"s"); c=", "; }
      if (conn_ok) {
        txt += c+std::to_string(conn_n)+" conn"+(conn_n==1?"":"s"); c=", "; }
      if (mesh_ok) {
        txt += c+std::to_string(mesh_n)+" mesh"+(mesh_n==1?"":"es"); c=", "; }
      if (grid_ok) {
        txt += c+std::to_string(grid_n)+" grid"+(grid_n==1?"":"s"); c=", "; }
      txt += "\n";
      log->label_fprintf (this->proc->log->fmrout, label.c_str(), txt.c_str());
      //
      //TODO Get physics and mtrl props later?
      label = phyd_ok ? std::to_string(phys_d)+" dof " : "? dof ";
      label+="phys";
      txt = this->model_name+": "+fmr::get_enum_string (
        fmr::Sim_time_name, sim_time);
      if (time_ok) {txt+= " analysis";}
      c=", ";
      if (mtrl_ok) {
        txt+= c+std::to_string(mtrl_n)+" material"+(mtrl_n==1?"":"s"); }
      txt += "\n";
      log->label_fprintf (this->proc->log->fmrout, label.c_str(), txt.c_str());
    }
    this->task.add (new Part(this));
    if (this->part_type == fmr::Partition::Join) {
      this->model_list.push_front("(merged part)");
    }else{
      this->model_list.push_front("(part 1)");
    }
#endif
    fmr::perf::timer_pause (& this->time);
    return err;
  }
  int Frun::run () {int err=0;
    fmr::perf::timer_resume (& this->time);
    const auto log = this->proc->log;
    err= this->prep ();
//    if (err) {return err;}//TODO early return segfaults
#if 1
    double dof = 10e3;
    if (this->parent->globals.count (fmr::Data::Node_sysn) > 0) {
      if (this->sims_ix < this->parent->globals.at
        (fmr::Data::Node_sysn).data.size()) {
        dof = 3.0 * double(this->parent->globals.at
          (fmr::Data::Node_sysn).data [this->sims_ix]);
    } }
    const double iters = 0.01 * dof;
    const double speed = 1e9 / 40.0;// dof/s Skylake XS sim solve speed
    const double  secs = iters * dof / speed;
#if 1
    if (log->detail >= this->verblevel) {
      const std::string label = this->task_name +" ZZZZ";
      log->label_fprintf(log->fmrout, label.c_str(), "%i: sleep %.1g s...\n",
        this->sims_ix, secs);
    }
#endif
    usleep (int (1e6 * secs));
#endif
#if 0
    Proc* P = this->proc->hier[send.hier_lv];
    auto log = this->proc->log;
    if (log->detail >= this->verblevel) {if (P) {
      fmr::Local_int p = P->is_in_parallel () ? 1 : P->get_proc_n ();
      const int n = this->get_part_n ();
        fmr::Local_int c = 0;
        switch (this->send.cncr) {
          case fmr::Concurrency::Once        :// Fall through.
          case fmr::Concurrency::Serial      : c = 1; break;
          case fmr::Concurrency::Independent :// Fall through.
          case fmr::Concurrency::Collective  : c = p; break;
          default: {}// Do nothing.
        }
      const std::string label = "Run "+std::to_string(n)+" part";
      log->label_fprintf (log->fmrout, label.c_str(),
        "%i %s %s / %i %s %s, %s\n",
        c, fmr::get_enum_string (fmr::Partition_name, this->part_type).c_str(),
        n==1?"part":"parts", p, P->task_name.c_str(),
        fmr::get_enum_string (fmr::Concurrency_name, this->send.cncr).c_str(),
        fmr::get_enum_string (fmr::Schedule_name, this->send.plan).c_str());
    } }
    Sims* run0 = this->task.first<Sims> (Base_type::Part);
    if (run0) {
      err= P->run (run0);//TODO fix return code handling
    }else{
      log->label_fprintf (log->fmrerr,"WARN""ING",
        "First Part task in sims is null.\n");
    }
#endif
    //...
#if 0
    if (geom_n > 0) {
      for (int i=0; i < geom_n; i++) {//TODO need barrier before this?
        const auto G = this->task.get<Sims>(i);
        if (G) {err= G->exit (err);}
    } }
#else
    fmr::perf::timer_resume (& this->time);
    return this->exit(err);//TODO need barrier before this?
#endif
  }
#if 0
  int Frun::init_task (int*, char**){int err=0;//TODO call this?
    return err;
  }
#endif
  int Frun::exit_task (int err ){return err; }
}// end Femera namespace
#undef FMR_DEBUG
