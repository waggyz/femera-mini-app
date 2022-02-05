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

  proc::Node::Node (femera::Work::Core_ptrs W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    char hostname [HOST_NAME_MAX];
    const auto err = gethostname (hostname, HOST_NAME_MAX);
    if (err) { this->name = "node"; }
    else     { this->name = std::string (hostname); }
    this->info_d = 3;
    this->abrv ="node";
  }
  void proc::Node::task_exit () {
    const auto bytes = fmr::form::si_unit_string (this->get_used_byte (), "B");
    femera::form::head_line (::stdout, 15, 80, this->abrv+" DRAM used",
      "%s maximum resident set size of this process", bytes.c_str());
  }

}//end femera:: namespace
#undef FMR_DEBUG
