#include "nvid.hpp"
#include "../Errs.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#ifdef FMR_HAS_NVIDIA

#include "cuda.h"
#include "cuda_runtime.h"

#define FMR_TRY_CUDA(ans) {femera::proc::nvid::try_cuda((ans), __FILE__, __LINE__);}
namespace femera {namespace proc {namespace nvid {
/* https://stackoverflow.com/questions/14038589
     /what-is-the-canonical-way-to-check-for-errors-using-the-cuda-runtime-api
*/
  inline void
  try_cuda (const cudaError_t code, const char *file, const int line,
  const bool do_throw=true ) {
    if (code != cudaSuccess) {
#ifdef FMR_DEBUG
      fprintf (stderr,"%s:%d %s \n", file, line, cudaGetErrorString (code));
#endif
      if (do_throw) {
        FMR_THROW( "called from\n"+std::string(file)+":"+std::to_string(line)
          +" "+cudaGetErrorString (code));
  } } }

} } }//end femera::proc::nvid namespace

namespace femera {

  fmr::Local_int proc::nvid::get_node_card_n () {
    int card_n=0; FMR_TRY_CUDA( cudaGetDeviceCount (& card_n) );
    return fmr::Local_int (card_n);
  }
  std::string proc::nvid::get_card_name (fmr::Local_int ix) {
    cudaDeviceProp prop;
    FMR_TRY_CUDA( cudaGetDeviceProperties (& prop, ix) );
    return std::string (prop.name);
  }

}//end femera:: namespace

#undef FMR_DEBUG
//end FMR_HAS_NVIDIA/#endif
#endif
