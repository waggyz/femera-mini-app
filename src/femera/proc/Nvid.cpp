#include "Nvid.hpp"
// #include "nvid.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
#ifdef FMR_HAS_NVIDIA
  proc::Nvid::Nvid (const femera::Work::Core_ptrs_t core)
  noexcept : Proc (core) {
    this->name ="NVIDIA";
    this->abrv ="gpu";
    this->task_type = task_cast (Task_type::Nvid);
  }
  void proc::Nvid::task_init (int*, char**) {
    fmr::Exit_int err = 0;
    try {
      this->team_n = proc::nvid::get_node_card_n ();
      this->name   = proc::nvid::get_card_name (0);//TODO loop over cards?
    }
    catch (const Warn& e)     { err =-1; printf ("%s\n", e.what()); }
    catch (const Errs& e)     { err = 1; printf ("%s\n", e.what()); }
    this->set_init ((err <= 0) ? true : false);
  }
  bool proc::Nvid::is_in_parallel () {
    return false;
  }
#endif
}//end femera:: namespace

#undef FMR_DEBUG
