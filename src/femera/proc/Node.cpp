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
    this->abrv ="node";
    this->task_type = task_cast (Plug_type::Node);
    this->info_d = 3;
  }
  void proc::Node::task_exit () {
    const auto bytes = fmr::form::si_unit_string (this->get_used_byte (), "B");
    const auto head = femera::form::text_line (80, " %4s %4s DRAM",
      this->get_base_name().c_str(), this->abrv.c_str());
    femera::form::head_line (::stdout, 15, 80, head,
      "%s maximum resident set size of this process", bytes.c_str());
  }

}//end femera:: namespace
#undef FMR_DEBUG
