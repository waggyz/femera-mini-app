#include "Jobs.hpp"
//#include "../proc/Main.hpp"

namespace femera {
  void sims::Jobs::task_init (int* argc, char** argv){
    if (this->proc == nullptr) {
//      this->proc = std::static_pointer_cast<Proc<Work>>
//        (proc::Main::new_task (argc, argv));
    }
  }
}//end femera namespace
