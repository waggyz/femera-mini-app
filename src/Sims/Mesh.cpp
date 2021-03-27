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
    if (log->detail >= this->verblevel) {
      fmr::perf::timer_pause (& this->time);
      const std::string label = this->task_name+" prep";
      log->label_fprintf (log->fmrout, label.c_str(), "%s\n", name.c_str());
      fmr::perf::timer_resume (& this->time);
    }
    //
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
