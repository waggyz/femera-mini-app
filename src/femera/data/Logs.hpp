#ifndef FEMERA_DATA_LOGS_HPP
#define FEMERA_DATA_LOGS_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from Data.
FMR_WARN_INLINE_OFF
  // Suppress -Winline warning for ~Logs () destructor made here implicitly.
  class Logs final: public Data <Logs> {// private: friend class Data;
FMR_WARN_INLINE_ON
  private:
    std::unordered_map <fmr::Data_name_t, FILE*> open_file_map = {
      { fmr::err  , ::stderr },// verb_d >= 0 (quiet)
      { fmr::log  , ::stdout },// verb_d >= 1 default: main thread
      { fmr::out  , ::stdout } // verb_d >= 1 default: all threads
    };
    fmr::Dim_int verb_d = 5;//TODO set by command line option -v<int>
  public:
    fmr::Dim_int get_verb ()    noexcept;
    fmr::Dim_int set_verb (int) noexcept;// returns verb_d after set
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_t&)
    noexcept;
    fmr::Global_int task_send (const fmr::Data_name_t& file, const std::string&)
    noexcept;
  public:
    Logs (femera::Work::Core_ptrs_t) noexcept;
    Logs () = delete;//NOTE Use the constructor above.
  };
  //
} }//end femera::data:: namespace

#include "Logs.ipp"

//end FEMERA_DATA_LOGS_HPP
#endif
