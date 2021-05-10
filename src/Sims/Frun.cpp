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
    this->  task_name ="Run1";
    this-> model_name ="(sim runner)";
    this->  verblevel = 3;
    this->  part_type = fmr::Partition::Join;
    this-> meter_unit ="dof";
    //this->data_list = {};
  }
  int Frun::chck () {
    const auto start = fmr::perf::get_now_ns();
    const auto log = this->proc->log;
    //
    //
    //
    if (this->verblevel <= log->timing) {
      log->proc_printf ("\"%s\",\"%s\",%lu,%lu\n",
        this->model_name.c_str(), "chck", start, fmr::perf::get_now_ns());
    }
    return 0;
  }
  int Frun::prep () {int err=0;
    const auto start = fmr::perf::get_now_ns();
    fmr::perf::timer_resume (& this->time);
    const auto log = this->proc->log;
    const std::string name = this->model_name;
    //
    this->geom_d = parent->get_dim_val (fmr::Data::Geom_d, this->sims_ix);
    const auto time_type = fmr::Sim_time (
      parent->get_enum_val (fmr::Data::Time_type, this->sims_ix));
    const auto grid_n = parent->get_local_val (fmr::Data::Grid_n,this->sims_ix);
    const auto mesh_n = parent->get_local_val (fmr::Data::Mesh_n,this->sims_ix);
    if (log->detail >= this->verblevel) {
      fmr::perf::timer_pause (& this->time);
      const auto timestr = fmr::get_enum_string (fmr::Sim_time_name, time_type);
      const auto szshort
        = fmr::get_enum_string (fmr::Sim_size_short, this->sims_size);
      const auto label = szshort +std::to_string(geom_d)+"D "
        + this->task_name +" prep";
      log->label_fprintf (log->fmrout, label.c_str(),
        "%s time, %s:%s_%u is %s\n", timestr.c_str(),
        parent->model_name.c_str(), this->task_name.c_str(), this->sims_ix,
        this->model_name.c_str());
      fmr::perf::timer_resume (& this->time);
    }
    if (grid_n > 0) {
      //TODO cell_dims
      const auto send_type = work_cast (Plug_type::Grid);
      for (fmr::Local_int i=0; i < grid_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
    } }
#if 0
    const auto gcad_n = parent->get_local_val (fmr::Data::Gcad_n,this->sims_ix);
    if (gcad_n > 0) {
      const auto send_type = work_cast (Plug_type::Gcad);
      for (fmr::Local_int i=0; i < gcad_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
    } }
#endif
    if (mesh_n > 0) {
      const auto send_type = work_cast (Plug_type::Mesh);
      for (fmr::Local_int i=0; i < mesh_n; i++) {
        err= fmr::detail::main->add_new_task (send_type, this);
#if 0
      for (auto type : {fmr::Data::Elem_n}) {//TODO needed here?
        this->locals [type] = fmr::Local_int_vals (type, mesh_n);
        this->data->get_local_vals (name, this->locals.at (type));
      }
#endif
    } }
    const auto geom_n = this->task.count ();// grid_n + mesh_n
    if (geom_n > 0) {for (int i=0; i < geom_n; i++) {// Run serially.
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
    //TODO Post, Solv, & Phys prep
    fmr::perf::timer_pause (& this->time);
    if (this->verblevel <= log->timing) {
      log->proc_printf ("\"%s\",\"%s\",%lu,%lu\n",
        this->model_name.c_str(), "prep", start, fmr::perf::get_now_ns());
    }
    return err;
  }
  int Frun::run () {int err=0;
    fmr::perf::timer_resume (& this->time);
    err= this->prep ();
#if 0
    if (err) {return err;}//TODO early return segfaults
#endif
    err= this->iter ();
    err= this->post ();
#if 0
    if (geom_n > 0) {
      for (int i=0; i < geom_n; i++) {//TODO need barrier before this?
        const auto G = this->task.get<Sims>(i);
        if (G) {err= G->exit (err);}
    } }
#endif
    fmr::perf::timer_pause (& this->time);
    return this->exit (err);//TODO need barrier before this?
  }
  int Frun::iter () {int err=0;// TODO this->solv->iter ?
    const auto start = fmr::perf::get_now_ns();
    const auto log = this->proc->log;
    //
    const int solv = 0;
    //
    switch (solv) {
      case 0 : {// Solv::None
        err= 1;
        fmr::Global_int node_n = 0;
        if (this->parent->globals.count (fmr::Data::Node_sysn) > 0) {
          if (this->sims_ix < this->parent->globals.at
            (fmr::Data::Node_sysn).data.size()) {
            node_n = this->parent->globals.at
              (fmr::Data::Node_sysn).data [this->sims_ix];
            err= 0;
        } }
        const auto elem_n = this->parent->globals.at
          (fmr::Data::Elem_sysn).data [this->sims_ix];
        if (this->verblevel <= log->timing) {
          log->proc_printf ("\"%s\",\"%s\",%lu,%lu,%lu,%lu,%lu\n",
            this->model_name.c_str(), "iter", start, fmr::perf::get_now_ns(),
            elem_n, node_n, 3*node_n);
        }
        break;}
      case 1 : {// Solv::Zzzz
        double dof = 10e3; err= 1;
        fmr::Global_int node_n = 0;
        if (this->parent->globals.count (fmr::Data::Node_sysn) > 0) {
          if (this->sims_ix < this->parent->globals.at
            (fmr::Data::Node_sysn).data.size()) {
            node_n = this->parent->globals.at
              (fmr::Data::Node_sysn).data [this->sims_ix];
            dof = 3.0 * double (node_n);
            err= 0;
        } }
        double speed = 1e9 // dof/s Skylake XS P2 tet solve speed on 40 cores
          - (dof <  10e3) ? 0.0 : 90e6 * (std::log (dof) - std::log (10e3));
        speed = (speed < 500e6) ? 500e6 : speed;// dof/s XL solve speed
        speed/= 40.0;// single-core speed
        const double iters = (dof < 500.0 ? 0.1 : 0.01) * dof;
        const double  secs = iters * dof / speed;
        const int ret = (secs < 1.0)
          ? usleep (int (1e6 * secs)) : sleep (int (secs));
        const auto elem_n = this->parent->globals.at
          (fmr::Data::Elem_sysn).data [this->sims_ix];
        if (this->verblevel <= log->timing) {
          log->proc_printf ("\"%s\",\"%s\",%lu,%lu,%lu,%lu,%lu\n",
            this->model_name.c_str(), "zzzz", start, fmr::perf::get_now_ns(),
            elem_n, node_n, 3*node_n);
        }
        if (ret==0) {if (log->detail >= this->verblevel) {
          const auto secstr = fmr::perf::format_time_units (secs);
          const auto szshort = fmr::get_enum_string (fmr::Sim_size_short,
            this->sims_size);
          const auto  label = szshort +std::to_string (geom_d)+"D "
            + this->task_name +" zzzz";
          log->label_fprintf (log->fmrout, label.c_str(),
            "sim_%u: %u geom, %lu elem, %lu node, %g DOF, sleep %s...\n",
            this->sims_ix, this->task.count(), elem_n, node_n, dof,
            secstr.c_str());
        } }
        break;}
      case 2 : break;// Solv::Flop
      default : err=1;
    }// end switch (solv)
    return err;
  }
  int Frun::post () {int err=0;
    const auto start = fmr::perf::get_now_ns();
    auto log = this->proc->log;
    //
    //
    //
    if (this->verblevel <= log->timing) {
      log->proc_printf ("\"%s\",\"%s\",%lu,%lu\n",
        this->model_name.c_str(), "post", start, fmr::perf::get_now_ns());
    }
    return err;
  }
#if 0
  int Frun::init_task (int*, char**){int err=0;//TODO call this?
    return err;
  }
#endif
  int Frun::exit_task (int err) {return err;}
}// end Femera namespace
#undef FMR_DEBUG
