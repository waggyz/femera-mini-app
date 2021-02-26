#ifndef FMR_API_PHYS_HPP
#define FMR_API_PHYS_HPP
/** */
//#include "core.h"
#include "data.hpp"
//#include "Phys.hpp"

namespace fmr {namespace phys {

// Inline functions ===========================================================
#include <immintrin.h>
//#include <stdio.h>

static inline int fmr_phys_voigt_2d_index(const int i,const int j ){
  // 2D Voigt vector index {xx,yy,xy}
  // from 2x2 tensor indices i,j
  #if 1
  return (i==j) ? i : 2;
  #else
  return int( i!=j ) * ( 1+j ) + i;
  #endif
}
static inline int fmr_phys_voigt_3d_index(const int i,const int j ){
  // 3D Voigt vector index {xx,yy,zz,xy,xz,yz}
  // from 3x3 tensor indices i,j
  #if 0
  return (i==j) ? i : 2+i+j;
  #else
  return int( i!=j ) * ( 2+j ) + i;
  #endif
}
#if 0
static inline void dbug_print_m256(const __m256d v ){// AVX register print
  double V[4];_mm256_store_pd(& V[0], v );
    printf("%9.2e %9.2e %9.2e %9.2e\n", V[0],V[1],V[2],V[3] );
    }
#endif
} }//end fmr::phys namespace

#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Phys.hpp")
//end FMR_API_PHYS_HPP
#endif
