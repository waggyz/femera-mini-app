#ifndef FEMERA_HAS_FMPI_IPP
#define FEMERA_HAS_FMPI_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
# if 0
  inline
  proc::Team_t proc::Fmpi::get_team_id ()
  noexcept {
    return Proc::team_id;
  }
#   endif
}//end femera namespace

#undef FMR_DEBUG

//end FEMERA_HAS_FMPI_IPP
#endif
