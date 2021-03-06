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
int ElastDmv3D::Setup( Elem* E ){
  JacT  ( E );
  IniRot();
#if 0
  MtrRot();// Assume these are already rotated in the input files.
#endif
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
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
  return 0;
}
int ElastDmv3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
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
  FLOAT_PHYS VECALIGNED G[Ne], u[Ne], f[Ne];
  FLOAT_PHYS VECALIGNED H[Dm*Dn], S[Dm*Dn];
  //
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],
             &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  std::copy( &E->gaus_weig[0],
             &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS VECALIGNED dmat[this->mtrl_dmat.size()];
  std::copy( &this->mtrl_dmat[0],
             &this->mtrl_dmat[this->mtrl_dmat.size()], dmat );
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_dmat.size() );
  for(uint j=0;j<mtrl_dmat.size();j++){
    if(j%6==0){printf("\n");}
    printf("%+9.2e ",D[j]);
  } printf("\n");
#endif
  const  FLOAT_PHYS* RESTRICT VECALIGNED D = &dmat[0];
  const    INT_MESH* RESTRICT E_c = &E->elem_conn[0];
  const  FLOAT_MESH* RESTRICT E_j = &E->elip_jacs[0];
  const  FLOAT_SOLV* RESTRICT S_u = &part_u[0];
         FLOAT_SOLV* RESTRICT S_f = &part_f[0];
  if(e0<ee){
#if 0
#ifdef FETCH_JAC
    std::memcpy( &jac , &E_j[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
#endif
#if 1
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& S_u[E_c[Nc*e0+i]*Dn], sizeof(FLOAT_SOLV)*Dn ); }
#endif
  }
  for(INT_MESH ie=e0;ie<ee;ie++){
#if 0
    const INT_MESH* RESTRICT conn = &E_c[Nc*ie];
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & f[Dn*i],& S_f[E_c[Nc*ie+i]*3], sizeof(FLOAT_SOLV)*Dn ); }
    for(int ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      for(int i=0; i<(Dm*Dm) ; i++){ H[i]=0.0; }
#if 0
// Outer loop SIMD degrades performance using SSE, AVX, etc.
#pragma omp simd
#endif
      for(int k=0; k<Nc; k++){
        for(int i=0; i<Dm ; i++){ G[Dm* k+i ]=0.0;
          for(int j=0; j<Dm ; j++){
#if 0
            G[Dm* k+i ] += jac[Dm* j+i ] * intp_shpg[ip*Ne+ Dm* k+j ];
#else
            G[Dm* k+i ] += E_j[Nj*ie+ Dm* j+i ] * intp_shpg[ip*Ne+ Dm* k+j ];
#endif
          }
#if 0
// Use this to match intrinsics loop, but it's slower on westmere (gcc)
        } for(int i=0; i<Dm ; i++){
#endif
          for(int j=0; j<Dm ; j++){
            H[Dm* i+j ] += G[Dm* k+i ] * u[Dn* k+j ];
          }
        }
      }//-------------------------------------------------- N*3*6*2 = 36*N FLOP
      if(ip==(intp_n-1)){
        if((ie+1)<ee){// Fetch stuff for the next iteration
          const INT_MESH* RESTRICT c = &E_c[Nc*(ie+1)];
          for (int i=0; i<Nc; i++){
            //std::memcpy( &jac, &E_j[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
            std::memcpy(&u[Dn*i],&S_u[c[i]*Dn], sizeof(FLOAT_SOLV)*Dn);
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
      const FLOAT_SOLV* VECALIGNED cH =& H[0];
      const FLOAT_PHYS dw = E_j[Nj*ie+ 9 ] * wgt[ip];
      compute_dmv_s( S, cH,D,dw );
      }
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
      } } }//------------------------------------------------ N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }//end intp loop
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& S_f[E_c[Nc*ie+i]*Dn],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
#if 0
      if( n >=my_node_start ){
        for(uint j=0;j<3;j++){
          this->part_sum1+= f[Dn* i+j ] * S_u[Dn* n+j ];
          //FIXME u already contains next elem part_u
          //this->part_sum1+= f[Dn* i+j ] * u[Dn* i+j];
        };
      };
#endif
    }
  }//end elem loop
  return 0;
  }