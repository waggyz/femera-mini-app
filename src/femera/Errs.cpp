#include "Errs.hpp"

#include <cstdio>     // std::fprintf, stderr

namespace femera {
  void Errs::print ()
  const noexcept {
    fprintf (::stderr, "%s\n", msg.c_str());
  }
}//end femera:: namespace
