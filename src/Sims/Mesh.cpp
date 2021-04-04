#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Mesh::Mesh (Sims* F) noexcept {
    this->parent = F; this->proc = F->proc; this->data = F->data;
    this->from = F->send;
    this->send = {from.hier_lv, fmr::Schedule::Block, fmr::Concurrency::Serial};
    this->  sims_size = F->sims_size;
    this->  work_type = work_cast (Plug_type::Mesh);
//    this->base_type = work_cast (Base_type::Geom);// TODO Remove?
    this->  task_name ="Mesh";
    this-> meter_unit ="elem";
    this-> model_name ="(unknown mesh)";
    this->  verblevel = 8;
    this->  part_type = fmr::Partition::None;
  }
  int Mesh::chck (){
    return 0;
  }
  int Mesh::prep () {int err=0;
    fmr::perf::timer_resume (& this->time);
    auto log = this->proc->log;
    const std::string name = this->model_name;
    const auto pid = this->parent->model_name;//TODO XS sims only
    const auto tid = this->data->make_data_id (pid, fmr::Data::Elem_form);
    bool is_found = this->data->enum_vals.count(tid) > 0;
    const auto form = is_found
      ? fmr::Elem_form(this->data->enum_vals.at(tid).data[this->sims_ix])
      : fmr::Elem_form::Unknown;
    // Initially, prep only meshes matching the enclosing space dimension.
    this->mesh_d = fmr::elem_form_d[fmr::enum2val(form)];
    this->geom_d = this->parent->geom_d;
#ifdef FMR_DEBUG
      log->label_fprintf(log->fmrout, "**** Frun Geom",
        "geom_d %u, geom_d %u\n", geom_d, mesh_d);
#endif
    const auto eid = this->data->make_data_id (pid, fmr::Data::Elem_n);
    is_found = this->data->local_vals.count(eid) > 0;
    this->elem_n = is_found
      ? this->data->local_vals.at(eid).data[this->sims_ix] : 0;
    if (this->mesh_d == this->geom_d && this->mesh_d > 0) {
      if (log->detail >= this->verblevel) {
        fmr::perf::timer_pause (& this->time);
#if 1
        const auto nid = this->data->make_data_id (pid, fmr::Data::Node_n);
        is_found = this->data->local_vals.count(nid) > 0;
        const auto nds = is_found
          ? this->data->local_vals.at(nid).data[this->sims_ix] : 0;
#endif
        //
        const std::string label = std::to_string(this->mesh_d)+"D "
          + this->task_name+" prep";
        log->label_fprintf (log->fmrout, label.c_str(), "%s: %u %s, %u node%s\n",
          name.c_str(), this->elem_n,
          fmr::get_enum_string(fmr::elem_form_name,form).c_str(),
          nds, (nds==1)?"":"s");
        fmr::perf::timer_resume (& this->time);
      }
    //
    }//end if this needs prepped
    fmr::perf::timer_pause (& this->time, this->elem_n);
    return err;
  }
  int Mesh::run (){int err=0;
    return err;
  }
#if 0
  int Mesh::prep (){
    return 0;
  }
  int Mesh::init_task (int*, char**){int err=0;
    return err;
  }
  int Mesh::exit_task (int err){ return err; }
#endif
}// end Femera namespace
#undef FMR_DEBUG
