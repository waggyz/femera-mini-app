#ifndef FEMERA_FLOG_HPP
#define FEMERA_FLOG_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from File.
  class Logs : public Data <Logs> { private: friend class Data;
  private:
    Logs (femera::Work::Core_ptrs) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::file:: namespace

#include "Logs.ipp"

//end FEMERA_FLOG_HPP
#endif
