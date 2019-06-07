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
int ThermElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = elem_n *( conn_n*(2*18)// FIXME wrong calcs
    +intp_n*( conn_n*(36+18) + 27 ) );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +9+9+1 ) );// Stack (assumes read once)
  this->stif_flop = elem_n
    * 3*conn_n *( 3*conn_n );
  this->stif_band = elem_n *(
    sizeof(FLOAT_SOLV)* 3*conn_n *( 3*conn_n -1+3)
+sizeof(INT_MESH) *conn_n );
  return 0;
};
int ThermElastOrtho3D::ElemLinear( Elem* E,
  FLOAT_SOLV *sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Cleanup local variables.
  const int Dm = 3;//E->mesh_d;// Node (mesh) Dimension FIXME should be elem_d?
  const int Dn = 4;//this->node_d;// this->node_d DOF/node
  const int Nj = 10;//Dm*Dm+1;// Jac inv & det
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ng = Dm*Nc;
  const int Ne = Dn*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n; };
  //
#if VERB_MAX>11
  printf("Meshd: %i, Noded: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
    (int)Dm,(int)Dn,(int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_MESH jac[Nj];//, det;
  FLOAT_PHYS u[Ne], f[Ne], uR[Ne];
  FLOAT_PHYS G[Ng], GS[Ng], H[Dm*Dn], S[Dm*Dn];//FIXME wrong sizes?
  //FIXME
  //const bool has_ther = ( this->ther_expa.size() > 0 );
  //
  // Make local copies of constant data structures
  FLOAT_PHYS intp_shpf[intp_n*Nc];
  //if(has_therm){
  std::copy( &E->intp_shpf[0],
             &E->intp_shpf[intp_n*Nc], intp_shpf );
  //}
  FLOAT_PHYS intp_shpg[intp_n*Ng];
  std::copy( &E->intp_shpg[0],
             &E->intp_shpg[intp_n*Ng], intp_shpg );
  FLOAT_PHYS wgt[intp_n];
  std::copy( &E->gaus_weig[0],
             &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
  FLOAT_PHYS gamma[3];// gamma = alpha * E/(1-2*nu), thermoelastic effect
  for(int i=0; i<Dm; i++){ gamma[i] = 1.0/(C[i] * C[9+i]); }//FIXME may be 1.0/this
  FLOAT_PHYS R[9] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    if(j%Dm==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  if(e0<ee){// Fetch first element data
    for (int i=0; i<Nc; i++){
      std::memcpy( &   u[Dn*i], &sysu[Econn[Nc*e0+i]*Dn],
        sizeof(FLOAT_SOLV)*Dn );
    }
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
  }// done fetching first element
#if VERB_MAX>10
    printf( "Displacements:");// (Elem: %u):", ie );
  for(int j=0;j<Ne;j++){
      if(j%Dn==0){printf("\n");}
      printf("%+9.2e ",u[j]);
    } printf("\n");
#endif
  for(INT_MESH ie=e0;ie<ee;ie++){//=============================== Element Loop
    for(int i=0;i<Ng;i++){ GS[i]=0.0; };
    // Transpose R
    std::swap(R[1],R[3]); std::swap(R[2],R[6]); std::swap(R[5],R[7]);
    for(int i=0; i<Nc; i++){// Rotate vectors in u
      for(int k=0; k<Dm; k++){ uR[(Dn* i+k) ]=0.0;
        for(int j=0; j<Dm; j++){
          uR[(Dn* i+k) ] += u[(Dn* i+j) ] * R[(Dm* j+k) ];
    } } }//--------------------------------------------- 2 *3*3*Nc = 18*Nc FLOP
    //if(has_ther){// Copy thermal vals from u to ur.
      for(int i=0; i<Nc; i++){ uR[Dn* i+Dm ]=u[Dn* i+Dm ]; }
    //}//FIXME Should just store in uR initially?
    for (int i=0; i<Nc; i++){// Fetch the current output values
      std::memcpy(& f[Dn*i],& sysf[Econn[Nc*ie+i]*Dn], sizeof(FLOAT_SOLV)*Dn );
    }
    if((ie+1)<ee){// Fetch stuff for the next iteration
      for (int i=0; i<Nc; i++){
        std::memcpy(&u[Dn*i],&sysu[Econn[Nc*(ie+1)+i]*Dn],sizeof(FLOAT_SOLV)*Dn);
      }
    }// Done fetching next iter stuff
    for(int ip=0; ip<intp_n; ip++){//=================== Integration Point Loop
      //G = MatMul3x3xN( jac,shg );
      for(int i=0; i<(Dm*Dn); i++){ H[i]=0.0; };
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Dm ; k++){ G[Dm* i+k ]=0.0;
          for(int j=0; j<Dm ; j++){
            G[Dm* i+k ] += intp_shpg[ip*Ng+ Dm* i+j ] * jac[Dm* j+k ];
          }
          for(int j=0; j<Dn ; j++){// Unsymmetric small strain tensor
            H[Dm* j+k ] += G[Dm* i+k ] * uR[Dn* i+j ];
          }
        }
      }//-------------------------------------------- 4 *3*3*Nc = 36*Nc*Ng FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<(Dm*Dn);j++){
        if(j%Dm==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      const FLOAT_PHYS dw = jac[Dm*Dm] * wgt[ip];
      if(ip==(intp_n-1)){
        if((ie+1)<ee){// Fetch stuff for the next iteration
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
      } }
      //if(has_ther){
        FLOAT_PHYS Tip=0.0;// Zero the temperature at this integration point
        for(int i=0; i<Nc; i++){// Interpolate temperature at this int. pt.
          Tip += intp_shpf[Nc*ip +i] * uR[Dn* i+Dm ];
        }
        // Apply thermal expansion to the volumetric (diagonal) strains
        H[ 0]-=Tip*C[ 9]; H[ 4]-=Tip*C[10]; H[ 8]-=Tip*C[11];
      //}
      // Elastic material response
      S[0]=(C[0]* H[0] + C[3]* H[4] + C[5]* H[8]);//Sxx
      S[4]=(C[3]* H[0] + C[1]* H[4] + C[4]* H[8]);//Syy
      S[8]=(C[5]* H[0] + C[4]* H[4] + C[2]* H[8]);//Szz
      //
      S[1]=( H[1] + H[3] )*C[6];// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[7];// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[8];// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      //if(has_ther){
#if 1
        // Apply thermal conductivities
        // Store heat flux in the last row of S
        S[ 9]=H[ 9]*C[12];
        S[10]=H[10]*C[13];
        S[11]=H[11]*C[14];
#endif
#if 1
        // Calculate volumetric thermoelastic effect temperature change
        // Small and neglected for quasi-static (high-cycle?) fatigue loading
        S[ 9]-= gamma[0]*S[0];
        S[10]-= gamma[1]*S[4];
        S[11]-= gamma[2]*S[8];
#endif
#if VERB_MAX>10
        printf("TEMPERATURE[%i]: %+9.2e\n",ip,Tip);
#endif
#if 0
        // Calculate plastic strain heating
        //FIXME This can be negative??
        dT=(C[18]* H[0] + C[21]* H[1] + C[23]* H[2]
          + C[21]* H[3] + C[19]* H[4] + C[22]* H[5]
          + C[23]* H[6] + C[22]* H[7] + C[20]* H[8]);
#endif
      //}// end thermal stuff
      //------------------------------------------------------ 18+9= 27*Ng FLOP
#if VERB_MAX>10
      printf( "Stress:");
      for(int j=0;j<(Dn*Dm);j++){
        if(j%Dm==0){ printf("\n"); }
        printf("%+9.2e ",S[j]);
      } printf("\n");
#endif
      // Apply integration weights and |jac|.
      for(int i=0; i<(Dm*Dn); i++){ S[i] *= dw; }
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Dm; k++){//FIXME Dm?
          for(int j=0; j<Dm; j++){
            GS[Dm* i+k ] += G[Dm* i+j ] * S[Dm* j+k ];
      } } }//---------------------------------------- 2 *3*3*Nc = 18*Nc*Ng FLOP
      //if(has_ther){// Compute nodal heat flow
        for(int i=0; i<Nc; i++){
          for(int j=0;j<Dm; j++){
            f[Dn* i+Dm ] += G[Dm* i+j ] * S[9+j];
      } }// }
    }//========================================================== End intp loop
    // Transpose R back again
    std::swap(R[1],R[3]); std::swap(R[2],R[6]); std::swap(R[5],R[7]);
    for(int i=0; i<Nc; i++){// rotate before summing in f
      for(int k=0; k<Dm; k++){
        for(int j=0; j<Dm; j++){
          f[Dn* i+k ] += GS[Dm* i+j ] * R[Dm* j+k ];
    } } }//--------------------------------------------- 2 *3*3*Nc = 18*Nc FLOP
#if VERB_MAX>10
    printf( "Nodal Response:");// (Elem: %u):", ie );
  for(int j=0;j<Ne;j++){
      if(j%Dn==0){printf("\n");}
      printf("%+9.2e ",f[j]);
    } printf("\n");
#endif
    for (int i=0; i<Nc; i++){// Write output back to system vector
      std::memcpy(& sysf[Econn[Nc*ie+i]*Dn],& f[Dn*i], sizeof(FLOAT_SOLV)*Dn );
    }
  }//============================================================ End elem loop
  return 0;
}