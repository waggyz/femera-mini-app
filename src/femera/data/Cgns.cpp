#include "Cgns.hpp"

#ifdef FMR_HAS_CGNS
#include "pcgnslib.h"
#endif

namespace femera {
  void data::Cgns::task_init (int*, char**) {
    this->version = std::to_string( CGNS_VERSION / 1000 )
      + "." + std::to_string( CGNS_VERSION % 1000 );
  }
}//end femera:: namespace


