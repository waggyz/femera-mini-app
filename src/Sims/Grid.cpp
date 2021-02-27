#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Grid::Grid (Sims* F) noexcept {
    this->proc=F->proc; this->data=F->data;
    this->       work_type = work_cast (Plug_type::Grid);
//    this->base_type = work_cast (Base_type::Geom);// TODO Remove?
    this->       task_name ="Grid";
    this->      meter_unit ="cell";
    this->      model_name ="(Grid master)";
    this->       verblevel = 8;
    this->            plan = F->dist_to_plan;//fmr::Schedule::List;
    this->            cncr = F->dist_to_cncr;//fmr::Concurrency::Serial;
    this->         hier_lv = F->dist_to_hier_lv;
    this-> dist_to_hier_lv = hier_lv;
    this->    dist_to_cncr = fmr::Concurrency::Serial;
    this->    dist_to_plan = fmr::Schedule::List;
    this->       part_algo = fmr::Partition::None;
  }
  int Grid::chck (){
    return 0;
  }
  int Grid::run (){int err=0;
 //   this->prep ();
    return this->exit (err);
  }
#if 0
  int Grid::prep (){
    return 0;
  }
  int Grid::init_task (int*, char**){int err=0;
    return err;
  }
  int Grid::exit_task (int err){ return err; }
#endif
}// end Femera namespace
#undef FMR_DEBUG