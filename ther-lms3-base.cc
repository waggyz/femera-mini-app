#if VERB_MAX > 10
#include <iostream>
#endif
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>// std::memcpy
#include "femera.h"
#ifdef __INTEL_COMPILER
#include <mkl.h>
#endif
//
int ThermIso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint Nr = uint(E->elem_conn_n);
#ifdef __INTEL_COMPILER
  const uint Nk =(Nr*(Nr + 1))/2;
#else
  const uint Nk = Nr * Nr;
#endif
  this->elem_stiff.resize(elem_n*Nk);
  this->ElemStiff( E );
  this->stif_flop = uint(E->elem_n) * Nr*Nr *2;
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS) * (Nk + Nr*2);
#if 1
  this->tens_flop = this->stif_flop;//FIXME
  this->tens_band = this->stif_band;
#else
  const uint conn_n = uint(E->elem_conn_n);//size()/elem_n/ 10) ;
  const uint jacs_n = uint(E->elip_jacs.
  const uint intp_n = uint(E->gaus_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(intp_n*conn_n +3+1 ) );// Stack (assumes read once)
#endif
  return 0;
}
int ThermIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u ){
#define FETCH_F
  const int Dn = 1;// this->node_d DOF/node
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn * Nc;
#ifdef __INTEL_COMPILER
#define FETCH_F
  const int Nk =(Ne*(Ne + 1))/2;
#else
  const int Nk = Ne*Ne;
#endif
  FLOAT_PHYS VECALIGNED u[Ne], f[Ne];
  const   INT_MESH* RESTRICT E_c =& E->elem_conn[0];
  const FLOAT_SOLV* RESTRICT E_k =& this->elem_stiff[0];
  const FLOAT_SOLV* RESTRICT S_u =& part_u[0];
        FLOAT_SOLV* RESTRICT S_f =& part_f[0];
  for(INT_MESH ie=e0; ie<ee; ie++){
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& S_u[E_c[Nc*ie+i]*Dn], Dn*sizeof(FLOAT_SOLV) );
#ifdef FETCH_F
      std::memcpy( & f[Dn*i],& S_f[E_c[Nc*ie+i]*Dn], Dn*sizeof(FLOAT_SOLV) );
#endif
    }
#ifdef FETCH_F
#ifdef __INTEL_COMPILER
    const FLOAT_SOLV* ck =& E_k[Nk*ie];
    const FLOAT_SOLV* VECALIGNED cu =& u[0];
#if 1
    cblas_dspmv (CblasRowMajor, CblasUpper, Ne,
      1.0,& ck[0],& cu[0], 1, 1.0,& f[0], 1);
#else
    switch(Ne){// This seems slightly faster?
      case(12):{
        cblas_dspmv (CblasRowMajor, CblasUpper, 12,
          1.0,& ck[0],& cu[0], 1, 1.0,& f[0], 1);
        break;}
      case(30):{
        cblas_dspmv (CblasRowMajor, CblasUpper, 30,
          1.0,& ck[0],& cu[0], 1, 1.0,& f[0], 1);
        break;}
      case(60):{
        cblas_dspmv (CblasRowMajor, CblasUpper, 60,
          1.0,& ck[0],& cu[0], 1, 1.0,& f[0], 1);
        break;}
    }
#endif
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