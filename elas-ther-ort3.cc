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
int ThermElastOrtho3D::ElemLinear( Elem* ){ return 1; };//FIXME
int ThermElastOrtho3D::ElemJacobi( Elem* ){ return 1; };//FIXME
int ThermElastOrtho3D::ElemStiff( Elem* ){ return 1; };//FIXME
int ThermElastOrtho3D::BlocLinear( Elem* ,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){ return 1; };
int ThermElastOrtho3D::ElemStrain( Elem* ,FLOAT_SOLV*  ){ return 1; }
//
int ThermElastOrtho3D::ElemJacobi(Elem* E, FLOAT_SOLV* sys_d ){
  //FIXME Doesn't do rotation yet
  const uint Dm = 3;//this->mesh_d
  const uint Dn = this->node_d;
  //const int mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint Nc = E->elem_conn_n;
  const uint Nj = 10,d2=9;
  const uint Ne = Dm*Nc;
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
  const FLOAT_PHYS scal_disp = udof_magn[0] ;
  FLOAT_PHYS scal_ther = udof_magn[3] * 5e-4;//FIXME magic number
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    if(j%Dm==0){printf("\n");}
    printf("%+9.2e ",mtrl_matc[j]);
  }; printf("\n");
#endif
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
        G[Nc* i+k] += jac[3* j+i ] * E->intp_shpg[ig+3* k+j ]; }; }; };
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
      #endif
      // xx yy zz
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*Dm] = G[Nc*0+j];
        B[Ne*1 + 1+j*Dm] = G[Nc*1+j];
        B[Ne*2 + 2+j*Dm] = G[Nc*2+j];
      // xy yx
        B[Ne*3 + 0+j*Dm] = G[Nc*1+j];
        B[Ne*3 + 1+j*Dm] = G[Nc*0+j];
      // yz zy
        B[Ne*4 + 1+j*Dm] = G[Nc*2+j];
        B[Ne*4 + 2+j*Dm] = G[Nc*1+j];
      // xz zx
        B[Ne*5 + 0+j*Dm] = G[Nc*2+j];
        B[Ne*5 + 2+j*Dm] = G[Nc*0+j];
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
          elem_diag[i]+=B[Ne*0 + i] * D[6*0 + k] * B[Ne*k + i] * scal_disp * w;
          elem_diag[i]+=B[Ne*1 + i] * D[6*1 + k] * B[Ne*k + i] * scal_disp * w;
          elem_diag[i]+=B[Ne*2 + i] * D[6*2 + k] * B[Ne*k + i] * scal_disp * w;
          elem_diag[i]+=B[Ne*3 + i] * D[6*3 + k] * B[Ne*k + i] * scal_disp * w;
          elem_diag[i]+=B[Ne*4 + i] * D[6*4 + k] * B[Ne*k + i] * scal_disp * w;
          elem_diag[i]+=B[Ne*5 + i] * D[6*5 + k] * B[Ne*k + i] * scal_disp * w;
        }
      }
      //if(Dn>Dm){
        for(uint i=0; i<Nc; i++){
          for(uint k=0; k<Dm ; k++){
            sys_d[E->elem_conn[Nc*ie+i]*Dn+Dm] +=// 1e-4* //1e-3 ok
              //G[Nc* k+i] * mtrl_matc[12+k] * G[Nc* k+i] * this->udof_magn[j] * w;
              G[Nc* 0+i] * G[Nc* k+i]*mtrl_matc[12+0] * scal_ther * w
             +G[Nc* 1+i] * G[Nc* k+i]*mtrl_matc[12+1] * scal_ther * w
             +G[Nc* 2+i] * G[Nc* k+i]*mtrl_matc[12+2] * scal_ther * w;
          }
        }
      //}
    };//end intp loop
#if 0
    printf("MTRL_MATC:");
    for(uint i=0;i<mtrl_matc.size(); i++){printf(" %e",mtrl_matc[i]);}
    printf("\n");
#endif
    for (uint i=0; i<Nc; i++){
      //int c=E->elem_conn[Nc*ie+i]*3;
      for(uint j=0; j<Dm; j++){
        sys_d[E->elem_conn[Nc*ie+i]*Dn+j] += elem_diag[Dm*i+j];
      }
    }
  };
  return 0;
};

int ThermElastOrtho3D::ElemRowSumAbs(Elem* E, FLOAT_SOLV* sys_d ){
  //FIXME Doesn't do rotation yet
  const uint ndof   = 3;//this->node_d
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
