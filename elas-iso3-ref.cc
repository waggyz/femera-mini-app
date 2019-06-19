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
int ElastIso3D::ElemLinear( Elem* E,
  FLOAT_SOLV* sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n;};
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_PHYS dw, G[Ne], u[Ne], f[Ne];
  FLOAT_PHYS H[Nd*Nf], S[Nd*Nf];
  //
  const FLOAT_PHYS* RESTRICT intp_shpg = &E->intp_shpg[0];
  const FLOAT_PHYS* RESTRICT       wgt = &E->gaus_weig[0];
  const FLOAT_PHYS* RESTRICT         C = &this->mtrl_matc[0];
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
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  for(INT_MESH ie=e0;ie<ee;ie++){
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Nf); j++){
        u[Nf*i+j] = sysu[conn[i]*Nf+j];
    } }
    for(int i=0; i<Ne; i++){ f[i]=0.0; }
    for(int ip=0; ip<intp_n; ip++){
      for(int i=0; i< 9 ; i++){ H[i]=0.0; }
      for(int i=0; i<Ne ; i++){ G[i]=0.0; }
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf ; k++){ 
          for(int j=0; j<Nd ; j++){
            G[Nf* i+k ] += intp_shpg[ip*Ne+ Nd* i+j ] * Ejacs[Nj*ie+ Nd* j+k ];
      } } }
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf ; k++){
          for(int j=0; j<Nf ; j++){
            H[Nf* k+j ] += G[Nf* i+k ] * u[Nf* i+j ];
      } } }//---------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%mesh_d==0){printf("\n"); }
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      dw = Ejacs[Nj*ie+ 9] * wgt[ip];
      //
      S[0]=(C[0]* H[0] + C[1]* H[4] + C[1]* H[8])*dw;//Sxx
      S[4]=(C[1]* H[0] + C[0]* H[4] + C[1]* H[8])*dw;//Syy
      S[8]=(C[1]* H[0] + C[1]* H[4] + C[0]* H[8])*dw;//Szz
      //
      S[1]=( H[1] + H[3] )*C[2]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[2]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[2]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      //-------------------------------------------------------- 18+9 = 27 FLOP
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf; k++){
          for(int j=0; j<Nf; j++){
            f[Nf* i+k ] += G[Nf* i+j ] * S[Nf* j+k ];
      } } }//------------------------------------------------- N*3*6 = 18*N FMA
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n"); }
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }//end intp loop
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Nf); j++){
        sysf[conn[i]*Nf+j] += f[Nf*i+j];
    } }//------------------------------------------------------------- 3*n FLOP
  }//end elem loop
  return 0;
  }