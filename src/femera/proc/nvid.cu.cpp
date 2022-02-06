#ifdef FMR_HAS_NVIDIA
#include "nvid.hpp"

#include <cstdint>
#include <string>



#include <cstdio>


#include "cuda.h"
#include "cuda_runtime.h"

namespace femera {

/* https://stackoverflow.com/questions/14038589
     /what-is-the-canonical-way-to-check-for-errors-using-the-cuda-runtime-api
*/
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void
gpuAssert (cudaError_t code, const char *file, int line, bool abort=true) {
   if (code != cudaSuccess) {
      fprintf(stderr,"GPUassert: %s %s %d\n",
        cudaGetErrorString(code), file, line);
//      if (abort) { exit(code); }//TODO_FMR_THROW
   }
}

  fmr::Local_int proc::nvid::get_node_card_n () {
    int card_n=0; gpuErrchk( cudaGetDeviceCount (& card_n) );
    return fmr::Local_int (card_n);
  }
  std::string proc::nvid::get_card_name (fmr::Local_int ix) {
    cudaDeviceProp prop;
    gpuErrchk( cudaGetDeviceProperties (& prop, ix) );
    return std::string (prop.name);
  }

}//end femera:: namespace
//end FMR_HAS_NVIDIA/#endif
#endif
