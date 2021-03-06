#if VERB_MAX > 10
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#endif
#include <ctype.h>
#include "femera.h"
//
int ElastOrtho3D::Setup( Elem* E ){
  IniRot();
  JacRot( E );
  JacT  ( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (36+18)+uint(E->elem_conn_n)*3 + 2*54 + 27 );
  this->tens_band = uint(E->elem_n) *(
     sizeof(FLOAT_PHYS)*(3*uint(E->elem_conn_n)*3+ jacs_n*10)
    +sizeof(INT_MESH)*uint(E->elem_conn_n) );
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) *(
    sizeof(FLOAT_PHYS)* 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) -1+2)
    +sizeof(INT_MESH) *uint(E->elem_conn_n) );
  return 0;
}
int ElastOrtho3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV *part_f, const FLOAT_SOLV* part_u ){
  //FIXME Cleanup local variables.
  const int Dm = E->mesh_d;// Node (mesh) Dimension FIXME should be elem_d?
  const int Dn = this->node_d;// this->node_d DOF/node
  const int Nj = Dm*Dm+1;// Jac inv & det
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn*Nc;
  FLOAT_PHYS dw, G[Ne], u[Ne],f[Ne];
  FLOAT_PHYS H[9], S[9], A[9];
  for(INT_MESH ie=e0;ie<ee;ie++){
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Dn); j++){
        u[Dn*i+j] = part_u[E->elem_conn[Nc*ie+i]*Dn+j];
      } }
    for (int i=0; i<Ne; i++){ f[i]=0.0; }
    for(int ip=0; ip<E->gaus_n; ip++){
      for(int i=0; i< 9 ; i++){ H[i]=0.0; A[i]=0.0; }
      for(int i=0; i<Ne ; i++){ G[i]=0.0; }
      //G = MatMul3x3xN( jac,shg );
      //A = MatMul3xNx3T( G,u );
      for(int k=0; k<Nc; k++){
        for(int i=0; i<Dm ; i++){
          for(int j=0; j<Dm ; j++){
            G[(Dm* k+i) ] += E->elip_jacs[Nj*ie+ Dm* j+i ] * E->intp_shpg[ip*Ne+ Dm* k+j ];
          } } }
      for(int k=0; k<Nc; k++){
        for(int i=0; i<Dm ; i++){
          for(int j=0; j<Dm ; j++){
            A[Dm* i+j ] += G[Dm* k+i ] * u[Dn* k+j ];
          } } }//------------------------------------------ N*3*6*2 = 36*N FLOP
      dw = E->elip_jacs[Nj*ie+ 9 ] * E->gaus_weig[ip];
      // [H] Small deformation tensor
      // [H][RT] : matmul3x3x3T
      for(int i=0; i<3; i++){
        for(int k=0; k<3; k++){ H[3* i+k ]=0.0;
          for(int j=0; j<3; j++){
            H[(3* i+k) ] += A[(3* i+j)] * this->mtrl_rotc[3* k+j ];
      } } }//---------------------------------------------- 27*2 =      54 FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<9;j++){
        if(j%mesh_d==0){printf("\n"); }
        printf("%+9.2e ",H[j]);
      }; printf("\n");
#endif
      S[0]=(this->mtrl_matc[0]* H[0] + this->mtrl_matc[3]* H[4] + this->mtrl_matc[5]* H[8])*dw;//Sxx
      S[4]=(this->mtrl_matc[3]* H[0] + this->mtrl_matc[1]* H[4] + this->mtrl_matc[4]* H[8])*dw;//Syy
      S[8]=(this->mtrl_matc[5]* H[0] + this->mtrl_matc[4]* H[4] + this->mtrl_matc[2]* H[8])*dw;//Szz
      //
      S[1]=( H[1] + H[3] )*this->mtrl_matc[6]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*this->mtrl_matc[7]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*this->mtrl_matc[8]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
#if VERB_MAX>10
      printf( "Stress (Natural Coords):");
      for(int j=0;j<9;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e ",S[j]);
      }; printf("\n");
#endif
      //--------------------------------------------------------- 18+9= 27 FLOP
      // [S][R] : matmul3x3x3, R is transposed
      for(int i=0; i<9; i++){ A[i]=0.0; };
      for(int i=0; i<3; i++){
        for(int k=0; k<3; k++){
          for(int j=0; j<3; j++){// A is transposed
            A[(3* k+i) ] += S[3* i+j ] * this->mtrl_rotc[3* j+k ];
      };};};//-------------------------------------------------- 27*2 = 54 FLOP
      //NOTE [A] is not symmetric Cauchy stress.
      //NOTE Cauchy stress is ( A + AT ) /2
#if VERB_MAX>10
      printf( "Rotated Stress (Global Coords):");
      for(int j=0;j<9;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e ",S[j]);
      }; printf("\n");
#endif
//#pragma omp simd
      for(int i=0; i<Nc; i++){
        for(int k=0; k<3; k++){
          for(int j=0; j<3; j++){
            f[(3* i+k) ] += G[(3* i+j) ] * A[(3* k+j) ];
      } } }//------------------------------------------------ N*3*6 = 18*N FLOP
    }//end intp loop
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Dn); j++){
        part_f[E->elem_conn[Nc*ie+i]*Dn+j] += f[Dn*i+j]; } }//--------- 3*N FLOP
  }//end elem loop
  return 0;
}