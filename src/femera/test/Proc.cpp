#include "../core.h"
#include "Proc.hpp"

namespace femera {
  void test::Proc::task_init (int*, char**) {
    FMR_PRAGMA_OMP(omp parallel)
    {
      const auto head = femera::form::text_line (40," %4s %4s %4s",
        this->get_base_name().c_str(), this->abrv.c_str(), "ID");
      const std::string text = std::to_string(proc->get_proc_id ());
      this->data->head_line (this->data->fmrout, head, text);
    }
  }
}//end femera namespace
