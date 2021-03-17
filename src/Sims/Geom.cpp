#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Geom::Geom (Sims* F) noexcept {
    this->proc=F->proc; this->data=F->data;
    this->from = F->send;
    this->send = {from.hier_lv, fmr::Schedule::Block, fmr::Concurrency::Serial};
    this->sims_size = F->sims_size;
    this-> work_type = work_cast (Base_type::Geom);
//    this->base_type = work_cast (Base_type::Sims);// TODO Remove?
    this-> task_name ="Geom";
    this->model_name ="(geometry master)";
    this-> verblevel = 8;
    this-> part_algo = fmr::Partition::Geom;
  }
  int Geom::chck (){
    return 0;
  }
  int Geom::run (){int err=0;
 //   this->prep ();
    return this->exit (err);
  }
#if 0
  int Geom::prep (){
    return 0;
  }
  int Geom::init_task (int*, char**){int err=0;
    return err;
  }
  int Geom::exit_task (int err){ return err; }
#endif
}// end Femera namespace
#undef FMR_DEBUG
