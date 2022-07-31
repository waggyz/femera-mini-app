#include "Errs.hpp"

#include <cstdio>     // std::fprintf, stderr

namespace femera {
  void Errs::print ()
  const noexcept {
    fprintf (this->file_ptr, "%s\n", msg.c_str());
  }
  void Errs::print (std::exception& e)
  noexcept {
    fprintf (this->file_ptr, "%s\n", e.what ());
  }
  void Errs::print (const std::string s, std::exception& e)
  noexcept {
    fprintf (this->file_ptr, "%s threw %s\n", s.c_str(), e.what ());
  }
  void Errs::print (const std::string s)
  noexcept {
    fprintf (this->file_ptr, "%s threw %s\n", s.c_str(), "an unknown exception");
  }
}//end femera:: namespace
