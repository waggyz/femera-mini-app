#include "../core.h"
#include "Jobs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  void sims::Jobs::task_init (int*, char**) {
    // Add a processing environment.
    if (this->proc == nullptr) {
      auto proc_ptr = proc::Main::new_task (this->get_core());
      this->proc = proc_ptr.get();
      this->add_task (proc_ptr);
    }
    if (this->proc == nullptr) {
      FMR_THROW("Failed to add a new processing environment.");
    }
    if (this->proc->proc == nullptr) { this->proc->proc = this->proc; }
#ifdef FMR_HAS_MPI
    this->do_exit_zero = true;
#endif
    // Add a data handler.
    if (this->data == nullptr) {
      auto data_ptr = data::File::new_task (this->get_core());
      this->data = data_ptr.get();
      this->add_task (data_ptr);
    }
    if (this->data == nullptr) {
      FMR_THROW("Failed to add a new data handler.");
    }
    if (this->proc->data == nullptr) { this->proc->data = this->data; }
    if (this->data->data == nullptr) { this->data->data = this->data; }
    // Add testbeds.
    if (this->test == nullptr) {
      auto test_ptr = test::Beds::new_task (this->get_core());
      this->test = test_ptr.get();
      this->add_task (test_ptr);
    }
    if (this->test == nullptr) {
      FMR_THROW("Failed to add new testbeds.");
    }
    if (this->proc->test == nullptr) { this->proc->test = this->test; }
    if (this->data->test == nullptr) { this->data->test = this->test; }
    if (this->test->test == nullptr) { this->test->test = this->test; }
  }
  void sims::Jobs::task_exit () {
#ifdef FMR_DEBUG
  FMR_THROW("Jobs::task_exit() test throw.");
#endif
  }
}//end femera namespace

#undef FMR_DEBUG
