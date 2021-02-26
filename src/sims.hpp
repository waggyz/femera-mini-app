#ifndef HAS_API_FSIM_HPP
#define HAS_API_FSIM_HPP
/** */
//#include "core.h"
#include "data.hpp"
//#include "Sims.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
namespace fmr {
}//end fmr:: namespace

namespace fmr {namespace sims {
} }//end fmr::sims:: namespace

#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Sims.hpp")
//end HAS_API_FSIM_HPP
#endif
