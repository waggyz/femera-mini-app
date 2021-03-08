#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Frun::Frun (Sims* F) noexcept {//TODO Change derived class name. Fsim? Sim1?
    this->parent = F; this->proc=F->proc; this->data=F->data;
    //
    this->       work_type = work_cast (Base_type::Frun);
//    this-> base_type = work_cast (Base_type::Sims);// TODO Remove?
    this->       task_name ="Run sims";
    this->      model_name ="(master runner)";
    this->       verblevel = 7;
    this->from = F->send;
    this->send = {1, fmr::Schedule::Once, fmr::Concurrency::Once,
      from.hier_lv
    };
    this->       part_algo = fmr::Partition::Merge;
    this->      meter_unit ="sim";
    this->         sims_ix = F->get_sims_n();
  }
  int Frun::chck (){
    return 0;
  }
  int Frun::prep (){int err=0;
    fmr::perf::timer_resume (& this->time);
    //TODO new Geom, get mesh data
    //
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
      txt = this->model_name+": "+fmr::Sim_time_name.at(sim_time);
      if (time_ok) {txt+= " analysis";}
      c=", ";
      if (mtrl_ok) {
        txt+= c+std::to_string(mtrl_n)+" material"+(mtrl_n==1?"":"s"); }
      txt += "\n";
      log->label_fprintf (this->proc->log->fmrout, label.c_str(), txt.c_str());
    }
#if 0
    Sims* run_sims = this;
    if (this->sims) {if (this->sims->task.first (this->work_type) == this) {
//      run_sims = new Frun (this->sims);
//      this->sims->task.add (run_sims);
//      int part_n = this->data->get_part_n (this->model_name);
//      for (int i=0; i<part_n; i++){this->task.add (new Part);
    }
    } }
    if (log->detail >= this->verblevel) {
      log->label_fprintf (log->fmrout,"Run sim name",
        "%s...\n", this->model_name.c_str() );
    }
#else
    //...
#endif
#if 0
    this->task.add (new Part(this, 0,
      this->data->make_data_id (this->model_name, fmr::Tree_type,{0})));
    // part[0] initializes all parts running under this Frun.
#else
    this->task.add (new Part(this));
#endif
    if (this->part_algo == fmr::Partition::Merge) {
      this->model_list.push_front("Merged part");
    }else{
      this->model_list.push_front("Part 1");
    }
    fmr::perf::timer_pause (& this->time);
    return err;
  }
  int Frun::run (){int err=0;
    fmr::perf::timer_resume (& this->time);
    err= this->prep ();
    //
    Proc* P = this->proc->hier[send.hier_lv];
    auto log = this->proc->log;
    if (log->detail >= this->verblevel) {if (P) {
      fmr::Local_int p = P->is_in_parallel () ? 1 : P->get_proc_n ();
      const int n = this->get_part_n ();
        fmr::Local_int c = 0;
        switch(this->send.cncr){
          case fmr::Concurrency::Once        :// Fall through.
          case fmr::Concurrency::Serial      : c = 1; break;
          case fmr::Concurrency::Independent :// Fall through.
          case fmr::Concurrency::Collective  : c = p; break;
          default: {}// Do nothing.
        }
      const std::string label = "Run "+std::to_string(n)+" part";
      log->label_fprintf (log->fmrout, label.c_str(),
        "%i %s %s / %i %s %s, %s\n",
        c, fmr::Partition_name.at(this->part_algo).c_str(),n==1?"part":"parts",
        p, P->task_name.c_str(),
        fmr::Concurrency_name.at(this->send.cncr).c_str(),
        fmr::Schedule_name.at(this->send.plan).c_str());
    } }
    Sims* run0 = this->task.first<Sims> (Base_type::Part);
    if (run0) {
      err= P->run (run0);//TODO fix return code handling
    }else{
      log->label_fprintf (log->fmrerr,"WARN""ING",
        "First Frun task in sims is null.\n");
    }
    //...
    fmr::perf::timer_resume (& this->time);
    return this->exit(err);
  }
#if 0
  int Frun::init_task (int*, char**){int err=0;//TODO call this?
    return err;
  }
#endif
  int Frun::exit_task (int err ){return err; }
}// end Femera namespace
#undef FMR_DEBUG
