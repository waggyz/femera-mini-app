#include "Errs.hpp"

#include <cstdio>     // std::printf

namespace femera {
  void Errs::print ()
  const noexcept {
    printf ("%s\n", msg.c_str());
  }
}//end femera:: namespace
