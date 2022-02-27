#include "Errs.hpp"

#include <cstdio>     // std::fprintf, stderr

namespace femera {
  void Errs::print ()
  const noexcept {
    fprintf (::stderr, "%s\n", msg.c_str());
  }
  void Errs::print (std::exception& e)
  noexcept {
    fprintf (::stderr, "%s\n", e.what ());
  }
  void Errs::print (std::string s,std::exception& e)
  noexcept {
    fprintf (::stderr, "%s threw %s\n", s.c_str(), e.what ());
  }
}//end femera:: namespace
