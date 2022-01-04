#ifndef FEMERA_HAS_FMPI_IPP
#define FEMERA_HAS_FMPI_IPP

namespace femera {
  inline
  void proc::Fmpi::task_init (int*, char**){
  }
  inline
  void proc::Fmpi::task_exit () {
  }
  inline
  proc::Team_t proc::Fmpi::get_team_id ()
  noexcept {
    return Proc::team_id;
  }
}//end femera namespace

//end FEMERA_HAS_FMPI_IPP
#endif
