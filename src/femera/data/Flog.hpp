#ifndef FEMERA_FLOG_HPP
#define FEMERA_FLOG_HPP

#include "../Data.hpp"

namespace femera { namespace data {
  class Flog;// Derive a CRTP concrete class from File.
  class Flog : public Data <Flog> {
  public:
    Flog (femera::Work::Core_t) noexcept;
    Flog () noexcept;
    void task_init (int* argc, char** argv);
    void task_exit ();
  };
} }//end femera::file:: namespace

#include "Flog.ipp"

//end FEMERA_FLOG_HPP
#endif
