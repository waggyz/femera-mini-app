#if VERB_MAX > 10
#include <iostream>
#endif
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>// std::memcpy
#ifdef __INTEL_COMPILER
#include <mkl.h>
#endif
#include "femera.h"
//
int ElastIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->elem_stiff.resize(elem_n*conn_n*conn_n*node_d*node_d);
  ElemStiff( E );
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) * 3*uint(E->elem_conn_n) *2;
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    *(3*uint(E->elem_conn_n) * 3*uint(E->elem_conn_n) +2);
#if 1
  this->tens_flop = this->stif_flop;//FIXME
  this->tens_band = this->stif_band;
#else
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
#endif
  return 0;
}
int ElastIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u ){
#define FETCH_F
  const int Dn = 3;// this->node_d DOF/node = mesh dimension
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn * Nc;
#ifdef __INTEL_COMPILER
#define FETCH_F
  const int Nk =(Ne*(Ne + 1))/2;
#else
  const int Nk = Ne * Ne;
#endif
  FLOAT_PHYS VECALIGNED u[Ne], f[Ne];
  const   INT_MESH* RESTRICT E_c =& E->elem_conn[0];
  const FLOAT_SOLV* RESTRICT E_k =& this->elem_stiff[0];
  const FLOAT_SOLV* RESTRICT S_u =& part_u[0];
        FLOAT_SOLV* RESTRICT S_f =& part_f[0];
  for(INT_MESH ie=e0; ie<ee; ie++){
#ifdef __INTEL_COMPILER
    const FLOAT_SOLV* RESTRICT k =& E_k[Nk*ie];
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& S_u[E_c[Nc*ie+i]*Dn], Dn*sizeof(FLOAT_SOLV) );
#ifdef FETCH_F
      std::memcpy( & f[Dn*i],& S_f[E_c[Nc*ie+i]*Dn], Dn*sizeof(FLOAT_SOLV) );
#endif
    }
#ifdef FETCH_F
#ifdef __INTEL_COMPILER
    const FLOAT_SOLV* RESTRICT cu =& S_u[0];
    cblas_dspmv (CblasRowMajor, CblasUpper, Ne,
      1.0,& k,& cu, 1, 1.0,& f, 1);
#else
    // Generic C matrix-vector multiply
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(int i=0; i<Ne; i++){
      for(int j=0; j<Ne; j++){
        f[ i ] += E_k[Nk*ie+ Ne* i+j ] * u[ j ];
    } }
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy(& S_f[Dn*E_c[Nc*ie+i]],& f[Dn*i], Dn*sizeof(FLOAT_SOLV) );
    }
#else
// Do not fetch f first.
#if 1
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(int i=0; i<Ne; i++){ f[i]=0.0;
      for(int j=0; j<Ne; j++){
        f[ i ] += E_k[Nk*ie+ Ne* i+j ] * u[ j ];
    } }
#else
    switch(Ne){
      case(12):{
        for(int i=0; i<12; i++){ f[i]=0.0;
          for(int j=0; j<12; j++){
            f[ i ] += E_k[Nk*ie+ 12* i+j ] * u[ j ];
        } }
        break;}
      case(30):{
        for(int i=0; i<30; i++){ f[i]=0.0;
          for(int j=0; j<30; j++){
            f[ i ] += E_k[Nk*ie+ 30* i+j ] * u[ j ];
        } }
        break;}
      case(60):{
        for(int i=0; i<60; i++){ f[i]=0.0;
          for(int j=0; j<60; j++){
            f[ i ] += E_k[Nk*ie+ 60* i+j ] * u[ j ];
        } }
        break;}
    }
#endif
    for (int i=0; i<Nc; i++){
      for(int j=0; j<Dn; j++){
        S_f[Dn*E_c[Nc*ie+ i]+j ] += f[Dn* i+j ];
    } }
#endif
  }//end elem loop
#undef FETCH_F
  return 0;
}