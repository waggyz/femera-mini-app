#include "Vals.hpp"

#define FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera { namespace data {
  Vals::Vals ()
  noexcept {
    this->name_vals.reserve (this->map_init_size);
    this->name_ints.reserve (this->map_init_size);
  }
  Vals::Vals (const fmr::Local_int n)
  noexcept : map_init_size (n) {
    this->name_vals.reserve (n);
    this->name_ints.reserve (n);
  }
} }//end femera::data:: namespace

#undef FMR_DEBUG
