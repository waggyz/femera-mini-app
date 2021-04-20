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
    const auto log = this->proc->log;
    const auto name = this->model_name;
    const auto pid = this->parent->model_name;//TODO XS sims only
    const auto tid = this->data->make_data_id (pid, fmr::Data::Elem_form);
    bool is_found = this->data->enum_vals.count(tid) > 0;
    const auto form = is_found
      ? fmr::Elem_form(this->data->enum_vals.at(tid).data[this->sims_ix])
      : fmr::Elem_form::Unknown;
    this->elem_d = fmr::elem_form_d[fmr::enum2val(form)];
    this->geom_d = this->parent->geom_d;
#ifdef FMR_DEBUG
      log->label_fprintf(log->fmrout, "**** Mesh prep",
        "%s: %uD %s mesh in %uD space\n",
        this->model_name.c_str(), elem_d,
        fmr::get_enum_string(fmr::elem_form_name,form).c_str(),geom_d);
#endif
    const auto eid = this->data->make_data_id (pid, fmr::Data::Elem_n);
    is_found = this->data->local_vals.count(eid) > 0;
    this->elem_n
      = is_found ? this->data->local_vals.at(eid).data[this->sims_ix] : 0;
    if ((this->elem_d == this->geom_d) && this->elem_d > 0) {
    //TODO for now, prep only meshes matching the enclosing space dimension.
    //TODO should prep all meshes with physics assigned.
      fmr::perf::timer_pause (& this->time, this->elem_n);//count elems prepped
      if (log->timing >= this->verblevel) {
        this->meter_unit = fmr::get_enum_string (fmr::elem_form_name, form);
      }
      if (log->detail >= this->verblevel) {
        const auto label = std::to_string(this->elem_d)+"D "
          + this->task_name+" prep";
        log->label_fprintf (log->fmrout, label.c_str(),"%u %s in %s\n",
          this->elem_n, this->meter_unit.c_str(), name.c_str());
      }
      fmr::perf::timer_resume (& this->time);
      this->ini_data_vals (this->data_list, 0);
      if (this->elem_n > 0) {//TODO check element type for nodes/elem
#if 0
        //TODO The local mesh node count is not available in a Gmsh single file
        //     nor from the CGNS mid-level library. The local elem connectivity
        //     needs to be scanned to find all of the unique local nodes.
        //     Or, an array of all nodes can be shared among meshes in
        //     XS & SM simms, because each sim is contained in 1 NUMA domain.
        //     So, load node_coor at global sim level, if needed for jacs calc.
#endif
        const auto each_conn_n = 0;//TODO each_conn_n(form)/elem_vert_n(form)?
        const auto each_jacs_n = 0;//TODO each_jacs_n(form)
        // element jacobian data (inverted, maybe transposed, may have det)
        this->ini_data_vals ({fmr::Data::Elem_conn}, elem_n * each_conn_n);
        this->ini_data_vals ({fmr::Data::Jacs_dets}, elem_n * each_jacs_n);
        this->data_list.push_back (fmr::Data::Elem_conn);
        this->data_list.push_back (fmr::Data::Jacs_dets);
      }
#ifdef FMR_DEBUG
      auto vals = this->locals [fmr::Data::Elem_conn];
      const std::string tstr = fmr::get_enum_string (fmr::vals_name, vals.type);
      log->label_fprintf (log->fmrerr, "****  get Mesh",
        "%lu %s:%s vals...\n",
        vals.data.size(),
        name.c_str(), tstr.c_str());
#endif
      this->get_data_vals (this->model_name, this->data_list);
    }//end if this needs prepped
    fmr::perf::timer_pause (& this->time);
    return err;
  }
  int Mesh::ini_data_vals (const Data_list list, const size_t n) {int err=0;
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Geom :
          this->geoms [type] = fmr::Geom_float_vals (type, n); break;
        default : {err= 1;
//          const auto log = this->proc->log;
//          const std::string namestr = fmr::get_enum_string (fmr::vals_name,type);
//          log->label_fprintf (log->fmrerr, "WARN""ING Sims",
//            "ini_data_vals (..) type of %s not handled.\n", namestr.c_str());
    } } }
    if (err>0) {return Sims::ini_data_vals (list, n);}
    return err;
  }
  int Mesh::get_data_vals (const fmr::Data_id name, const Data_list list) {
    int err=0;
#ifdef FMR_DEBUG
    const auto log = this->proc->log;
    auto vals = this->locals [fmr::Data::Elem_conn];
    const std::string tstr = fmr::get_enum_string (fmr::vals_name, vals.type);
    log->label_fprintf (log->fmrerr, "****    1 Sims",
      "%lu %s:%s local vals...\n",vals.data.size(),name.c_str(), tstr.c_str());
#endif
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Geom :
          err= this->data->get_geom_vals (name, this->geoms.at (type)); break;
        default : {err= 1;
//          const std::string namestr = fmr::get_enum_string (fmr::vals_name,type);
//          log->label_fprintf (log->fmrerr, "WARN""ING Sims",
//            "get_data_vals (..) Vals_type of %s not handled.\n",namestr.c_str());
    } } }
#ifdef FMR_DEBUG
    auto val2 = this->locals [fmr::Data::Elem_conn];
    const std::string vstr = fmr::get_enum_string (fmr::vals_name, val2.type);
    log->label_fprintf (log->fmrerr, "****    2 Sims",
      "%lu %s:%s local vals...\n",val2.data.size(),name.c_str(), vstr.c_str());
#endif
    if (err>0) {return Sims::get_data_vals (name, list);}
    return err;
  }
  int Mesh::run (){int err=0;
    const auto log = this->proc->log;
    if (log->detail >= this->verblevel) {
      const auto label = "run "+this->task_name;
      log->label_fprintf(log->fmrout, label.c_str(),
        "%s: %uD mesh in the %uD space of %s\n",
        this->model_name.c_str(), this->elem_d, this->geom_d,
        this->parent->model_name.c_str());
    }
    //fmr::perf::timer_resume (& this->time);
    //
    // this->data->make_mesh (this->parent->model_name);//TODO XS sims only
    //
    //fmr::perf::timer_pause (& this->time);
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
