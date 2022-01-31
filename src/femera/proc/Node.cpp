#include "Node.hpp"

#include <unistd.h>
#include <limits.h>

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

}//end femera:: namespace
