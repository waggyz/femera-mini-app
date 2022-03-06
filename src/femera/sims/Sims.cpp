#include "Sims.hpp"
#include "../core.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void sims::Sims::task_init (int*, char**) {
  }
  void sims::Sims::task_exit () {
  }
}//end femera namespace

#undef FMR_DEBUG
