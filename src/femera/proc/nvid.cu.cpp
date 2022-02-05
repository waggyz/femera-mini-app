#include "nvid.hpp"

#include <cstdint>


#include <string>

namespace femera { namespace proc {
#ifdef FMR_HAS_NVIDIA
  ::fmr::Local_int nvid::get_node_card_n () {
    int card_n; cudaGetDeviceCount (& card_n);
    return fmr::Local_int (card_n);
  }
  std::string nvid::get_card_name (fmr::Local_int ix) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties (& prop, ix);
    return std::string (prop.name);
  }

#endif
} }//end femera::proc::namespace