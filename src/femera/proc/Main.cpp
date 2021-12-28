#include "Main.hpp"
#include "Ftop.hpp"
#include "Fmpi.hpp"
#include "Fomp.hpp"
#include "Fcpu.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void Main:: task_init (int* argc , char** argv) {
    this->add_task (std::make_shared<proc::Ftop> (proc::Ftop(this->ptrs())));
#ifdef FMR_HAS_MPI
    this->add_task (std::make_shared<proc::Fmpi> (proc::Fmpi(this->ptrs())));
#endif
#ifdef FMR_HAS_OMP
    this->add_task (std::make_shared<proc::Fomp> (proc::Fomp(this->ptrs())));
#endif
    this->add_task (std::make_shared<proc::Fcpu> (proc::Fcpu(this->ptrs())));
    for (auto P : this->task_list) {
      P->init (argc, argv);// Work::init(..) is pure virtual
#ifdef FMR_DEBUG
      printf ("init %s\n", P->name.c_str());
#endif
    }
  }
}// end femera::namespace

#undef FMR_DEBUG
