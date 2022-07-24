#ifndef FEMERA_DATA_LOGS_HPP
#define FEMERA_DATA_LOGS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from Data.
  class Logs final: public Data <Logs> {// private: friend class Data;
  private:
    fmr::Dim_int verb_d = 5;//TODO set by command line option -v<int>
    bool did_init_tf = false;
  public:
    fmr::Dim_int set_verb (fmr::Dim_int) noexcept;
    bool do_log (fmr::Dim_int)           noexcept;
    bool did_init ()                     noexcept;
    bool set_init (bool)                 noexcept;
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_t&) noexcept;
    std::size_t task_send (const fmr::Data_name_t& file,
      const std::string& text, fmr::Dim_int out_d=1) noexcept;
  public:
    Logs (femera::Work::Core_ptrs_t) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Logs.ipp"

//end FEMERA_DATA_LOGS_HPP
#endif
