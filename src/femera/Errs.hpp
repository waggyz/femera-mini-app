#ifndef FEMERA_HAS_ERRS_HPP
#define FEMERA_HAS_ERRS_HPP

#include <exception>
#include <cstdio>     // std::printf

namespace femera {
  class Errs : public std::exception {
  public:
  /** Constructor (C strings).
    *  @param message C-style string error message.
    *                 The string contents are copied upon construction.
    *                 Hence, responsibility for deleting the char* lies
    *                 with the caller.
    */
  explicit Errs(const char* message)
      : msg(message) {}

  /** Constructor (C++ STL strings).
    *  @param message The error message.
    */
  explicit Errs(const std::string& message)
      : msg(message) {}

  /** Destructor.
    * Virtual to allow for subclassing.
    */
  virtual ~Errs() noexcept {}

  /** Returns a pointer to the (constant) error description.
    *  @return A pointer to a const char*. The underlying memory
    *          is in posession of the Errs object. Callers must
    *          not attempt to free the memory.
    */
  virtual const char* what() const noexcept {
      return msg.c_str();
  }
  virtual void print() const noexcept {
      printf ("%s\n", msg.c_str());
  }
  protected:
    /** Error message.
     */
    std::string msg;
  };
}//end femera:: namespace

//#include "Errs.ipp"

//end FEMERA_HAS_ERRS_HPP
#endif
