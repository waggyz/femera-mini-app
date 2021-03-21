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
    this->  sims_size = F->sims_size;
    //
    this->  work_type = work_cast (Base_type::Frun);
//    this-> base_type = work_cast (Base_type::Sims);// TODO Remove?
    this->  task_name ="Frun";
    this-> model_name ="(sim runner)";
    this->  verblevel = 7;
    this->  part_type = fmr::Partition::Merge;
    this-> meter_unit ="sim";
  }
  int Frun::chck (){
    return 0;
  }
  int Frun::prep () {int err=0;
    fmr::perf::timer_resume (& this->time);
    auto log = this->proc->log;
    //
    const std::string name = this->model_name;
    auto mesh_n = parent->locals.at (fmr::Data::Mesh_n).data [this->sims_ix];
    //
#if 1
    this->enums [fmr::Data::Elem_type]
      = fmr::Enum_int_vals (fmr::Data::Elem_type, mesh_n,0);
    this->data->get_enum_vals (name, this->enums.at (fmr::Data::Elem_type));
    //
    this->locals [fmr::Data::Node_n]
      = fmr::Local_int_vals (fmr::Data::Node_n, mesh_n,0);
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Node_n));
    //
    this->locals [fmr::Data::Elem_n]
      = fmr::Local_int_vals (fmr::Data::Elem_n, mesh_n,0);
    this->data->get_local_vals (name, this->locals.at (fmr::Data::Elem_n));
    //
#endif
    const auto geom_d
      = parent->dims.at (fmr::Data::Geom_d).data [this->sims_ix];
#if 1
    if (log->detail >= this->verblevel) {
      fmr::perf::timer_pause (& this->time);
      std::string label = std::to_string(geom_d)+"D "+this->task_name+" prep";
      log->label_fprintf (log->fmrout, label.c_str(),
        "%s:%s_%u %s\n",
        parent->model_name.c_str(), this->task_name.c_str(), this->sims_ix,
        this->model_name.c_str());
      fmr::perf::timer_resume (& this->time);
    }
#endif
    //TODO new Geom, get mesh data
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
    if (this->part_type == fmr::Partition::Merge) {
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
    err= this->prep ();
#if 1
    // quick estimate of solve time
    double dof   = 10e3;//TODO Get this from current sim
    double iters = 0.01 * dof;
    double speed =  1e9 / 40.0;// dof/s Skylake XS sim solve speed
    usleep (int(1e6 * iters * dof / speed));
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
