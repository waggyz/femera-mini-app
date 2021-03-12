#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Part::Part (Sims* F) noexcept {
    this->parent = F; this->proc=F->proc; this->data=F->data;
    this->from = F->send;
    this->send = {from.hier_lv, fmr::Schedule::List, fmr::Concurrency::Serial};
    this-> work_type = work_cast (Base_type::Part);
//    this->base_type = work_cast (Base_type::Sims);// TODO Remove?
    this-> task_name ="Part";
    this->meter_unit ="part";
    this->model_name ="(partition master)";
    this-> verblevel = 8;
    this-> part_algo = fmr::Partition::Geom;// Meshes & Grids
    this->   tree_lv = 1;
    this->   part_ix = F->get_part_n();
    this->   data_id = this->make_id ();
  }
  int Part::run (){int err=0;
    this->prep ();
    //
    //TODO Iterate?.
    //
    return this->exit (err);
  }
  int Part::chck (){
    return 0;
  }
  fmr::Data_id Part::make_id (){
      std::vector<fmr::Local_int> path={};
      Sims* F=this; Part* P=this;
      this->tree_lv = 0;
      while (F->work_type == work_cast(Base_type::Part)
        && this->tree_lv < this->data->get_hier_max()) {
        // Walk up the Part hierarchy to the root Sims.
        this->tree_lv++;
        path.push_back (P->part_ix);
        F = F->parent;
        P = FMR_CAST_TO_DERIVED<Part*>(F->parent);
      }
      this->data_id
        = this->data->make_data_id (F->get_id(), fmr::Tree_type::Part, path);
//        = this->data->make_data_id (F->model_name, fmr::Tree_type::Part, path);
    return this->data_id;
  }
  fmr::Data_id Part::get_id () {
    return this->data_id;
  }
  int Part::prep () {int err=0;
    auto log = this->proc->log;
    if (this->part_ix == 0) {// task[0] loads the parts
      if (log->detail >= this->verblevel) {
        const std::string label = "get geom";
        log->label_fprintf (log->fmrout,
          label.c_str(),"%u/%u %s %s ...\n",
          this->part_ix, this->parent->get_part_n (),
          this->model_name.c_str(), this->get_id().c_str());
      }
      //TODO get Mesh, Grid Elem_types?
      // get_geom_size (..)//TODO Mesh, Grid sizes
      this->task.add (new Geom(this));//TODO Mesh, Grid
#if 0
      Geom* G = new Geom(this);
      // geom_d, node_n, part_1d_n, part_1d_n, part_3d_n, phys_d, mtrl_n
      //...
      //
      if (G) {delete G; G=nullptr;}
#endif
    }
    return err;
  }
#if 0
  int Part::init_task (int*, char**){int err=0;
    return err;
  }
#endif
  int Part::exit_task (int err) {return err;}
}// end Femera namespace
#undef FMR_DEBUG
