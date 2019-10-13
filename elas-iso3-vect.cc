#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
#define HAS_AVX
//
int ElastIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10);
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  return 0;
}
int ElastIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const int Nt = 4*Nc;
  const int intp_n = int(E->gaus_n);
  //const INT_ORDER elem_p =E->elem_p;
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];
#ifndef HAS_AVX
  FLOAT_PHYS VECALIGNED H[Nd*4], S[Nd*4];
#endif
  //
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT C     = &matc[0];
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&sys_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf );}
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
#ifndef HAS_AVX
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[4*i],& part_f[conn[i]*3], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#if 0
    const __m256d j0 = _mm256_load_pd (&jac[0]);  // j0 = [j3 j2 j1 j0]
    const __m256d j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    const __m256d j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
#else
    const __m256d vJ[3]={
      _mm256_load_pd(&jac[0]),
      _mm256_loadu_pd(&jac[3]),
      _mm256_loadu_pd(&jac[6]) };
#endif
  {// Scope vf registers
  __m256d vf[Nc];
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
#ifdef HAS_AVX
      __m256d vH[3];
      compute_g_h( &G[0],&vH[0], Ne, &vJ[0], &intp_shpg[ip*Ne], &u[0] );
#else
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
      for(int i=0; i<12 ; i++){ H[i]=0.0; };
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf ; k++){ G[4* i+k ]=0.0;
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
          for(int j=0; j<Nd ; j++){
            G[(4* i+k) ] += intp_shpg[ip*Ne+ Nd* i+j ] * jac[Nd* j+k ];
          }
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
          for(int j=0; j<Nf ; j++){
            H[(4* k+j) ] += G[(4* i+k) ] * u[Nf* i+j ];
          }
        }
      }//-------------------------------------------------- N*3*6*2 = 36*N FLOP
#endif
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%Nd==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      }; printf("\n");
#endif
      const FLOAT_PHYS dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifndef FETCH_U_EARLY
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sys_u[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef FETCH_JAC
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
#ifdef HAS_AVX
      {// vector register scope
      __m256d vS[3];
      compute_iso_s( &vS[0], &vH[0], C[1]*dw,C[2]*dw );
#if VERB_MAX>10
      if(ie==4){
        printf( "S[%u]:", ie );
        for(uint j=0;j<12;j++){
          if(j%4==0){printf("\n");}
          printf("%+9.2e ",S[j]);
        } printf("\n");
      }
#endif
#else
      S[ 0]=(C[0]* H[0] + C[1]* H[5] + C[1]* H[10])*dw;//Sxx
      S[ 5]=(C[1]* H[0] + C[0]* H[5] + C[1]* H[10])*dw;//Syy
      S[10]=(C[1]* H[0] + C[1]* H[5] + C[0]* H[10])*dw;//Szz

      S[1]=( H[1] + H[4] )*C[2]*dw;// S[3]= S[1];//Sxy Syx
      S[2]=( H[2] + H[8] )*C[2]*dw;// S[6]= S[2];//Sxz Szx
      S[6]=( H[6] + H[9] )*C[2]*dw;// S[7]= S[5];//Syz Szy
      S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
#if VERB_MAX>10
      if(ie==4){
        printf( "S[%u]:", ie );
        for(uint j=0;j<12;j++){
          if(j%4==0){printf("\n");}
          printf("%+9.2e ",S[j]);
        } printf("\n");
      }
#endif
#endif
      //------------------------------------------------------- 18+9 = 27 FLOP
#ifdef HAS_AVX
    if(ip==0){
      for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
    }
    accumulate_f( &vf[0], &vS[0], &G[0], Nc );
    }// end vS register scope
#else
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
#endif
      for(int i=0; i<Nc; i++){
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for(int k=0; k<Nf; k++){
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
          for(int j=0; j<Nf; j++){
            f[(4* i+k) ] += G[(4* i+j) ] * S[(4* j+k) ];// 18*N FMA FLOP
      } } }//------------------------------------------------ N*3*6 = 18*N FLOP
#endif
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
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
      for(int j=0; j<3; j++){
        part_f[3*conn[i]+j] = sf[j]; } }
  }// end vf register scope
  }//============================================================ end elem loop
  return 0;
}
