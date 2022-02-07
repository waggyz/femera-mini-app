#ifndef FEMERA_NVIDCU_HPP
#define FEMERA_NVIDCU_HPP

#include "../../fmr/fmr.hpp"

#ifdef FMR_HAS_NVIDIA
#include <string>

namespace femera { namespace proc { namespace nvid {

  fmr::Local_int get_node_card_n ();
  std::string    get_card_name   (fmr::Local_int ix);

} } }//end femera::proc::nvid:: namespace
//end FMR_HAS_NVIDIA
#endif
//end FEMERA_NVIDCU_HPP
#endif
