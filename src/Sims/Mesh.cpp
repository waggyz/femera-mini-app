#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Mesh::Mesh (Sims* F) noexcept {
    this->proc=F->proc; this->data=F->data;
    this->from = F->send;
    this->send = {from.hier_lv, fmr::Schedule::Block, fmr::Concurrency::Serial};
    this->sims_size = F->sims_size;
    this-> work_type = work_cast (Plug_type::Mesh);
//    this->base_type = work_cast (Base_type::Geom);// TODO Remove?
    this-> task_name ="Mesh";
    this->meter_unit ="elem";
    this->model_name ="(Mesh master)";
    this-> verblevel = 8;
    this-> part_algo = fmr::Partition::None;
  }
  int Mesh::chck (){
    return 0;
  }
  int Mesh::run (){int err=0;
 //   this->prep ();
    return this->exit (err);
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
