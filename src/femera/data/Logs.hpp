#ifndef FEMERA_FLOG_HPP
#define FEMERA_FLOG_HPP

#include "../Data.hpp"
#include "../../fmr/form.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from File.
  class Logs : public Data <Logs> { private: friend class Data;
  private:
    fmr::Line_size_int line_sz = 80;
    fmr::Line_size_int head_sz = 15;
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
