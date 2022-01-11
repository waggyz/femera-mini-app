#include "../core.h"
#include "Jobs.hpp"

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  void sims::Jobs::task_init (int*, char**) {
    if (this->proc == nullptr) {
      auto proc_ptr = proc::Main::new_task (this->get_core());
      this->proc = proc_ptr.get();
      this->add_task (proc_ptr);//FIXME check error
    }
    if (this->proc == nullptr) {
      FMR_THROW("Failed to initialize processing environment.");
    }
    if (this->proc->proc == nullptr) { this->proc->proc = this->proc; }
    if (this->test == nullptr) {
      auto test_ptr = test::Beds::new_task (this->get_core());
      this->test = test_ptr.get();
      this->add_task (test_ptr);//FIXME check error
    }
    if (this->test == nullptr) {
      FMR_THROW("Failed to initialize testbeds.");
    }
    if (this->test->test == nullptr) { this->test->test = this->test; }
    if (this->proc->test == nullptr) { this->proc->test = this->test; }
    if (this->data == nullptr) {
      auto data_ptr = data::File::new_task (this->get_core());
      this->data = data_ptr.get();
      this->add_task (data_ptr);//FIXME check error
    }
    if (this->data == nullptr) {
      FMR_THROW("Failed to initialize data handler.");
    }
    if (this->data->data == nullptr) { this->data->data = this->data; }
    if (this->proc->data == nullptr) { this->proc->data = this->data; }
    if (this->test->data == nullptr) { this->test->data = this->data; }
  }
  void sims::Jobs::task_exit () {
  }
}//end femera namespace

#undef FMR_DEBUG
