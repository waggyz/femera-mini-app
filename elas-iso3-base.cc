#if VERB_MAX > 10
#include <iostream>
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>// std::memcpy
#include "femera.h"
//#define FETCH_JAC
//
int ElastIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
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
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Dm = 3;// Mesh Dimension
  const int Dn = 3;// this->node_d DOF/node
  const int Nj = Dm*Dm+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn*Nc;
  const int intp_n = int(E->gaus_n);
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
#if 0
#ifdef FETCH_JAC
  FLOAT_PHYS jac[Nj];
#endif
#endif
  FLOAT_PHYS G[Ne], u[Ne], f[Ne];
  FLOAT_PHYS H[Dm*Dn], S[Dm*Dn];
  //
  FLOAT_PHYS intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],
             &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS wgt[intp_n];
  std::copy( &E->gaus_weig[0],
             &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &part_u[0];
        FLOAT_SOLV* RESTRICT sysf  = &part_f[0];
  if(e0<ee){
#if 0
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
#endif
#if 1
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& sysu[Econn[Nc*e0+i]*Dn], sizeof(FLOAT_SOLV)*Dn ); }
#endif
  }
  for(INT_MESH ie=e0;ie<ee;ie++){
#if 0
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[Dn*i],& sysf[Econn[Nc*ie+i]*3], sizeof(FLOAT_SOLV)*Dn ); }
    for(int ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      for(int i=0; i<(Dm*Dm) ; i++){ H[i]=0.0; }
#if 0
// Degrades performance using SSE, AVX, etc.
#pragma omp simd
#endif
      for(int k=0; k<Nc; k++){
        for(int i=0; i<Dm ; i++){ G[Dm* k+i ]=0.0;
          for(int j=0; j<Dm ; j++){
#if 0
            G[Dm* k+i ] += jac[Dm* j+i ] * intp_shpg[ip*Ne+ Dm* k+j ];
#else
            G[Dm* k+i ] += Ejacs[Nj*ie+ Dm* j+i ] * intp_shpg[ip*Ne+ Dm* k+j ];
#endif
          } } for(int i=0; i<Dm ; i++){//FIXME Testing different merge for SSE
          for(int j=0; j<Dm ; j++){
            H[Dm* i+j ] += G[Dm* k+i ] * u[Dn* k+j ];
          }
        }
      }//------------------------------------------------- N*3*6*2 = 36*N FLOP
      if(ip==(intp_n-1)){
        if((ie+1)<ee){// Fetch stuff for the next iteration
          const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
          for (int i=0; i<Nc; i++){
            //std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
            std::memcpy(&u[Dn*i],&sysu[c[i]*Dn], sizeof(FLOAT_SOLV)*Dn);
          }
        }// Done fetching next iter stuff
      }
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      {
      const FLOAT_PHYS dw = Ejacs[Nj*ie+ 9 ] * wgt[ip];
      const FLOAT_PHYS Cdw[3] = { C[0]*dw, C[1]*dw, C[2]*dw };
      //
      S[0]= Cdw[0]* H[0] + Cdw[1]* H[4] + Cdw[1]* H[8];//Sxx
      S[4]= Cdw[1]* H[0] + Cdw[0]* H[4] + Cdw[1]* H[8];//Syy
      S[8]= Cdw[1]* H[0] + Cdw[1]* H[4] + Cdw[0]* H[8];//Szz
      //
      S[1]=( H[1] + H[3] )*Cdw[2];// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*Cdw[2];// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*Cdw[2];// S[6]= S[2];//Sxz Szx
      }//------------------------------------------------------- 18+9 = 27 FLOP
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
#if 0
#ifdef HAS_PRAGMA_SIMD
#ifndef HAS_AVX
      // May be slower for >=AVX, not much faster 40% efficient) otherwise...
#pragma omp simd
#endif
#endif
#endif
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Dn; k++){
          for(int j=0; j<Dn; j++){
            f[Dn* i+k ] += G[Dm* i+j ] * S[Dn* k+j ];// 18*N FMA FLOP
      } } }//----------------------------------------------- N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }//end intp loop
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& sysf[Econn[Nc*ie+i]*Dn],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
#if 0
      if( n >=my_node_start ){
        for(uint j=0;j<3;j++){
          this->part_sum1+= f[Dn* i+j ] * sysu[Dn* n+j ];
          //FIXME u already contains next elem part_u
          //this->part_sum1+= f[Dn* i+j ] * u[Dn* i+j];
        };
      };
#endif
    }
  }//end elem loop
  return 0;
  }