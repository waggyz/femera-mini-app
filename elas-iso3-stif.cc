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
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->elem_stiff.resize(elem_n*conn_n*conn_n*node_d*node_d);
  ElemStiff( E );
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) ) *2;
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  return 0;
}
int ElastIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u ){
  const int Dn = 3;// this->node_d DOF/node
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn*Nc, Nk=Ne*Ne;
  FLOAT_PHYS u[Ne], f[Ne];
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_SOLV* RESTRICT Ek    = &this->elem_stiff[0];
  const FLOAT_SOLV* RESTRICT sysu  = &part_u[0];
        FLOAT_SOLV* RESTRICT sysf  = &part_f[0];
  for(INT_MESH ie=e0;ie<ee;ie++){
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Dn*i],& sysu[Econn[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
      std::memcpy( & f[Dn*i],& sysf[Econn[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
    }
#ifdef HAS_PRAGMA_SIMD
#pragma omp simd
#endif
    for(int i=0; i<Ne; i++){
      for(int j=0; j<Ne; j++){
        f[ i ] += Ek[Nk*ie+ Ne* i+j ] * u[ j ];
    } }
    for (uint i=0; i<uint(Nc); i++){
      std::memcpy(& sysf[Econn[Nc*ie+i]*Dn],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
    }
  }//end elem loop
  return 0;
}