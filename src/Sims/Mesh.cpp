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
    this->geom_d = this->parent->geom_d;
    const auto fid = this->data->make_data_id (pid, fmr::Data::Elem_form);
    bool is_found = this->data->enum_vals.count(fid) > 0;
    auto form = fmr::Elem_form::Unknown;
    if (is_found) {
      form = fmr::Elem_form(this->data->enum_vals.at(fid).data [this->sims_ix]);
    }
#if 0
    this->elem_info = fmr::Elem_info (form, 4);//TODO each_node_n
#else
    this->elem_info.form = form;
    this->elem_info.elem_d
      = fmr::elem_form_d [fmr::enum2val (this->elem_info.form)];
#endif
#ifdef FMR_DEBUG
      log->label_fprintf(log->fmrout, "**** Mesh prep",
        "%s: %uD %s mesh in %uD space\n", this->model_name.c_str(),
        this->elem_info.elem_d,
        fmr::get_enum_string(fmr::elem_form_name, this->elem_info.form).c_str(),
        geom_d);
#endif
    const auto eid = this->data->make_data_id (pid, fmr::Data::Elem_n);
    is_found = this->data->local_vals.count(eid) > 0;
    this->elem_n
      = is_found ? this->data->local_vals.at(eid).data[this->sims_ix] : 0;
    if (this->elem_info.elem_d == this->geom_d && this->elem_info.elem_d > 0) {
      //TODO for now, prep only meshes matching the enclosing space dimension.
      //TODO should prep all meshes with physics assigned.
#if 0
      if (log->detail >= this->verblevel) {
        const auto label = std::to_string(this->elem_info.elem_d)+"D "
          + this->task_name+" prep ****";
        log->label_fprintf (log->fmrout, label.c_str(),"%u %s in %s\n",
          this->elem_n, this->meter_unit.c_str(), name.c_str());
      }
#endif
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
//        const auto each_conn_n = 0;//TODO each_conn_n(form)/elem_vert_n(form)?
        const auto each_jacs_n = 0;//TODO each_jacs_n(form)
        // element jacobian data (inverted, maybe transposed, may have det)
        this->ini_data_vals ({fmr::Data::Elem_conn}, elem_n * elem_info.node_n);
        this->ini_data_vals ({fmr::Data::Jacs_dets}, elem_n * each_jacs_n);
        this->data_list.push_back (fmr::Data::Elem_conn);
        this->data_list.push_back (fmr::Data::Jacs_dets);
      }
#ifdef FMR_DEBUG
      auto vals = this->locals [fmr::Data::Elem_conn];
      const auto tstr = fmr::get_enum_string (fmr::vals_name, vals.type);
      log->label_fprintf (log->fmrerr, "****  get Mesh",
        "%lu %s:%s vals...\n",
        vals.data.size(),
        name.c_str(), tstr.c_str());
#endif
      this->get_data_vals (this->model_name, this->data_list);
      if (this->elem_n > 0) {
        if (this->locals.count (fmr::Data::Elem_conn) > 0) {
          const auto sz
            = fmr::Local_int (this->locals [fmr::Data::Elem_conn].data.size());
          this->elem_info = fmr::Elem_info (form, sz / this->elem_n);
        }else{
          log->label_fprintf (log->fmrerr, "WARNING Mesh",
            "%s locals [fmr::Data::Elem_conn] not found.\n",
            this->model_name.c_str());
      } }
      if (log->timing >= this->verblevel) {
        this->meter_unit
          = fmr::get_enum_string (fmr::elem_form_name, this->elem_info.form);
      }
      fmr::perf::timer_pause (& this->time, this->elem_n);//count elems prepped
      if (log->detail >= this->verblevel) {
#if 0
        const auto label = std::to_string(this->elem_info.elem_d)+"D "
          + this->task_name+" prep";
        log->label_fprintf (log->fmrout, label.c_str(),"%u %s in %s\n",
          this->elem_n, this->meter_unit.c_str(), name.c_str());
#endif

        const auto formstr = fmr::get_enum_string (
          fmr::elem_form_name, this->elem_info.form);
        std::string pchar
          = fmr::math::poly_letter_name.at(this->elem_info.poly).first;
        const std::string label = this->task_name +" prep";
        log->label_fprintf (log->fmrout, label.c_str(),
          "%u %u-node %s%u %s...\n",
          this->elem_n, elem_info.node_n, pchar.c_str(), this->elem_info.pord,
          formstr.c_str());

      }
      fmr::perf::timer_resume (& this->time);
    }//end if this needs prepped
    fmr::perf::timer_pause (& this->time);
    return err;
  }
  int Mesh::ini_data_vals (const Data_list list, const size_t n) {int err=0;
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Geom :
          this->geoms [type] = fmr::Geom_float_vals (type, n);
          break;
        default : err+= Sims::ini_data_vals ({type}, n);
//          const auto log = this->proc->log;
//          const std::string namestr
//              = fmr::get_enum_string (fmr::vals_name,type);
//          log->label_fprintf (log->fmrerr, "WARN""ING Sims",
//            "ini_data_vals (..) type of %s not handled.\n", namestr.c_str());
    } }// }
//    if (err>0) {return Sims::ini_data_vals (list, n);}//TODO Remove?
    return err;
  }
  int Mesh::get_data_vals (const fmr::Data_id name, const Data_list list) {
    int err=0;
    const auto log = this->proc->log;
    for (auto type : list) {
      switch (fmr::vals_type [fmr::enum2val (type)]) {
        case fmr::Vals_type::Geom : {
          if (this->geoms.count (type) == 0) {err=1;}
          else {
            err= this->data->get_geom_vals (name, this->geoms.at (type));
            if ((err > 0) && type == fmr::Data::Jacs_dets) {
              err=0;// Get Elem_conn, Node_coor to calculate jacs_dets
              err= this->get_data_vals (name,
                {fmr::Data::Elem_conn, fmr::Data::Node_coor});
              if (this->verblevel <= log->detail) {
                const auto formstr = fmr::get_enum_string (
                  fmr::elem_form_name, this->elem_info.form);
                std::string pchar
                  = fmr::math::poly_letter_name.at(this->elem_info.poly).first;
//                uint each_node_n = fmr::math::poly_terms (
//                  elem_info.elem_d, elem_info.poly, elem_info.pord);
                const std::string label = this->task_name +" jacs calc";
                log->label_fprintf (log->fmrout, label.c_str(),
                  "%s (%u-node %s%u %s)...\n", this->model_name.c_str(),
                  elem_info.node_n, pchar.c_str(), this->elem_info.pord,
                  formstr.c_str());
              }
              //FIXME IAMHERE
          } }
          break;}
        default : err= Sims::get_data_vals (name, {type});
      }
#if 1
      if (err > 0) {
        const std::string valstr
          = fmr::get_enum_string (fmr::vals_name, type);
        log->label_fprintf (log->fmrerr, "WARN""ING Mesh",
          "get_data_vals (%s, {%s}) not found.\n", name.c_str(), valstr.c_str());
      }
#endif
    }
    return err;
  }
  int Mesh::run (){int err=0;
    const auto log = this->proc->log;
    if (log->detail >= this->verblevel) {
      const auto label = "run "+this->task_name;
      log->label_fprintf(log->fmrout, label.c_str(),
        "%s: %uD mesh in the %uD space of %s\n",
        this->model_name.c_str(), this->elem_info.elem_d, this->geom_d,
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
