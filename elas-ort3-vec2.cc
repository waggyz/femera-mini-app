#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
int ElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n)
    * intp_n *( uint(E->elem_conn_n)* (36+18) + 2*54 + 27 );
  this->tens_band = uint(E->elem_n)
    *( sizeof(FLOAT_PHYS)*(3*uint(E->elem_conn_n)*3+ jacs_n*10)
      +sizeof(INT_MESH)*uint(E->elem_conn_n) );
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n)
    *( sizeof(FLOAT_PHYS)* 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) 
      -1+3) +sizeof(INT_MESH) *uint(E->elem_conn_n) );
  return 0;
}
int ElastOrtho3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT part_u ){
  //FIXME Cleanup local variables.
  const int Nd = 3;//this->node_d
  const int Nf = 3;// this->node_d DOF/node
  const int Nj =10;//,d2=9;//mesh_d*mesh_d;
  const int Nc = E->elem_conn_n;// Number of Nodes/Element
  const int Ne = Nf*Nc;
  const int Nt =  4*Nc;
  const int intp_n = int(E->gaus_n);
  //const INT_ORDER elem_p =E->elem_p;
#if VERB_MAX>11
  printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
      (int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];//,f[Nt];
#if 0
  FLOAT_PHYS VECALIGNED H[12], S[9];
#endif
  // Make local copies
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
  FLOAT_PHYS VECALIGNED rotc[this->mtrl_rotc.size()];
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  std::copy( &this->mtrl_rotc[0], &this->mtrl_rotc[mtrl_rotc.size()], rotc );
  //
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const __m256d vC[4] ={
    matc[0],matc[3],matc[5],0.0,
    matc[3],matc[1],matc[4],0.0,
    matc[5],matc[4],matc[2],0.0,
    matc[6],matc[7],matc[8],0.0 };
  const FLOAT_PHYS VECALIGNED R[9+1] = {
    rotc[0],rotc[1],rotc[2],
    rotc[3],rotc[4],rotc[5],
    rotc[6],rotc[7],rotc[8],0.0};
  const __m256d vR[3]={
    _mm256_load_pd(&R[0]),
    _mm256_loadu_pd(&R[3]),
    _mm256_loadu_pd(&R[6]) };
  //      FLOAT_SOLV* RESTRICT sysf  = &part_f[0];
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const   INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i], &part_u[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf );
    }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){
#ifndef FETCH_JAC
    std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    const __m256d vJ[3]={
      _mm256_load_pd (&jac[0]),
      _mm256_loadu_pd(&jac[3]),
      _mm256_loadu_pd(&jac[6]) };
    {// Scope vf registers
    __m256d vf[Nc];
    for(int ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      __m256d vH[Nd];
      rotate_g_h( &G[0],&vH[0], Ne, &vJ[0], &intp_shpg[ip*Ne], &R[0], &u[0] );
      const FLOAT_PHYS dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifdef FETCH_JAC
        std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
#endif
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy( &u[Nd*i], &part_u[c[i]*Nd], sizeof(FLOAT_SOLV)*Nd ); }
      } }
      // [H] Small deformation tensor
      // [H][RT] : matmul3x3x3T
#if 0
      {// begin scoping unit
      __m256d vS[Nd];
      {//FIXME revert Svoigt[6] to __m256d Svoigt[2]?
      FLOAT_PHYS VECALIGNED Svoigt[6];
      compute_ort_s_voigt( &Svoigt[0], &vH[0], &vC[0], dw );
      rotate_s_voigt( &vS[0], &Svoigt[0], &vR[0] );
      }
#else
      // Reuse vH instead of new vS
      {//FIXME revert Svoigt[6] to __m256d Svoigt[2]?
      FLOAT_PHYS VECALIGNED Svoigt[6];
      compute_ort_s_voigt( &Svoigt[0], &vH[0], &vC[0], dw );
      rotate_s_voigt( &vH[0], &Svoigt[0], &vR[0] );
      }
#endif
      // [S][R] : matmul3x3x3, R is transposed
      // initialize element f
#if 0
      // Hmm... switch case is slower...
      if(ip==0){
        switch(elem_p){
        case(1):
          for(int i=0; i< 4; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
          break;
        case(2):
          for(int i=0; i<10; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
          break;
        case(3):
          for(int i=0; i<20; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
          break;
        }
      }
#else
#if 1
      if(ip==0){
        for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(Nc>4){
          for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(Nc>10){
          for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        }
        }
      }
#else
      if(ip==0){
        for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      }
#endif
#endif
#if 0
      accumulate_f( &vf[0], &vS[0], &G[0], Nc );
      } // end variable scope
#else
      accumulate_f( &vf[0], &vH[0], &G[0], Nc );
#endif
    }//end intp loop
#if VERB_MAX>12
    printf( "ff:");
    for(uint j=0;j<Ne;j++){
      if(j%mesh_d==0){printf("\n"); }
      printf("%+9.2e ",f[j]);
    } printf("\n");
#endif
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      double VECALIGNED sf[4];
      _mm256_store_pd(&sf[0],vf[i]);
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<3; j++){
//#pragma omp atomic write
        part_f[3*conn[i]+j] = sf[j];
      }
#if 0
    if( (ie+1) < ee ){
    const   INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
        vf[i]=_mm256_loadu_pd(&part_f[3*c[i]]);
    }
#endif
    }//--------------------------------------------------- N*3 =  3*N FLOP
    }// end f register scope
  }// end elem loop
return 0;
}
