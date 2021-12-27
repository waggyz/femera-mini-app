#ifndef FEMERA_FOMP_IPP
#define FEMERA_FOMP_IPP

#include "omp.h"

namespace femera {
  inline
  proc::Fomp::Fomp (femera::Work::Make_work_t W) noexcept {
    this->name ="OpenMP";
    std::tie (this->proc, this->data, this->test) = W;
  }
  inline
  proc::Fomp::Fomp () noexcept {
    this->name ="OpenMP";
    this->info_d = 3;
  }
  inline
  void proc::Fomp::task_exit () {
  }
}//end femera namespace

//end FEMERA_FOMP_IPP
#endif
