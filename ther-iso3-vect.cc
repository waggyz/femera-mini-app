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
  //const int Nf = 1;// this->node_d DOF/node
  const int Nj = 10;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nd*Nc;
  const int intp_n = int(E->gaus_n);
  //const INT_ORDER elem_p =E->elem_p;
  const FLOAT_PHYS C = this->mtrl_matc[0];//TODO Cx,Cy,Cz in one __m256d?
  __m256d Cv = _mm256_setzero_pd();
  if (mtrl_matc.size()==3){
    Cv = _mm256_set_pd(0.0,this->mtrl_matc[2],mtrl_matc[1],mtrl_matc[0]);
  }
  const int NC = this->mtrl_matc.size();
  FLOAT_PHYS u[Nc];
#ifdef THIS_FETCH_JAC
  FLOAT_MESH VECALIGNED jac[Nj];
#endif
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
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
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){ u[i]=sys_u[c[i]]; }
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
    FLOAT_PHYS f[Nc];
    for(int i=0; i<Nc; i++){ f[i]=part_f[conn[i]]; }
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      __m256d vH =_mm256_setzero_pd();
      __m256d vG[Nc];
      // G = MatMul3x3xN( jac,shg );
      // H = MatMul3xNx3T( G,u );// [H] Thermal gradient
      thermal_g_h( &vG[0],&vH, Nc, &vJ[0], &intp_shpg[ip*Ne], &u[0] );
      FLOAT_PHYS Cdw = 0.0;
      switch(NC){
      case 1:{
#ifdef THIS_FETCH_JAC
        Cdw = C * jac[9] * wgt[ip];
#else
        Cdw = C * det * wgt[ip];
#endif
      break;}
      case 3:{
#ifdef THIS_FETCH_JAC
        Cdw = jac[9] * wgt[ip];
#else
        Cdw = det * wgt[ip];
#endif
      break;}
      default:{//TODO do this check earlier.
        fprintf(stderr,
          "ERROR Please use 1 or 3 conductivity values, not %i.\n", NC);
        return 1;
      } }
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
        const INT_MESH* RESTRICT cnxt = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for (int i=0; i<Nc; i++){ u[i]=sys_u[cnxt[i]]; }
#ifdef THIS_FETCH_JAC
        std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
      // Reuse vH instead of new vS
      if(NC==1){
        thermal_iso_s(& vH, Cdw );
      } else {
        thermal_xyz_s(& vH, Cv, Cdw );
      //TODO ortho could be more efficient?
      }
#if 0
      if(ip==0){
        for(int i=0; i<Nc; i++){ f[i]=part_f[conn[i]]; }
      }
#endif
      accumulate_q( &f[0], vH, &vG[0], Nc );
    }//========================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){ part_f[conn[i]] = f[i]; }
  }//============================================================ end elem loop
  return 0;
}
#undef THIS_FETCH_JAC
