#ifndef FEMERA_HAS_ERRS_HPP
#define FEMERA_HAS_ERRS_HPP

#include "../fmr/fmr.hpp"

#include <exception>
/*
  https://stackoverflow.com/questions/348833
    /how-to-know-the-exact-line-of-code-where-an-exception-has-been-caused
*/
#define FMR_THROW(arg) throw Errs(arg, __FILE__, __LINE__)
#define FMR_WARN( arg) throw Warn(arg, __FILE__, __LINE__)

namespace femera {
  class Errs : public std::exception {
    //https://stackoverflow.com/questions/8152720/correct-way-to-inherit-from-stdexception
  protected:
    /** Error message.
     */
    std::string msg ="";
    file_ptr = ::stderr;
  public:
    /** Constructor (C strings).
      *  @param message C-style string error message.
      *                 The string contents are copied upon construction.
      *                 Hence, responsibility for deleting the char* lies
      *                 with the caller.
      */
    explicit Errs (const char* message) : msg (message) {}
    /** Constructor (C++ STL strings).
      *  @param message The error message.
      */
    explicit Errs (const std::string& message) : msg (message) {}
    explicit Errs (const char* message, const char* file, int line)
      : msg (std::string(file)+":"+std::to_string(line)+" "+message) {}
    explicit Errs (const std::string& message, const char* file, int line)
      : msg (std::string(file)+":"+std::to_string(line)+" "+message) {}
    explicit Errs () =default;
    /** Destructor.
      * Virtual to allow for subclassing.
      */
    virtual ~Errs () noexcept {}
    /** Returns a pointer to the (constant) error description.
      *  @return A pointer to a const char*. The underlying memory
      *          is in posession of the Errs object. Callers must
      *          not attempt to free the memory.
      */
    virtual const char* what () const noexcept;
    virtual void       print () const noexcept;
    static  void       print (std::exception&) noexcept;
    static  void       print (std::string, std::exception&) noexcept;
    static  void       print (std::string) noexcept;
  };
  class Note : public Errs {
  public:
    /** Constructor (C strings).
      *  @param message C-style string error message.
      *                 The string contents are copied upon construction.
      *                 Hence, responsibility for deleting the char* lies
      *                 with the caller.
      */
    explicit Note (const char* message) {
      this->msg      = message;
      this->file_ptr =::stdout;
    }
    /** Constructor (C++ STL strings).
      *  @param message The error message.
      */
    explicit Note (const std::string& message) {
      this->msg      = message;
      this->file_ptr =::stdout;
    }
    explicit Note (const char* message, const char* file, int line) {
      this->msg      = std::string(file)+":"+std::to_string(line)+" "+message;
      this->file_ptr =::stdout;
    }
    explicit Note (const std::string& message, const char* file, int line) {
      this->msg      = std::string(file)+":"+std::to_string(line)+" "+message;
      this->file_ptr =::stdout;
    }
    /** Destructor.
      * Virtual to allow for subclassing.
      */
    ~Note () noexcept final override {}
  };
  class Warn : public Errs {
  public:
    /** Constructor (C strings).
      *  @param message C-style string error message.
      *                 The string contents are copied upon construction.
      *                 Hence, responsibility for deleting the char* lies
      *                 with the caller.
      */
    explicit Warn (const char* message) {this->msg = message;}
    /** Constructor (C++ STL strings).
      *  @param message The error message.
      */
    explicit Warn (const std::string& message) {this->msg = message;}
    explicit Warn (const char* message, const char* file, int line) {
      this->msg = std::string(file)+":"+std::to_string(line)+" "+message;}
    explicit Warn (const std::string& message, const char* file, int line) {
      this->msg = std::string(file)+":"+std::to_string(line)+" "+message;}
    /** Destructor.
      * Virtual to allow for subclassing.
      */
    ~Warn () noexcept final override {}
  };
}//end femera:: namespace

#include "Errs.ipp"

//end FEMERA_HAS_ERRS_HPP
#endif
