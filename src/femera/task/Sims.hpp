#ifndef FEMERA_HAS_SIMS_HPP
#define FEMERA_HAS_SIMS_HPP

#include "../Task.hpp"

namespace fmr {
  enum class Application : fmr::Enum_int { None=0, Error, Unknown,
    //#include "sims-application-enum.inc"// TODO generate from pymera
    Research,
    Engineering,
    Parameter_sweep,
    Sensitivity,
    UQ,
    ML,
    end
  };
}
namespace femera { namespace task {
  class Sims;// Derive a CRTP concrete class from Task.
  class Sims final: public Task<Sims> { friend class Task;
  public:
  inline fmr::Application get_application ();
  inline fmr::Application set_application (fmr::Application);

  private:
    fmr::Application app = fmr::Application::Unknown;
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Sims (femera::Work::Core_ptrs_t) noexcept;
    Sims () =delete;
  };
} }//end femera::task:: namespace

#include "Sims.ipp"

//end FEMERA_HAS_SIMS_HPP
#endif

