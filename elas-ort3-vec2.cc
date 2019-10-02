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
  FLOAT_SOLV *part_f, const FLOAT_SOLV* part_u ){
  //FIXME Cleanup local variables.
  const int Nd = 3;//this->node_d
  const int Nf = 3;// this->node_d DOF/node
  const int Nj =10;//,d2=9;//mesh_d*mesh_d;
  const int Nc = E->elem_conn_n;// Number of Nodes/Element
  const int Ne = Nf*Nc;
  const int Nt =  4*Nc;
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
#if VERB_MAX>11
  printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
      (int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_PHYS dw;
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];//,f[Nt];
  FLOAT_PHYS VECALIGNED H[12], S[9];//FIXME S[7]
  // Make local copies
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C);
  const __m256d c0 = _mm256_set_pd(0.0,C[5],C[3],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[4],C[1],C[3]);
  const __m256d c2 = _mm256_set_pd(0.0,C[2],C[4],C[5]);
  //c3 = _mm256_set_pd(0.,C[7],C[8],C[6]);
  const FLOAT_PHYS VECALIGNED R[9+1] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8],0.0};
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &part_u[0];
  //      FLOAT_SOLV* RESTRICT sysf  = &part_f[0];
  {// Scope vf registers
  __m256d vf[Nc];
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const   INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i], &sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf );
#if 0
         vf[i]=_mm256_loadu_pd(&part_f[3*c[i]]);
#endif
    }
  }
  //for(int i=0; i<Nc; i++){const vf[i]=_mm256_set1_pd(0.0); }
  //for(int i=0; i<Nc; i++){const __m256d zs={0.0,0.0,0.0,0.0}; vf[i]=zs; }
  for(INT_MESH ie=e0;ie<ee;ie++){
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
  const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
  const __m256d j0 = _mm256_load_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
  const __m256d j1 = _mm256_loadu_pd(&jac[3]); // j1 = [j6 j5 j4 j3]
  const __m256d j2 = _mm256_loadu_pd(&jac[6]); // j2 = [j9 j8 j7 j6]
  for(int ip=0; ip<intp_n; ip++){
    //G = MatMul3x3xN( jac,shg );
    //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
    compute_g_h( &G[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne], &R[0], &u[0] );
#if VERB_MAX>12
    printf( "Small Strains (Elem: %i):", ie );
    for(uint j=0;j<H.size();j++){
      if(j%mesh_d==0){printf("\n");}
      printf("%+9.2e ",H[j]);
    }; printf("\n");
#endif
    dw = jac[9] * wgt[ip];
    if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifdef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
#endif
      const   INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
        std::memcpy( &u[Nd*i], &sysu[c[i]*Nd], sizeof(FLOAT_SOLV)*Nd ); }
    } }
    // [H] Small deformation tensor
    // [H][RT] : matmul3x3x3T
    compute_ort_s( &S[0], &H[0],&C[0],c0,c1,c2, dw );
#if VERB_MAX>12
    printf( "Stress (Natural Coords):");
    for(uint j=0;j<9;j++){
      if(j%3==0){printf("\n");}
      printf("%+9.2e ",S[j]);
    } printf("\n");
#endif
    // [S][R] : matmul3x3x3, R is transposed
    //accumulate_f( &f[0], &part_f[0], &conn[0], &R[0], &S[0], &G[0] );
    {// begin scoping unit
    __m256d a[3];
    rotate_s( &a[0], &R[0], &S[0] );
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
    if(ip==0){
      for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      if(elem_p>1){
        for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      if(elem_p>2){
        for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      }
      }
    }
#endif
    accumulate_f( &vf[0], &a[0], &G[0], elem_p );
    } // end variable scope
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
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<3; j++){
        double VECALIGNED sf[4];
        _mm256_store_pd(&sf[0],vf[i]);
        part_f[3*conn[i]+j] = sf[j];
      }
#if 0
      if( (ie+1) < ee ){
      const   INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
         vf[i]=_mm256_loadu_pd(&part_f[3*c[i]]);
      }
#endif
    }//--------------------------------------------------- N*3 =  3*N FLOP
  }// end elem loop
  }// end f register scope
return 0;
}
