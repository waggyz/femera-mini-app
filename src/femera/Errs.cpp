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
  void Errs::print (const std::string s, std::exception& e)
  noexcept {
    fprintf (::stderr, "%s threw %s\n", s.c_str(), e.what ());
  }
  void Errs::print (const std::string s)
  noexcept {
    fprintf (::stderr, "%s threw %s\n", s.c_str(), "an unknown exception");
  }
  /*
  void Note::print ()//TODO
  const noexcept {
    fprintf (::stdout, "%s\n", msg.c_str());
  }
  void Note::print (std::exception& e)
  noexcept {
    fprintf (::stdout, "%s\n", e.what ());
  }
  void Note::print (const std::string s, std::exception& e)
  noexcept {
    fprintf (::stdout, "%s threw %s\n", s.c_str(), e.what ());
  }
  void Note::print (const std::string s)
  noexcept {
    fprintf (::stdout, "%s threw %s\n", s.c_str(), "an unknown exception");
  }
  */
}//end femera:: namespace
