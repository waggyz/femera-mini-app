#ifndef FEMERA_NVID_HPP
#define FEMERA_NVID_HPP

#include "../../fmr/fmr.hpp"

#include <string>

namespace femera { namespace proc { namespace nvid {
#ifdef FMR_HAS_NVIDIA

  fmr::Local_int get_node_card_n ();
  std::string    get_card_name   (fmr::Local_int ix);
//end FMR_HAS_NVIDIA
#endif
  
} } }//end femera::proc::nvid:: namespace

//end FEMERA_NVID_HPP
#endif
