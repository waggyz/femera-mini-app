#ifndef FEMERA_DATA_LOGS_HPP
#define FEMERA_DATA_LOGS_HPP

#include "../core.h"
//#include "../Data.hpp"
//#include "../../fmr/form.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from Data.//TODO Change to Dlim?
  class Logs : public Data <Logs> {// private: friend class Data;
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Logs (femera::Work::Core_ptrs_t) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Logs.ipp"

//end FEMERA_DATA_LOGS_HPP
#endif
