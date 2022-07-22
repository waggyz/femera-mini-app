#ifndef FEMERA_DATA_LOGS_HPP
#define FEMERA_DATA_LOGS_HPP

// Below used in Logs.cpp
#include "File.hpp"
//TODO refactor to include "../Data.hpp" instead of "File.hpp"
//#include "../Data.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from Data.
  class Logs final: public Data <Logs> {// private: friend class Data;
  private:
    fmr::Dim_int verb_d = 5;//TODO set by command line option -v<int>
  public:
    fmr::Dim_int set_verb (fmr::Dim_int) noexcept;
    bool do_log (fmr::Dim_int) noexcept;
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_NEW_t&){return true;}//TODO to .?pp
  public:
    Logs (femera::Work::Core_ptrs_t) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Logs.ipp"

//end FEMERA_DATA_LOGS_HPP
#endif
