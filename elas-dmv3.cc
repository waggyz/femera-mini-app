#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
int ElastDmv3D::ElemLinear( Elem* ){ return 1; }//FIXME
int ElastDmv3D::ElemJacobi( Elem* ){ return 1; }//FIXME
int ElastDmv3D::BlocLinear( Elem* ,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){
  return 1;
  }
int ElastDmv3D::ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*){
  return 1;
}
int ElastDmv3D::ElemJacobi( Elem*, FLOAT_SOLV*, const FLOAT_SOLV* ){
  return 1; }
int ElastDmv3D::ElemJacNode(Elem* E, FLOAT_SOLV* part_d ){
#define DIAG_FROM_STIFF
  const uint Dm = 3;
  const uint Nj = 10;
  const uint Nc = E->elem_conn_n;
  const uint Ne = uint(Dm*Nc);// One row of stiffness matrix
  const uint elem_n = E->elem_n;
  const uint intp_n = E->gaus_n;
#ifdef HAS_AVX
  const uint Dr = 8;
#else
  const uint Dr = 6;
#endif
#ifdef DIAG_FROM_STIFF
  const uint Nk = Ne*Ne;// Elements of stiffness matrix
  FLOAT_PHYS elem_stiff[Nk];
#endif
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_SOLV* RESTRICT D     = &mtrl_dmat[0];
  for(uint ie=0;ie<elem_n;ie++){
#ifdef DIAG_FROM_STIFF
    for(uint i=0;i<Nk;i++){ elem_stiff[i]=0.0; }
#endif
    for(uint ip=0;ip<intp_n;ip++){
#if 1
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<Dm ; i++){ G[Nc* i+k ]=0.0;
          for(uint j=0; j<Dm ; j++){
            G[Nc* i+k ] += E->elip_jacs[Nj*ie+ Dm* j+i ]
                         * E->intp_shpg[ip*Ne+ Dm* k+j ];
      } } }
#else
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i< 3;i++){
      for(uint j=0;j< 3;j++){
        G[Nc* i+k] += E->elip_jacs[Nj*ie+3* j+i] * E->intp_shpg[ig+3* k+j];
      } } }
#endif
      for(uint j=0; j<Nc; j++){
      // xx yy zz
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
      }
      FLOAT_PHYS dw = E->elip_jacs[Nj*ie+9] * E->gaus_weig[ip];
#ifdef DIAG_FROM_STIFF
      for(uint i=0; i<Ne; i++){
      for(uint l=0; l<Ne; l++){
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        elem_stiff[Ne* i+l ] += B[Ne* j+i ] * D[Dr* k+j ] * B[Ne* k+l ] * dw;
      } } } }
#else
      for(uint n=0; n<Nc; n++){
      for(uint m=0; m<3 ; m++){ uint i=3*n+m;
      for(uint o=0; o<3 ; o++){ uint l=3*n+o;
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        part_d[E->elem_conn[Nc*ie+n]*9+ 3* m+o ]
          += B[Ne* j+i ] * D[Dr* k+j ] * B[Ne* k+l ] * dw
#if 0
          *((m==o)?1.0:-1.0)
#endif
          ;
      } } } } }
#endif
    }//end intp loop
#ifdef DIAG_FROM_STIFF
    for(uint i=0; i<Nc; i++){
    for(uint k=0; k<3 ; k++){
    for(uint j=0; j<3 ; j++){
      part_d[E->elem_conn[Nc*ie+i]*9+ 3* k+j ]
        += elem_stiff[3*Ne*i +3*i   +Ne* k+j ];
    } } }
#endif
  }//end elem loop
  return 0;
}
//
int ElastDmv3D::ElemNonlinear( Elem* E, const INT_MESH e0, const INT_MESH e1,
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u, const FLOAT_SOLV*, bool ){
  return this->ElemLinear( E, e0,e1, part_f, part_u );
  }
int ElastDmv3D::ElemStiff(Elem* E ){//FIXME should be ScatterStiff( E )
  const int Dm = 3;//E->mesh_d
  const int Dn = 3;//this->node_d;
  const int Nj = 10;
  const int Nc = int(E->elem_conn_n);
  const int Ne = Dm*Nc;
  const int Nr = Dn*Nc;// One row/col of stiffness matrix
#ifdef HAS_AVX
  const uint Dr = 8;
#else
  const uint Dr = 6;
#endif
#ifdef __INTEL_COMPILER
  const int Nk =int(Nr*(Nr + 1))/2;// Packed symmetric stiffness
#else
  const int Nk = Nr * Nr;// Elements of stiffness matrix
#endif
  const INT_MESH elem_n = int(E->elem_n);
  const int intp_n = int(E->gaus_n);
  //
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne];
  for(int j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_SOLV* RESTRICT D     = &mtrl_dmat[0];
  for(INT_MESH ie=0;ie<elem_n;ie++){
    for(int ip=0;ip<intp_n;ip++){
      int ig=ip*Ne;
      for(int i=0;i<Ne;i++){ G[i]=0.0; }
      for(int k=0;k<Nc;k++){
      for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        G[Nc* i+k] += E->elip_jacs[Nj*ie+3* j+i ] * E->intp_shpg[ig+3* k+j ];
      } } }
      for(int j=0; j<Nc; j++){
      // xx yy zz
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
      }
#if VERB_MAX>10
      printf( "[B]:");
      for(int j=0;j<B.size();j++){
        if(j%Ne==0){ printf("\n"); }
        printf("%+9.2e ",B[j]);
      } printf("\n");
#endif
      FLOAT_PHYS w = E->elip_jacs[Nj*ie+9] * E->gaus_weig[ip];
#ifdef __INTEL_COMPILER
      int ik=-1;
#endif
      for(int i=0; i<Nr; i++){
      for(int l=0; l<Nr; l++){
#ifdef __INTEL_COMPILER
        ik=ik+int(i<=l);
        if(i<=l){
#endif
      for(int k=0; k<6 ; k++){
      for(int j=0; j<6 ; j++){
#ifdef __INTEL_COMPILER
        // Use packed symmmetric matrix storage.
          elem_stiff[Nk*ie + ik ]+=B[Ne* j+i ] * D[Dr* k+j ] * B[Ne* k+l ] * w;
        }
#else
        elem_stiff[Nk*ie +Nr* i+l ]+=B[Ne* j+i ] * D[Dr* k+j ] * B[Ne* k+l ] * w;
#endif
      } } } }
    }// end intp loop
  }// End elem loop
  return 0;
}//============================================================== End ElemStiff
int ElastDmv3D::ElemJacobi(Elem* E, FLOAT_SOLV* part_d ){
  const uint ndof   = 3;//this->node_d
  const uint  Nj = 10;
  const uint  Nc = E->elem_conn_n;
  const uint  Ne = uint(ndof*Nc);
  const uint elem_n = E->elem_n;
  const uint intp_n = E->gaus_n;
#ifdef HAS_AVX
  const uint Dr = 8;
#else
  const uint Dr = 6;
#endif
  //
  RESTRICT Phys::vals elem_diag(Ne);
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_PHYS* RESTRICT D     = &mtrl_dmat[0];
#if 0
  for(int i=0;i<48;i++){ printf(" %7.1e",D[i]); if(!(i%8)){printf("\n");} }
#endif
  for(uint ie=0;ie<elem_n;ie++){
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i< 3;i++){
      for(uint j=0;j< 3;j++){
        G[Nc* i+k] += E->elip_jacs[Nj*ie+3* j+i] * E->intp_shpg[ig+3* k+j]; } } }
      for(uint j=0; j<Nc; j++){
      // xx yy zz
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
      }
      FLOAT_PHYS w = E->elip_jacs[Nj*ie+9] * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        elem_diag[i]+=(B[Ne*j + i] * D[Dr*j + k] * B[Ne*k + i])*w;
      } } }
    }//end intp loop
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        part_d[E->elem_conn[Nc*ie+i]*3+j] += elem_diag[3*i+j];
      } }
    elem_diag=0.0;
  }
  return 0;
}
int ElastDmv3D::ElemRowSumAbs(Elem*, FLOAT_SOLV* ){
  return 1; }
int ElastDmv3D::ElemStrain( Elem*,FLOAT_SOLV* ){
  return 1; }