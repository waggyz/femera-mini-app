#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
int ElastDmv3D::Setup( Elem* E ){
  JacT  ( E );
  IniRot();
#if 0
  MtrRot();// Assume these are already rotated in the input files.
#endif
#if 0
  printf("\nelas_prop\n");
  for(uint i=0;i<elas_prop.size();i++){ printf(" %7.1e",mtrl_matc[i]);
    if(!((i+1)%8)){printf("\n");} }
  printf("\nmtrl_matc\n");
  for(uint i=0;i<mtrl_matc.size();i++){ printf(" %7.1e",mtrl_matc[i]);
    if(!((i+1)%8)){printf("\n");} }
  printf("\nmtrl_dmat\n");
  for(uint i=0;i<mtrl_dmat.size();i++){ printf(" %7.1e",mtrl_dmat[i]);
    if(!((i+1)%8)){printf("\n");} }
#endif
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10);
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 36*2 + 10 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  //
#if 0
  this->mtrl_matc.resize(48);
  std::copy( &this->mtrl_dmat[0], &this->mtrl_dmat[48], &this->mtrl_matc[0] );
#endif
  //
  return 0;
}
int ElastDmv3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT part_u ){
  part_resp_glob( E, this, e0, ee, part_f, part_u,
    [](__m256d vH[3], const FLOAT_PHYS D[48], const FLOAT_PHYS dw){
      compute_dmv_s( &vH[0], &D[0], dw );
      return 0;
      }
    );
  return 0;
}
#if 0
int ElastDmv3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* RESTRICT part_f, const FLOAT_SOLV* RESTRICT part_u ){
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
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED align_dmat[48];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_dmat[0], &this->mtrl_dmat[48], align_dmat );
  //
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_PHYS* RESTRICT D     = &align_dmat[0];
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  if(e0<ee){
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&part_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
    const __m256d vJ[3]={
      _mm256_loadu_pd(&Ejacs[Nj*ie  ]),  // j0 = [j3 j2 j1 j0]
      _mm256_loadu_pd(&Ejacs[Nj*ie+3]),  // j1 = [j6 j5 j4 j3]
      _mm256_loadu_pd(&Ejacs[Nj*ie+6]) };// j2 = [j9 j8 j7 j6]
    const FLOAT_MESH det=Ejacs[Nj*ie+9];
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    {// Scope vf registers
    __m256d vf[Nc];
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      __m256d vH[3];
      compute_g_h( &G[0],&vH[0], Nc, &vJ[0], &intp_shpg[ip*Ne], &u[0] );
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%Nd==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      const FLOAT_PHYS dw = det * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
        const INT_MESH* RESTRICT cnxt = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& part_u[cnxt[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
      } }
#if 0
      __m256d vS[3]; compute_dmv_s( &vS[0], &vH[0], &D[0], dw );
#else
      compute_dmv_s( &vH[0], &D[0], dw );
#endif
#if 0
      printf("vH:\n");
      print_m256(vH[0]); print_m256(vH[1]); print_m256(vH[2]);
      printf("s:\n"); print_m256(s0); print_m256(s1);
      printf("vS:\n");
      print_m256(vS[0]); print_m256(vS[1]); print_m256(vS[2]);
#endif
      if(ip==0){
        for(int i=0; i<Nc; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
      }
      accumulate_f( &vf[0], &vH[0], &G[0], Nc );
      //}// end vS register scope
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
#endif