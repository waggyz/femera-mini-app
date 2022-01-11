#ifndef FEMERA_HAS_ERRS_IPP
#define FEMERA_HAS_ERRS_IPP

#include <cstdio>     // std::printf

namespace femera {
  const char* Errs::what() const noexcept {
      return msg.c_str();
  }
  void Errs::print() const noexcept {
      printf ("%s\n", msg.c_str());
  }
}//end femera:: namespace

//end FEMERA_HAS_ERRS_IPP
#endif
