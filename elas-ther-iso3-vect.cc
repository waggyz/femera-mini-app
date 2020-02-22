#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

#ifdef FETCH_JAC
#define THIS_FETCH_JAC
#endif
//
int ThermIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (24+6) + 5 );//FIXME These are wrong
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * uint(E->elem_conn_n) *( uint(E->elem_conn_n)*2 );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * uint(E->elem_conn_n) *( uint(E->elem_conn_n) +1);
  return 0;
}
int ThermIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 4;// this->node_d DOF/node
  const int Nj = 10;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nd*Nc;
  const int intp_n = int(E->gaus_n);
  //const INT_ORDER elem_p =E->elem_p;
  const FLOAT_PHYS C = this->mtrl_matc[0];
  FLOAT_PHYS u[Nf];
#ifdef THIS_FETCH_JAC
  FLOAT_MESH VECALIGNED jac[Nj];
#endif
  FLOAT_PHYS VECALIGNED intp_shpf[intp_n*Nc];
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpf[0], &E->intp_shpf[intp_n*Nc], intp_shpf );
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
  //
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  if(e0<ee){
#ifdef THIS_FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&sys_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifdef THIS_FETCH_JAC
    const __m256d vJ[3]={
      _mm256_load_pd(&jac[0]),   // j0 = [j3 j2 j1 j0]
      _mm256_loadu_pd(&jac[3]),  // j1 = [j6 j5 j4 j3]
      _mm256_loadu_pd(&jac[6]) };// j2 = [j9 j8 j7 j6]
#else
    const __m256d vJ[3]={
      _mm256_loadu_pd(&Ejacs[Nj*ie  ]),  // j0 = [j3 j2 j1 j0]
      _mm256_loadu_pd(&Ejacs[Nj*ie+3]),  // j1 = [j6 j5 j4 j3]
      _mm256_loadu_pd(&Ejacs[Nj*ie+6]) };// j2 = [j9 j8 j7 j6]
    const FLOAT_MESH det=Ejacs[Nj*ie+9];
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    __m256d vf[Nc];
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      FLOAT_PHYS VECALIGNED G[Nf*Nc]
      // G = MatMul3x3xN( jac,shg );
      // H = MatMul3xNx3T( G,u );
      //
      // H[0:2,0:2] Displacement gradient
      // H[  3,0:2] Thermal gradient
      // H[  3,3  ] Temperature
      __m256d vH[4];
      thermelast_g_h_t(& G[0],& vH[0],...
        Nc,& vJ[0],& intp_shpg[ip*Ne],& intp_shpf[ip*Nc], & u[0] );
#ifdef THIS_FETCH_JAC
      const FLOAT_PHYS dw = jac[9] * wgt[ip];
#else
      const FLOAT_PHYS dw = det * wgt[ip];
#endif
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
        const INT_MESH* RESTRICT cnxt = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sys_u[cnxt[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef THIS_FETCH_JAC
        std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
      // vH[0]: Hxx Hxy Hxz --
      // vH[1]: Hyx Hyy Hyz --
      // vH[2]: Hzx Hzy Hzz --
      // vH[3]: Tx  Ty  Tz  T
      // Reuse vH instead of new vS.
      thermelast_iso_s(& vH[0],& C[0], dw );
      // vH[0]: Sxx Sxy Sxz Tx
      // vH[1]: Sxy Syy Syz Ty
      // vH[2]: Sxz Syz Szz Tz
      if(ip==0){
        for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[Nf*conn[i]]); }
      }
      accumulate_f( &vf[0],& vH[0], &G[0], Nc );
    }//========================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      double VECALIGNED sf[4];
      _mm256_store_pd(&sf[0],vf[i]);
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<4; j++){
        part_f[4*conn[i]+j] = sf[j]; }
    }// end elem loop
  return 0;
}
#undef THIS_FETCH_JAC
