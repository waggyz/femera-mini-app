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
int ThermElastIso3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (42+6+24) +1 +3+21+3+6 +12 );
  this->tens_band = uint(E->elem_n) *(
     sizeof(FLOAT_PHYS)*(4*uint(E->elem_conn_n)*3+ jacs_n*10)
    +sizeof(INT_MESH)*uint(E->elem_conn_n) );
  this->stif_flop = uint(E->elem_n)
    * 4*uint(E->elem_conn_n) *( 4*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) *(
    sizeof(FLOAT_PHYS)* 4*uint(E->elem_conn_n) *( 4*uint(E->elem_conn_n) -1+2)
    +sizeof(INT_MESH) *uint(E->elem_conn_n) );
  return 0;
};
int ThermElastIso3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV *part_f, const FLOAT_SOLV* part_u ){
  //FIXME Cleanup local variables.
  const int Dm = 3;//E->mesh_d;// Node (mesh) Dimension FIXME should be elem_d?
  const int Dn = 4;//this->node_d;// this->node_d DOF/node
  const int Nj = 10;//Dm*Dm+1;// Jac inv & det
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ng = Dm*Nc;
  const int Ne = Dn*Nc;
  const int intp_n = int(E->gaus_n);
#if VERB_MAX>11
  printf("Dim: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
    (int)mesh_d,(int)elem_n,(int)intp_n,(int)Nc);
#endif
  //INT_MESH   conn[Nc];
  //FLOAT_MESH jac[Nj];
  FLOAT_PHYS VECALIGNED G[Ng], u[Ne],f[Ne];
  FLOAT_PHYS VECALIGNED S[Dm*Dn], H[Dm*Dn];
  //
  // Make local copies of constant data structures
  FLOAT_PHYS VECALIGNED intp_shpf[intp_n*Nc];
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ng];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpf[0], &E->intp_shpf[intp_n*Nc], intp_shpf );
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ng], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
#if 0
  FLOAT_PHYS gamma[3];// gamma = alpha * E/(1-2*nu), thermoelastic effect
  for(int i=0; i<Dm; i++){ gamma[i] = 1.0/(C[i] * C[9+i]); }//FIXME may be 1.0/this
#endif
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &part_u[0];
        FLOAT_SOLV* RESTRICT sysf  = &part_f[0];
  for(INT_MESH ie=e0;ie<ee;ie++){//============================================
    const FLOAT_MESH* RESTRICT jac = &Ejacs[Nj*ie];
#if 0
    //std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc);
    //std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
    //std::copy( &Econn[Nc*ie],
    //           &Econn[Nc*ie+Nc], conn );
    //std::copy( &Ejacs[Nj*ie],
    //           &Ejacs[Nj*ie+Nj], jac );// det=jac[9];
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( &u[Dn*i], &sysu[Econn[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
      std::memcpy( &f[Dn*i], &sysf[Econn[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
    }
    for(int ip=0; ip<intp_n; ip++){//==========================================
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// Small deformation tensor
      for(int i=0; i<(Dm*Dn) ; i++){ H[i]=0.0; }
      for(int k=0; k<Nc; k++){
        for(int i=0; i<Dm ; i++){ G[Dm* k+i ]=0.0;
          for(int j=0; j<Dm ; j++){
            G[Dm* k+i ] += jac[Dm* j+i ] * intp_shpg[Ng*ip + Dm* k+j ];
          }
          for(int j=0; j<Dn ; j++){
            H[Dn* i+j ] += G[Dm* k+i ] * u[Dn* k+j ];
          }// The last COLUMMN H[3,7,11] has dT/dx
        }
      }//---------------------------------------------- N*3*(6+8) = 42*Nc FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<HH.size();j++){
        if(j%mesh_d==0){ printf("\n"); }
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      const FLOAT_PHYS dw = jac[9] * wgt[ip];//------------------------ 1 FLOP
      FLOAT_PHYS Tip=0.0;// Zero the temperature at this integration point
      for(int i=0; i<Nc; i++){// Interpolate temperature
        Tip+= intp_shpf[Nc*ip +i] * u[Dn* i+Dm ];//----------------- 6*Nc FLOP
      }
      // Apply thermal expansion to the volumetric (diagonal) strains
      H[0]-=Tip*C[3]; H[4]-=Tip*C[3]; H[8]-=Tip*C[3];//------------- 3 FLOP
      //
      S[0] = C[0]* H[0] + C[1]* H[4] + C[1]* H[8];//Sxx
      S[4] = C[1]* H[0] + C[0]* H[4] + C[1]* H[8];//Syy
      S[8] = C[1]* H[0] + C[1]* H[4] + C[0]* H[8];//Szz
      //
      S[1] =(H[1] + H[3])*C[2];// S[3]= S[1];//Sxy Syx
      S[5] =(H[5] + H[7])*C[2];// S[7]= S[5];//Syz Szy
      S[2] =(H[2] + H[6])*C[2];// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];//------------------------------ 21 FLOP
      // Apply thermal conductivity, storing heat flux in the last ROW of S
      S[ 9] = H[Dn* 0+Dm] * C[4];
      S[10] = H[Dn* 1+Dm] * C[4];
      S[11] = H[Dn* 2+Dm] * C[4];//------------------------------------ 3 FLOP
#if 1
      // Calculate volumetric thermoelastic effect temperature change
      // Small and neglected for quasi-static (high-cycle?) fatigue loading
      S[ 9]-= S[0] * C[5];
      S[10]-= S[4] * C[5];
      S[11]-= S[8] * C[5];//------------------------------------------- 6 FLOP
#endif
#if VERB_MAX>10
      printf( "Stress (Natural Coords):");
      for(int j=0;j<9;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e ",S[j]);
      } printf("\n");
#endif
      // Apply integration weights and |jac|.
      for(int i=0; i<(Dm*Dn); i++){ S[i] *= dw; }//------------------- 12 FLOP
      // Accumulate nodal forces
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Dn; k++){
          for(int j=0; j<Dm; j++){
            f[Dn* i+k ] += G[Dm* i+j ] * S[Dm* k+j ];
      } } }//----------------------------------------------- N*3*8 = 24*N FLOP
#if VERB_MAX>10
      printf( "ff:");
      for(int j=0;j<Ne;j++){
        if(j%mesh_d==0){ printf("\n"); }
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }// end intp loop =========================================================
    for (int i=0; i<Nc; i++){// Write output back to system vector
      std::memcpy(& sysf[Econn[Nc*ie+i]*Dn],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
    }
  }//end elem loop ============================================================
  return 0;
}