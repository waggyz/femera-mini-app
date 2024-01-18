#include "Errs.hpp"

#include <cstdio>     // std::fprintf, stderr
#include <cstring>    // std::strlen

namespace femera {
  std::size_t Errs::print ()
  const noexcept {
    fprintf (::stderr, "%s\n", msg.c_str());
    return (msg.size () + 1);
  }
  std::size_t Errs::print (std::exception& e)
  noexcept {
    fprintf (::stderr, "%s\n", e.what ());
    return (std::strlen (e.what ()) + 1);
  }
  std::size_t Errs::print (const std::string s, std::exception& e)
  noexcept {
    fprintf (::stderr, "%s threw %s\n", s.c_str(), e.what ());
    return (s.size () + std::strlen (e.what ()) + 8);
  }
  std::size_t Errs::print (const std::string s)
  noexcept {
    fprintf (::stderr, "%s threw %s\n", s.c_str(), "an unknown exception");
    return (s.size () + 28);
  }//
  //
}//end femera:: namespace
//
