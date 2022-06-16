#include "../core.h"
#include "Node.hpp"

#include <unistd.h>
#include <limits.h>
#include <cstdio> 

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {

  proc::Node::Node (const femera::Work::Core_ptrs_t core)
  noexcept : Proc (core) {
    char hostname [HOST_NAME_MAX];
    const auto err = gethostname (hostname, HOST_NAME_MAX);
    if (err) { this->name = "node"; }
    else     { this->name = std::string (hostname); }
    this->abrv ="node";
    this->task_type = task_cast (Task_type::Node);
    this->info_d = 3;
  }
  void proc::Node::task_exit () {
    const auto use = fmr::form::si_unit (this->get_used_byte (), "B");
    const auto max = fmr::form::si_unit (this->get_dram_byte (), "B");
    const auto label = femera::form::text_line (80, "%4s %4s DRAM",
      this->get_base_abrv().c_str(), this->get_abrv ().c_str());
    femera::form::name_line (::stdout, 14, 80, label,
      "%s /%s maximum resident set size of this process",
      use.c_str(), max.c_str());
  }

}//end femera:: namespace
#undef FMR_DEBUG
