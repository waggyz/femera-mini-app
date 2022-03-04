#include "File.hpp"
#include "Logs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void data::File::task_init (int*, char**) {
    const auto L = std::move(Data<data::Logs>::new_task (this->get_core()));
    this->add_task (std::move(L));
//    this->log = L.get();
  }
}// end femera::namespace

#undef FMR_DEBUG
