#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

// Vectorize f calculation
#define VECTORIZED
#define VECT_C
// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
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
  FLOAT_SOLV* sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const int Nt = 4*Nc;
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_PHYS dw;
  FLOAT_MESH __attribute__((aligned(32))) jac[Nj];
  FLOAT_PHYS __attribute__((aligned(32))) G[Nt], u[Ne];
  FLOAT_PHYS __attribute__((aligned(32))) H[Nd*4], S[Nd*4];//FIXME S size
  //
  FLOAT_PHYS __attribute__((aligned(32))) intp_shpg[intp_n*Ne];
  FLOAT_PHYS __attribute__((aligned(32))) wgt[intp_n];
  FLOAT_PHYS __attribute__((aligned(32))) C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C );
#ifdef VECT_C
  const __m256d c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  const __m256d c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
  //__m256d c3; c3 = _mm256_set_pd(0.0,C[2],C[2],C[2]);
#endif
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
#ifndef VECTORIZED
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
#endif
  {// Scope vf registers
  __m256d vf[Nc];
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
      std::memcpy( & u[Nf*i],&sysu[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf );}
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
#ifndef VECTORIZED
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[4*i],& sysf[conn[i]*3], sizeof(FLOAT_SOLV)*Nf ); }
#endif
    const __m256d j0 = _mm256_load_pd (&jac[0]);  // j0 = [j3 j2 j1 j0]
    const __m256d j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    const __m256d j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
#ifdef VECTORIZED
      compute_g_h( &G[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne], &u[0] );
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
      dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifndef FETCH_U_EARLY
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef FETCH_JAC
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
#ifdef VECT_C
      compute_iso_s( &S[0], &H[0],C[2],c0,c1,c2, dw );
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
#ifdef VECTORIZED
    if(ip==0){
      for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
      if(elem_p>1){
        for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
      if(elem_p>2){
        for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
      }
      }
    }
    __m256d a[3];
    a[0] = _mm256_load_pd(&S[0]); // [a3 a2 a1 a0]
    a[1] = _mm256_load_pd(&S[4]); // [a6 a5 a4 a3]
    a[2] = _mm256_load_pd(&S[8]); // [a9 a8 a7 a6]
    accumulate_f( &vf[0], &a[0], &G[0], elem_p );
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
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<3; j++){
        double __attribute__((aligned(32))) sf[4];
        _mm256_store_pd(&sf[0],vf[i]);
        sys_f[3*conn[i]+j] = sf[j]; } }
  }//============================================================ end elem loop
  }// end vf register scope
  return 0;
}
#undef VECTORIZED
