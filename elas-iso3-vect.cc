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
  FLOAT_PHYS __attribute__((aligned(32))) G[Nt], u[Ne], f[Nt];
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
  __m256d c0,c1,c2;
  c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
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
  {// Scope f registers
#ifdef VECTORIZED
  __m256d f0,f1,f2,f3;
  __m256d f4,f5,f6,f7,f8,f9;
  __m256d f10,f11,f12,f13,f14,f15,f16,f17,f18,f19;
  {// scope zs register
  const __m256d zs={0.0,0.0,0.0,0.0};
  f0=zs,f1=zs,f2=zs,f3=zs;
  f4=zs,f5=zs,f6=zs,f7=zs,f8=zs,f9=zs;
  f10=zs,f11=zs,f12=zs,f13=zs,f14=zs,f15=zs,f16=zs,f17=zs,f18=zs,f19=zs;
  }
#endif
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
    __m256d j0,j1,j2;
    j0 = _mm256_load_pd (&jac[0]);  // j0 = [j3 j2 j1 j0]
    j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
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
      //Vectorized calc for diagonal of S
      { // Scope vector registers
        __m256d s048;
        s048= _mm256_mul_pd(_mm256_set1_pd(dw),
              _mm256_add_pd(_mm256_mul_pd(c0,
              _mm256_set1_pd(H[0])),
              _mm256_add_pd(_mm256_mul_pd(c1,
              _mm256_set1_pd(H[5])),
              _mm256_mul_pd(c2,
              _mm256_set1_pd(H[10])))));
              _mm256_store_pd(&S[0], s048);
      } // end scoping unit
      S[5]=S[1]; S[10]=S[2];// Move the diagonal to their correct locations
      // Then, do the rest. Dunno if this is faster...
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
      {// Scope variables
        __m256d a036, a147, a258;
      a036 = _mm256_load_pd(&S[0]); // [a3 a2 a1 a0]
      a147 = _mm256_load_pd(&S[4]); // [a6 a5 a4 a3]
      a258 = _mm256_load_pd(&S[8]); // [a9 a8 a7 a6]
        if(ip==0){
          f0 = _mm256_loadu_pd(&sys_f[3*conn[ 0]]);
          f1 = _mm256_loadu_pd(&sys_f[3*conn[ 1]]);
          f2 = _mm256_loadu_pd(&sys_f[3*conn[ 2]]);
          f3 = _mm256_loadu_pd(&sys_f[3*conn[ 3]]);
          if(elem_p>1){
          f4 = _mm256_loadu_pd(&sys_f[3*conn[ 4]]);
          f5 = _mm256_loadu_pd(&sys_f[3*conn[ 5]]);
          f6 = _mm256_loadu_pd(&sys_f[3*conn[ 6]]);
          f7 = _mm256_loadu_pd(&sys_f[3*conn[ 7]]);
          f8 = _mm256_loadu_pd(&sys_f[3*conn[ 8]]);
          f9 = _mm256_loadu_pd(&sys_f[3*conn[ 9]]);
          }
          if(elem_p>2){
          f10 = _mm256_loadu_pd(&sys_f[3*conn[10]]);
          f11 = _mm256_loadu_pd(&sys_f[3*conn[11]]);
          f12 = _mm256_loadu_pd(&sys_f[3*conn[12]]);
          f13 = _mm256_loadu_pd(&sys_f[3*conn[13]]);
          f14 = _mm256_loadu_pd(&sys_f[3*conn[14]]);
          f15 = _mm256_loadu_pd(&sys_f[3*conn[15]]);
          f16 = _mm256_loadu_pd(&sys_f[3*conn[16]]);
          f17 = _mm256_loadu_pd(&sys_f[3*conn[17]]);
          f18 = _mm256_loadu_pd(&sys_f[3*conn[18]]);
          f19 = _mm256_loadu_pd(&sys_f[3*conn[19]]);
          }
        }
        {
        __m256d g0,g1,g2,g3,g4,g5,g6,g7,g8,g9,g10,g11;
          g0 = _mm256_set1_pd(G[0])  ; g1 = _mm256_set1_pd(G[1])  ; g2 = _mm256_set1_pd(G[2]);
          f0 = _mm256_add_pd(f0, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

          g3 = _mm256_set1_pd(G[4])  ; g4 = _mm256_set1_pd(G[5])  ; g5 = _mm256_set1_pd(G[6]);
          f1 = _mm256_add_pd(f1, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

          g6 = _mm256_set1_pd(G[8])  ; g7 = _mm256_set1_pd(G[9])  ; g8 = _mm256_set1_pd(G[10]);
          f2 = _mm256_add_pd(f2, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

          g9 = _mm256_set1_pd(G[12]) ; g10= _mm256_set1_pd(G[13]) ; g11= _mm256_set1_pd(G[14]);
          f3 = _mm256_add_pd(f3, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));
        if(elem_p>1){
          g0 = _mm256_set1_pd(G[16]) ; g1 = _mm256_set1_pd(G[17]) ; g2 = _mm256_set1_pd(G[18]);
          f4 = _mm256_add_pd(f4, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

          g3 = _mm256_set1_pd(G[20]) ; g4 = _mm256_set1_pd(G[21]) ; g5 = _mm256_set1_pd(G[22]);
          f5 = _mm256_add_pd(f5, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

          g6 = _mm256_set1_pd(G[24]) ; g7 = _mm256_set1_pd(G[25]) ; g8 = _mm256_set1_pd(G[26]);
          f6 = _mm256_add_pd(f6, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

          g9 = _mm256_set1_pd(G[28]) ; g10= _mm256_set1_pd(G[29]) ; g11= _mm256_set1_pd(G[30]);
          f7 = _mm256_add_pd(f7, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));

          g0 = _mm256_set1_pd(G[32]) ; g1 = _mm256_set1_pd(G[33]) ; g2 = _mm256_set1_pd(G[34]);
          f8 = _mm256_add_pd(f8, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

          g3 = _mm256_set1_pd(G[36]) ; g4 = _mm256_set1_pd(G[37]) ; g5 = _mm256_set1_pd(G[38]);
          f9 = _mm256_add_pd(f9, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));
        }
        if(elem_p>2){
          g6 = _mm256_set1_pd(G[40]) ; g7 = _mm256_set1_pd(G[41]) ; g8 = _mm256_set1_pd(G[42]);
          f10 = _mm256_add_pd(f10, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

          g9 = _mm256_set1_pd(G[44]) ; g10= _mm256_set1_pd(G[45]) ; g11= _mm256_set1_pd(G[46]);
          f11 = _mm256_add_pd(f11, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));

          g0 = _mm256_set1_pd(G[48]) ; g1 = _mm256_set1_pd(G[49]) ; g2 = _mm256_set1_pd(G[50]);
          f12 = _mm256_add_pd(f12, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

          g3 = _mm256_set1_pd(G[52]) ; g4 = _mm256_set1_pd(G[53]) ; g5 = _mm256_set1_pd(G[54]);
          f13 = _mm256_add_pd(f13, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

          g6 = _mm256_set1_pd(G[56]) ; g7 = _mm256_set1_pd(G[57]) ; g8 = _mm256_set1_pd(G[58]);
          f14 = _mm256_add_pd(f14, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

          g9 = _mm256_set1_pd(G[60]) ; g10= _mm256_set1_pd(G[61]) ; g11= _mm256_set1_pd(G[62]);
          f15 = _mm256_add_pd(f15, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));

          g0 = _mm256_set1_pd(G[64]) ; g1 = _mm256_set1_pd(G[65]) ; g2 = _mm256_set1_pd(G[66]);
          f16 = _mm256_add_pd(f16, _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));

          g3 = _mm256_set1_pd(G[68]) ; g4 = _mm256_set1_pd(G[69]) ; g5 = _mm256_set1_pd(G[70]);
          f17 = _mm256_add_pd(f17, _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));

          g6 = _mm256_set1_pd(G[72]) ; g7 = _mm256_set1_pd(G[73]) ; g8 = _mm256_set1_pd(G[74]);
          f18 = _mm256_add_pd(f18, _mm256_add_pd(_mm256_mul_pd(g6,a036), _mm256_add_pd(_mm256_mul_pd(g7,a147),_mm256_mul_pd(g8,a258))));

          g9 = _mm256_set1_pd(G[76]) ; g10= _mm256_set1_pd(G[77]) ; g11= _mm256_set1_pd(G[78]);
          f19 = _mm256_add_pd(f19, _mm256_add_pd(_mm256_mul_pd(g9,a036), _mm256_add_pd(_mm256_mul_pd(g10,a147),_mm256_mul_pd(g11,a258))));
        }
        }
      } // end variable scope
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
#ifdef VECTORIZED
    _mm256_store_pd(&f[ 0],f0);
    _mm256_store_pd(&f[ 4],f1);
    _mm256_store_pd(&f[ 8],f2);
    _mm256_store_pd(&f[12],f3);
    if(elem_p>1){
    _mm256_store_pd(&f[16],f4);
    _mm256_store_pd(&f[20],f5);
    _mm256_store_pd(&f[24],f6);
    _mm256_store_pd(&f[28],f7);
    _mm256_store_pd(&f[32],f8);
    _mm256_store_pd(&f[36],f9);
    }
    if(elem_p>2){
    _mm256_store_pd(&f[40],f10);
    _mm256_store_pd(&f[44],f11);
    _mm256_store_pd(&f[48],f12);
    _mm256_store_pd(&f[52],f13);
    _mm256_store_pd(&f[56],f14);
    _mm256_store_pd(&f[60],f15);
    _mm256_store_pd(&f[64],f16);
    _mm256_store_pd(&f[68],f17);
    _mm256_store_pd(&f[72],f18);
    _mm256_store_pd(&f[76],f19);
    }
#endif
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for(int j=0; j<3; j++){
        sys_f[3* conn[i]+j ] = f[4* i+j ]; } }
  }//============================================================ end elem loop
  }// end f register scope
  return 0;
}
#undef VECTORIZED
