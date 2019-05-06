#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
//void ElastOrtho3D::ElemLinear( Elem* ){};//FIXME
int ElastOrtho3D::ElemLinear( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ElemJacobi( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ScatStiff( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::BlocLinear( Elem* ,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){
  return 0;
};
//
int ElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = elem_n *( conn_n*(2*18)
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

//void ElastOrtho3D::ElemLinear( Elem* E, FLOAT_SOLV *sys_f, const FLOAT_SOLV* sys_u ){
int ElastOrtho3D::ElemLinear( Elem* E, FLOAT_SOLV *sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Cleanup local variables.
//#pragma omp critical
//{
  const int Nd = 3;// Node (mesh) Dimension FIXME can include temperature?
  const int Nf = 3;// this->ndof_n DOF/node
  const int Nj = Nd*Nf+1;//FIXME wrong?
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n; };
  //
  FLOAT_MESH jac[Nj+16];//, det;
  FLOAT_PHYS u[Ne+16], f[Ne+16], GS[Ne+16], uR[Ne+16];
  FLOAT_PHYS G[Ne+16], H[Nd*Nf+16], S[Nd*Nf+16];//FIXME wrong sizes?
  //
  FLOAT_PHYS intp_shpg[intp_n*Ne];
  FLOAT_PHYS wgt[intp_n];
  FLOAT_PHYS C[this->mtrl_matc.size()];
  int sysn = E->node_n*3;
  for(int i=0; i<(intp_n*Ne);i++){ intp_shpg[i] = E->intp_shpg[i]; };
  for(int i=0; i<(intp_n);i++){ wgt[i] = E->gaus_weig[i]; };
  for(int i=0; i<mtrl_matc.size();i++){ C[i] = this->mtrl_matc[i]; };
  FLOAT_PHYS R[16] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8], 0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  FLOAT_PHYS Rs[16];
  for(int i=0; i<16;i++){ Rs[i] = R[i]; };
  Rs[1]=R[3]; Rs[2]=R[6]; Rs[5]=R[7];
  Rs[3]=R[1]; Rs[6]=R[2]; Rs[7]=R[5];

  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
//  for (int i; i<sysn; i++) sysu[i] = 1.;
//  for (int i; i<sysn; i++) sysf[i] = 0.;

  if(e0<ee){
    for (int i=0; i<Nc; i++){
      for( int j=0; j<3; j++){ u[Nf*i+j] = sysu[Econn[Nc*e0+i]*Nf+j]; };
    };
    for( int j=0; j<Nj; j++){ jac[j] = Ejacs[Nj*e0+j]; };
  }

  #pragma omp target data map(to:u[0:Ne+16], Rs[0:16], R[0:16], jac[0:Nj+16],intp_shpg[0:intp_n*Ne]) \
                          map(alloc:uR[0:Ne+16],GS[0:Ne],f[0:3*Nc],G[0:Ne+16])
  for(INT_MESH ie=e0;ie<ee;ie++){

    // READ f from HOST
    for (int i=0; i<Nc; i++){
      for(int j=0; j<3; j++){
        f[Nf*i+j]=sysf[Econn[Nc*ie+i]*3+j];
      }
    }

    // MOVE f to DEVICE
    #pragma omp target update to(Rs[0:16], u[0:Ne+16])
    #pragma omp target update to(f[0:3*Nc])

    #pragma omp target teams distribute parallel for collapse(2)
    for(int i=0; i<Nc; i++){// Rotate vectors in u
      for(int k=0; k<3; k++){
        uR[(3*i+k)] = 0.0;
        for(int j=0; j<3; j++){
          uR[(3* i+k) ] += u[(3* i+j) ] * Rs[(3* j+k) ];
        }
      }
    }//-------------------------------------------- 2 *3*3*Nc = 18*Nc FLOP

    if((ie+1)<ee){// Fetch stuff for the next iteration
      for (int i=0; i<Nc; i++){
        for(int j=0; j<3; j++){
          u[Nf*i+j]=sysu[Econn[Nc*(ie+1)+i]*Nf+j];
        };
      };
    };
    
    #pragma omp target teams distribute parallel for
    for(int i=0;i<Ne;i++){
      GS[i]=0.0;
    }

    #pragma omp target update from(uR[0:Ne+16])
    #pragma omp target update from(GS[0:Ne])

    for(int ip=0; ip<intp_n; ip++){

      //FLOAT_PHYS H_loc[9];
      //for(int i=0; i<9; i++){
      //  H_loc[i]=0.0;
      //}
      //for(int i=0; i<9; i++){
       // H[i]=0.0;
      //}
       
      double h0 = 0.;
      double h1 = 0.;
      double h2 = 0.;
      double h3 = 0.;
      double h4 = 0.;
      double h5 = 0.;
      double h6 = 0.;
      double h7 = 0.;
      double h8 = 0.;

#pragma omp target teams distribute parallel for reduction(+:h0, h1, h2, h3, h4, h5, h6,h7,h8) map(tofrom:G[0:Ne+16]) 
      for(int i=0; i<Nc; i++){
        for(int k=0; k<3; k++){
          G[3* i+k ]=0.0;
          G[3* i+k ] += intp_shpg[ip*Ne+ 3* i+0 ] * jac[3* 0+k ];
          G[3* i+k ] += intp_shpg[ip*Ne+ 3* i+1 ] * jac[3* 1+k ];
          G[3* i+k ] += intp_shpg[ip*Ne+ 3* i+2 ] * jac[3* 2+k ];
        }
        h0 += G[(3* i+0) ] * uR[Nf* i+0 ];
        h1 += G[(3* i+1) ] * uR[Nf* i+1 ];
        h2 += G[(3* i+2) ] * uR[Nf* i+2 ];
        h3 += G[(3* i+0) ] * uR[Nf* i+0 ];
        h4 += G[(3* i+1) ] * uR[Nf* i+1 ];
        h5 += G[(3* i+2) ] * uR[Nf* i+2 ];
        h6 += G[(3* i+0) ] * uR[Nf* i+0 ];
        h7 += G[(3* i+1) ] * uR[Nf* i+1 ];
        h8 += G[(3* i+2) ] * uR[Nf* i+2 ];
        
      } //------------------------------------------- 4 *3*3*Nc = 36*Nc*Ng FLOP

      const FLOAT_PHYS dw = jac[9] * wgt[ip];

      // Material Response
      S[0]=(C[0]* h0 + C[3]* h4 + C[5]* h8)*dw;//Sxx
      S[4]=(C[3]* h0 + C[1]* h4 + C[4]* h8)*dw;//Syy
      S[8]=(C[5]* h0 + C[4]* h4 + C[2]* h8)*dw;//Szz
      //
      S[1]=( h1 + h3 )*C[6]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( h5 + h7 )*C[7]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( h2 + h6 )*C[8]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      //------------------------------------------------------ 18+9= 27*Ng FLOP
  #pragma omp target teams distribute parallel for map(to:S[0:Nd*Nf+16])
      for(int i=0; i<Nc; i++){
        for(int k=0; k<3; k++){
          for(int j=0; j<3; j++){
            GS[(3* i+k) ] += G[(3* i+j) ] * S[(3* j+k) ];
          }
        }
      }//--------------------------------------- 2 *3*3*Nc = 18*Nc*Ng FLOP


    }//end intp loop

    if((ie+1)<ee){// Fetch stuff for the next iteration
       for(int i=0; i<Nj; i++){
          jac[i] = Ejacs[Nj*(ie+1)+i];
        }
     }
#pragma omp target update to(jac[0:Nj+16])

//    #pragma omp target update to(GS[0:Ne])
    #pragma omp target teams distribute parallel for
    for(int i=0; i<Nc; i++){// rotate before summing in f
      for(int k=0; k<3; k++){
        for(int j=0; j<3; j++){
          f[(3*i+k) ] += GS[(3*i+j) ] * R[(3*j+k)];
        }
      }
    }//-------------------------------------------- 2 *3*3*Nc = 18*Nc FLOP

    #pragma omp target update from(f[0:3*Nc])

    for (int i=0; i<Nc; i++){
      for(int j=0;j<3;j++){
        sysf[Econn[Nc*ie+i]*3+j] = f[Nf*i+j];
      }
    }
  }//end elem loop
  
  return 0;
};

int ElastOrtho3D::ElemJacobi(Elem* E, FLOAT_SOLV* sys_d ){
  //FIXME Doesn't do rotation yet
  const uint ndof   = 3;//this->ndof_n
  //const int mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = ndof*Nc;
  const uint intp_n = uint(E->gaus_n);
  //
  FLOAT_PHYS det;
  FLOAT_PHYS elem_diag[Ne];
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; };
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[3],mtrl_matc[5],0.0,0.0,0.0,
    mtrl_matc[3],mtrl_matc[1],mtrl_matc[4],0.0,0.0,0.0,
    mtrl_matc[5],mtrl_matc[4],mtrl_matc[2],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[6]*2.0,0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[7]*2.0,0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[8]*2.0};
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij],
               &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_diag[i]=0.0; };
    for(uint ip=0;ip<intp_n;ip++){
      //G   = MatMul3x3xN(jac,shg);
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; };
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<3;i++){
      for(uint j=0;j<3;j++){
        G[3* i+k] += jac[3* j+i ] * E->intp_shpg[ig+3* k+j ]; }; }; };
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
      #endif
      // xx yy zz
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
        B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
        B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
      // xy yx
        B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
        B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
      // yz zy
        B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
        B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
      // xz zx
        B[Ne*5 + 0+j*ndof] = G[Nc*2+j];
        B[Ne*5 + 2+j*ndof] = G[Nc*0+j];
      };
      #if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n");}
        printf("%+9.2e ",B[j]);
      }; printf("\n");
      #endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
      for(uint k=0; k<6 ; k++){
      //for(uint l=0; l<6 ; l++){
        //K[Ne*i+l]+= B[Ne*j + ij] * D[6*j + k] * B[Ne*k + l];
        //uint l=i;
        //elem_d[ie*Ne+i]+= B[Ne*j + i] * D[6*j + k] * B[Ne*k + i];
        elem_diag[i]+=(B[Ne*0 + i] * D[6*0 + k] * B[Ne*k + i])*w;
        elem_diag[i]+=(B[Ne*1 + i] * D[6*1 + k] * B[Ne*k + i])*w;
        elem_diag[i]+=(B[Ne*2 + i] * D[6*2 + k] * B[Ne*k + i])*w;
        elem_diag[i]+=(B[Ne*3 + i] * D[6*3 + k] * B[Ne*k + i])*w;
        elem_diag[i]+=(B[Ne*4 + i] * D[6*4 + k] * B[Ne*k + i])*w;
        elem_diag[i]+=(B[Ne*5 + i] * D[6*5 + k] * B[Ne*k + i])*w;
      };};//};//};
    };//end intp loop
    for (uint i=0; i<Nc; i++){
      //int c=E->elem_conn[Nc*ie+i]*3;
      for(uint j=0; j<3; j++){
        sys_d[E->elem_conn[Nc*ie+i]*3+j] += elem_diag[3*i+j];
      }; };
    //elem_diag=0.0;
  };
  return 0;
};

int ElastOrtho3D::ElemRowSumAbs(Elem* E, FLOAT_SOLV* sys_d ){
  //FIXME Doesn't do rotation yet
  const uint ndof   = 3;//this->ndof_n
  //const int mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = uint(ndof*Nc);
  const uint intp_n = E->gaus_n;
  //
  FLOAT_PHYS det;
  FLOAT_PHYS elem_sum[Ne];
  FLOAT_PHYS K[Ne*Ne];
  //RESTRICT Phys::vals B(Ne*6);
  FLOAT_PHYS B[Ne*6];//6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; };
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2]*2.0,0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2]*2.0,0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2]*2.0};
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij],
               &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_sum[i]=0.0; };
    for(uint i=0;i<(Ne*Ne);i++){ K[i]=0.0; };
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; };
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<3;i++){
      for(uint j=0;j<3;j++){
        G[3* i+k] += jac[3* j+i] * E->intp_shpg[ig+3* k+j]; }; }; };
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
      #endif
      // xx yy zz
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
        B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
        B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
      // xy yx
        B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
        B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
      // yz zy
        B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
        B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
      // xz zx
        B[Ne*5 + 0+j*ndof] = G[Nc*2+j];
        B[Ne*5 + 2+j*ndof] = G[Nc*0+j];
      };
      #if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n");}
        printf("%+9.2e ",B[j]);
      }; printf("\n");
      #endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
      for(uint l=0; l<Ne; l++){
      for(uint j=0; j<6 ; j++){
      for(uint k=0; k<6 ; k++){
        K[Ne*i+l]+= B[Ne*j + i] * D[6*j + k] * B[Ne*k + l]*w;
        //elem_sum[i]+=std::abs(B[Ne*j + i] * D[6*j + k] * B[Ne*k + l])*w; };
      };};};};
    };//end intp loop
    for (uint i=0; i<Ne; i++){
      for(uint j=0; j<Ne; j++){
        //elem_sum[i] += K[Ne*i+j]*K[Ne*i+j];
        elem_sum[i] += std::abs(K[Ne*i+j]);
      };};
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        sys_d[E->elem_conn[Nc*ie+i]*3+j] += elem_sum[3*i+j];
      };};
    //K=0.0; elem_sum=0.0;
  };
  return 0;
};
int ElastOrtho3D::ElemStrain( Elem* E,FLOAT_SOLV* sys_f ){
  //FIXME Clean up local variables.
  const uint ndof= 3;//this->ndof_n
  const uint  Nj =10;//,d2=9;//mesh_d*mesh_d;
  const INT_MESH elem_n = E->elem_n;
  const uint intp_n = uint(E->gaus_n);
  const uint     Nc = E->elem_conn_n;// Number of Nodes/Element
  const uint     Ne = ndof*Nc;
  //FLOAT_PHYS det;
  INT_MESH   conn[Nc];
  FLOAT_MESH jac[Nj];
  FLOAT_PHYS dw, G[Ne], f[Ne];
  FLOAT_PHYS H[9], S[9], A[9], B[9];
  //
  for(uint i=0; i< 9 ; i++){ A[i]=0.0; };
  for(uint i=0; i< 9 ; i++){ H[i]=0.0; };
  H[0]=1.0; H[4]=1.0; H[8]=1.0;// unit pressure
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
  const FLOAT_PHYS R[9] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};
    for(uint i=0; i<3; i++){
      for(uint k=0; k<3; k++){
        for(uint j=0; j<3; j++){
            H[(3* i+k) ] += A[(3* i+j)] * R[3* k+j ];
    };};};
  const auto Econn = &E->elem_conn[0];
  const auto Ejacs = &E->elip_jacs[0];
  //
  for(INT_MESH ie=0;ie<elem_n;ie++){
    std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc);
    std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
    //
    for(uint i=0;i<(Ne);i++){ f[i]=0.0; };
    for(uint ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      //for(uint i=0; i<(Ne) ; i++){ G[i]=0.0; };
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<3 ; i++){ G[3* k+i ]=0.0;
          for(uint j=0; j<3 ; j++){
            G[(3* k+i) ] += jac[3* j+i ] * intp_shpg[ip*Ne+ 3* k+j ];
          };
        };
      };//------------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(uint j=0;j<HH.size();j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      }; printf("\n");
#endif
      //det=jac[9 +Nj*l]; FLOAT_PHYS w = det * wgt[ip];
      dw = jac[9] * wgt[ip];
      //
      S[0]=(C[0]* H[0] + C[1]* H[4] + C[1]* H[8])*dw;//Sxx
      S[4]=(C[1]* H[0] + C[0]* H[4] + C[1]* H[8])*dw;//Syy
      S[8]=(C[1]* H[0] + C[1]* H[4] + C[0]* H[8])*dw;//Szz
      //
      S[1]=( H[1] + H[3] )*C[2]*dw;// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[2]*dw;// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[2]*dw;// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      //------------------------------------------------------- 18+9 = 27 FLOP
      for(uint i=0; i<3; i++){
        //for(int k=0; k<3; k++){ A[3* i+k ]=0.0;
        for(uint k=0; k<3; k++){ B[3* k+i ]=0.0;
          for(uint j=0; j<3; j++){
            //A[3* i+k ] += S[3* i+j ] * R[3* j+k ];
            B[(3* k+i) ] += S[(3* i+j) ] * R[3* j+k ];// B is transposed
      };};};
      for(uint i=0; i<Nc; i++){
        for(uint k=0; k<3; k++){
          for(uint j=0; j<3; j++){
            f[(3* i+k) ] += G[(3* i+j) ] * B[(3* k+j) ];
      };};};//---------------------------------------------- N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(uint j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      }; printf("\n");
#endif
    };//end intp loop
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        //sys_f[3*conn[i]+j] +=f[(3*i+j)];
        sys_f[4*conn[i]+j] += std::abs( f[(3*i+j)] );
    }; };//--------------------------------------------------- N*3 =  3*N FLOP
  };//end elem loop
  return 0;
  };
