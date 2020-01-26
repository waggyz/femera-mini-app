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
  const int Dn = 3;// this->node_d DOF/node
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn*Nc, Nk=Ne*Ne;
  FLOAT_PHYS u[Ne];
#ifdef FETCH_F
  FLOAT_PHYS f[Ne];
#endif
  const   INT_MESH* RESTRICT E_c = &E->elem_conn[0];
  const FLOAT_SOLV* RESTRICT E_k    = &this->elem_stiff[0];
  const FLOAT_SOLV* RESTRICT S_u  = &part_u[0];
        FLOAT_SOLV* RESTRICT S_f  = &part_f[0];
  for(INT_MESH ie=e0;ie<ee;ie++){
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& S_u[E_c[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
#ifdef FETCH_F
      std::memcpy( & f[Dn*i],& S_f[E_c[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
#endif
    }
#ifdef FETCH_F
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(int i=0; i<Ne; i++){
      for(int j=0; j<Ne; j++){
        f[ i ] += E_k[Nk*ie+ Ne* i+j ] * u[ j ];
    } }
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& S_f[Dn*E_c[Nc*ie+i]],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
    }
#else
    for(int i=0; i<Ne; i++){
      for(int j=0; j<Nc; j++){
        for(int k=0; k<Dn; k++){
          S_f[Dn*E_c[Nc*ie+j]+k ] += E_k[Nk*ie+ Ne* i+Dn* j+k ] * u[Dn* j+ k ];
    } } }
#endif
  }//end elem loop
#undef FETCH_F
  return 0;
}