#include "../core.h"
#include "Jobs.hpp"

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {
  void sims::Jobs::task_init (int* argc, char** argv) {
    if (this->proc == nullptr) {
      this->proc_ptr = proc::Main::new_task (this->get_core());
      this->proc = this->proc_ptr.get();
      this->proc->init (argc,argv);
    }
    if (this->proc == nullptr) {
      throw std::runtime_error ("Failed to initialize processing environment.");
    }
    if (this->proc->proc == nullptr) { this->proc->proc = this->proc; }
    if (this->test == nullptr) {
      this->test_ptr = test::Beds::new_task (this->get_core());
      this->test = this->test_ptr.get();
      this->test->init (argc,argv);
    }
    if (this->test == nullptr) {
      throw std::runtime_error ("Failed to initialize testbeds.");
    }
    if (this->test->test == nullptr) { this->test->test = this->test; }
    if (this->proc->test == nullptr) { this->proc->test = this->test; }
    if (this->data == nullptr) {
      this->data_ptr = data::File::new_task (this->get_core());
      this->data = this->data_ptr.get();
      this->data->init (argc,argv);
    }
    if (this->data == nullptr) {
      throw std::runtime_error ("Failed to initialize data handler.");
    }
    if (this->data->data == nullptr) { this->data->data = this->data; }
    if (this->proc->data == nullptr) { this->proc->data = this->data; }
    if (this->test->data == nullptr) { this->test->data = this->data; }
#ifdef FMR_DEBUG
    printf ("Jobs: init %s\n",proc->name.c_str());
    printf ("Jobs: init %s\n",test->name.c_str());
    printf ("Jobs: init %s\n",data->name.c_str());
#endif
  }
}//end femera namespace

#undef FMR_DEBUG
