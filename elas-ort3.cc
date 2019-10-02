#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
int ElastOrtho3D::ElemLinear( Elem* ){ return 1; }//FIXME
int ElastOrtho3D::ElemJacobi( Elem* ){ return 1; }//FIXME
int ElastOrtho3D::BlocLinear( Elem* ,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){ return 1; }
int ElastOrtho3D::ElemNonlinear( Elem*, const INT_MESH, const INT_MESH,
  FLOAT_SOLV*, const FLOAT_SOLV*, const FLOAT_SOLV*, bool ){
  return 1; }
int ElastOrtho3D::ElemJacobi( Elem*, FLOAT_SOLV*, const FLOAT_SOLV* ){
  return 1; }
//
int ElastOrtho3D::ElemStrainStress(std::ostream& of,
  Elem* E, FLOAT_SOLV* part_u) {
  //FIXME Cleanup local variables.
  const int Dm = 3;//E->mesh_d;// Node (mesh) Dimension FIXME should be elem_d?
  const int Dn = 3;//this->node_d;// this->node_d DOF/node
  const int Nj = 10;//Dm*Dm+1;// Jac inv & det
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Dn*Nc;
  const INT_MESH elem_n =E->elem_n;
  const int intp_n = int(E->gaus_n);
  //
  INT_MESH e0=0, ee=elem_n;
  //if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  //}else{ e0=E->halo_elem_n; ee=elem_n; };
  //
#if VERB_MAX>11
  printf("Meshd: %i, Noded: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
    (int)Dm,(int)Dn,(int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_MESH jac[Nj];//, det;
  FLOAT_PHYS u[Ne], uR[Ne];
  FLOAT_PHYS G[Dm*Nc], H[Dm*Dn], S[Dm*Dn];//FIXME wrong sizes?
  //FIXME
  FLOAT_PHYS intp_shpf[intp_n*Nc];
  // Make local copies of constant data structures
  //if(has_therm){
  std::copy( &E->intp_shpf[0],
             &E->intp_shpf[intp_n*Nc], intp_shpf );
  //}
  FLOAT_PHYS intp_shpg[intp_n*Dm*Nc];
  std::copy( &E->intp_shpg[0],
             &E->intp_shpg[intp_n*Dm*Nc], intp_shpg );
  //FLOAT_PHYS wgt[intp_n];
  //std::copy( &E->gaus_weig[0],
  //           &E->gaus_weig[intp_n], wgt );
  FLOAT_PHYS C[this->mtrl_matc.size()];
  std::copy( &this->mtrl_matc[0],
             &this->mtrl_matc[this->mtrl_matc.size()], C );
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
  const FLOAT_SOLV* RESTRICT sysu  = &part_u[0];
#if VERB_MAX>10
    printf( "Displacements:");// (Elem: %u):", ie );
  for(int j=0;j<Ne;j++){
      if(j%Dn==0){printf("\n");}
      printf("%+9.2e ",u[j]);
    } printf("\n");
#endif
  for(INT_MESH ie=e0;ie<ee;ie++){//=============================== Element Loop
    // Transpose R
    std::swap(R[1],R[3]); std::swap(R[2],R[6]); std::swap(R[5],R[7]);
    for (int i=0; i<Nc; i++){
      std::memcpy(&u[Dn*i],&sysu[Econn[Nc*ie+i]*Dn],sizeof(FLOAT_SOLV)*Dn);
    }
    std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
    for(int i=0; i<Nc; i++){// Rotate vectors in u
      for(int k=0; k<Dm; k++){ uR[(Dn* i+k) ]=0.0;
        for(int j=0; j<Dm; j++){
          uR[(Dn* i+k) ] += u[(Dn* i+j) ] * R[(Dm* j+k) ];
    } } }
    for(int ip=0; ip<intp_n; ip++){//=================== Integration Point Loop
      of << E->elem_glid[ie] <<","<< ip+1;
      of <<",";
      //FIXME Int pt coords.
      //
      //
      //
      //G = MatMul3x3xN( jac,shg );
      for(int i=0; i<(Dm*Dn); i++){ H[i]=0.0; };
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Dm ; k++){ G[Dm* i+k ]=0.0;
          for(int j=0; j<Dm ; j++){
            G[Dm* i+k ] += intp_shpg[ip*Dm*Nc+ Dm* i+j ] * jac[Dm* j+k ];
          }
          for(int j=0; j<Dn ; j++){// Unsymmetric small strain tensor
            H[Dm* j+k ] += G[Dm* i+k ] * uR[Dn* i+j ];
          }
        }
      }
      FLOAT_PHYS strain[Dm*Dm];
      for(int i=0; i<(Dm*Dn); i++){ strain[i]=0.0; }
      for(int i=0; i<Dm; i++){// un-rotate strains
        for(int k=0; k<Dm ; k++){
          for(int j=0; j<Dm ; j++){
            for(int l=0; l<Dm ; l++){
              strain[Dm* i+l ] +=  R[Dm* i+j ] * H[Dm* j+k ] * R[Dm* l+k ];
            }
          }
        }
      }
      of<< strain[0] <<","<< strain[4] <<","<< strain[8] <<","
        << strain[1]+strain[3] <<","
        << strain[5]+strain[7] <<","
        << strain[2]+strain[6];
      of<< ",";
#if VERB_MAX>11
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<(Dm*Dn);j++){
        if(j%Dm==0){printf("\n");}
        printf("%+9.2e ",strain[j]);
      } printf("\n");
#endif
      // Elastic material response
      S[0]=(C[0]* H[0] + C[3]* H[4] + C[5]* H[8]);//Sxx
      S[4]=(C[3]* H[0] + C[1]* H[4] + C[4]* H[8]);//Syy
      S[8]=(C[5]* H[0] + C[4]* H[4] + C[2]* H[8]);//Szz
      //
      S[1]=( H[1] + H[3] )*C[6];// S[3]= S[1];//Sxy Syx
      S[5]=( H[5] + H[7] )*C[7];// S[7]= S[5];//Syz Szy
      S[2]=( H[2] + H[6] )*C[8];// S[6]= S[2];//Sxz Szx
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      FLOAT_PHYS stress[Dm*Dm];
      for(int i=0; i<(Dm*Dn); i++){ stress[i]=0.0; }
      for(int i=0; i<Dm; i++){
        for(int k=0; k<Dm ; k++){
          for(int j=0; j<Dm ; j++){
            for(int l=0; l<Dm ; l++){
              stress[Dm* i+l ] +=  R[Dm* i+j ] * S[Dm* j+k ] * R[Dm* l+k ];
            }
          }
        }
      }
      of<< stress[0] <<","<< stress[4] <<","<< stress[8] <<","
        <<(stress[1]+stress[3])*0.5 <<","
        <<(stress[5]+stress[7])*0.5 <<","
        <<(stress[2]+stress[6])*0.5;
      of<< '\n';
#if VERB_MAX>11
      printf( "Stress:");
      for(int j=0;j<(Dn*Dm);j++){
        if(j%Dm==0){printf("\n");}
        printf("%+9.2e ",stress[j]);
      } printf("\n");
#endif
    }//========================================================== End intp loop
  std::swap(R[1],R[3]); std::swap(R[2],R[6]); std::swap(R[5],R[7]);
  }//============================================================ End elem loop
  return 0;
}
int ElastOrtho3D::ElemStiff(Elem* E  ){
  //FIXME Doesn't do rotation yet
  //FIXME Doesn't do thermal yet
  const uint Dm = 3;//E->mesh_d
  const uint Dn = this->node_d;
  const uint Nj = 10,d2=9;
  const uint Nc = E->elem_conn_n;
  const uint Ne = Dm*Nc;
  const uint Nr = Dn*Nc;// One row of stiffness matrix
  const uint Nk = Nr*Nr;// Elements of stiffness matrix
  const uint elem_n = E->elem_n;
  const uint intp_n = uint(E->gaus_n);
  //
  FLOAT_PHYS det;
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[3],mtrl_matc[5],0.0,0.0,0.0,
    mtrl_matc[3],mtrl_matc[1],mtrl_matc[4],0.0,0.0,0.0,
    mtrl_matc[5],mtrl_matc[4],mtrl_matc[2],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[6]*2.0,0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[7]*2.0,0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[8]*2.0 };
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    if(j%Dm==0){ printf("\n"); }
    printf("%+9.2e ",mtrl_matc[j]);
  } printf("\n");
#endif
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij], &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<3;i++){
      for(uint j=0;j<3;j++){
        G[Nc* i+k] += jac[3* j+i ] * E->intp_shpg[ig+3* k+j ]; } } }
#if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){ printf("\n"); }
        printf("%+9.2e",jac[j]);
      } printf(" det:%+9.2e\n",det);
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
#if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){ printf("\n"); }
        printf("%+9.2e ",B[j]);
      } printf("\n");
#endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
      for(uint l=0; l<Ne; l++){
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        elem_stiff[Nk*ie +Nr* i+l ]+=B[Ne* i+j ] * D[6* k+j ] * B[Ne* k+l ] * w;
      } } } }
#if 0
      if(Dn>Dm){//FIXME
        for(uint i=0; i<Nc; i++){
          for(uint k=0; k<Dm ; k++){
            part_d[E->elem_conn[Nc*ie+i]*Dn+Dm] +=// 1e-4* //1e-3 ok
              //G[Nc* k+i] * mtrl_matc[12+k] * G[Nc* k+i] * this->udof_magn[j] * w;
              G[Nc* 0+i] * G[Nc* k+i]*mtrl_matc[12+0] * w
             +G[Nc* 1+i] * G[Nc* k+i]*mtrl_matc[12+1] * w
             +G[Nc* 2+i] * G[Nc* k+i]*mtrl_matc[12+2] * w;
          } }
      }
#endif
    }//end intp loop
  }//end elem loop
  return 0;
}//============================================================== End ElemStiff
int ElastOrtho3D::ElemJacobi(Elem* E, FLOAT_SOLV* part_d ){
  //FIXME Doesn't do rotation yet
  const uint Dm = 3;//E->mesh_d
  const uint Dn = this->node_d;
  const uint Nj = 10,d2=9;
  const uint Nc = E->elem_conn_n;
  const uint Ne = Dm*Nc;
  const uint elem_n = E->elem_n;
  const uint intp_n = uint(E->gaus_n);
  //
  FLOAT_PHYS det;
  FLOAT_PHYS elem_diag[Ne];
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[3],mtrl_matc[5],0.0,0.0,0.0,
    mtrl_matc[3],mtrl_matc[1],mtrl_matc[4],0.0,0.0,0.0,
    mtrl_matc[5],mtrl_matc[4],mtrl_matc[2],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[6]*2.0,0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[7]*2.0,0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[8]*2.0 };
  const FLOAT_PHYS scal_disp = udof_magn[0] ;
  FLOAT_PHYS scal_ther;
  if(Dn>Dm){ scal_ther = udof_magn[3]; }// * 1e-3; }//4e-4
  //if(Dn>Dm){ scal_ther = udof_magn[3] *1e-2; }
  else{ scal_ther=1.0; }
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    if(j%Dm==0){printf("\n");}
    printf("%+9.2e ",mtrl_matc[j]);
  } printf("\n");
#endif
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij], &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint i=0;i<Ne;i++){ elem_diag[i]=0.0; }
    for(uint ip=0;ip<intp_n;ip++){
      //G   = MatMul3x3xN(jac,shg);
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<3;i++){
      for(uint j=0;j<3;j++){
        G[Nc* i+k] += jac[3* j+i ] * E->intp_shpg[ig+3* k+j ]; } } }
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){ printf("\n"); }
        printf("%+9.2e",jac[j]);
      } printf(" det:%+9.2e\n",det);
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
      };
      #if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n"); }
        printf("%+9.2e ",B[j]);
      } printf("\n");
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
      if(Dn>Dm){
        for(uint i=0; i<Nc; i++){
          for(uint k=0; k<Dm; k++){
            part_d[E->elem_conn[Nc*ie+i]*Dn+Dm] +=
              G[Nc* 0+i] * G[Nc* k+i]*mtrl_matc[12+0] * scal_ther * w
             +G[Nc* 1+i] * G[Nc* k+i]*mtrl_matc[12+1] * scal_ther * w
             +G[Nc* 2+i] * G[Nc* k+i]*mtrl_matc[12+2] * scal_ther * w;
          }
        }
      }
    }//end intp loop
    for (uint i=0; i<Nc; i++){
      //int c=E->elem_conn[Nc*ie+i]*3;
      for(uint j=0; j<Dm; j++){
        part_d[E->elem_conn[Nc*ie+i]*Dn+j] += elem_diag[Dm*i+j];
        //part_d[E->elem_conn[Nc*ie+i]*Dn+j] += std::abs(elem_diag[Dm*i+j]);
      }
      //for(uint j=3; j<Dn; j++){ part_d[E->elem_conn[Nc*ie+i]*Dn+j] = 1.0; }
    }
    //elem_diag=0.0;
  }
  return 0;
}
int ElastOrtho3D::ElemRowSumAbs(Elem* E, FLOAT_SOLV* part_d ){
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
        part_d[E->elem_conn[Nc*ie+i]*3+j] += elem_sum[3*i+j];
      };};
    //K=0.0; elem_sum=0.0;
  };
  return 0;
};
int ElastOrtho3D::ElemStrain( Elem* E,FLOAT_SOLV* part_f ){
  //FIXME Clean up local variables.
  const uint ndof= 3;//this->node_d
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
        //part_f[3*conn[i]+j] +=f[(3*i+j)];
        part_f[4*conn[i]+j] += std::abs( f[(3*i+j)] );
    }; };//--------------------------------------------------- N*3 =  3*N FLOP
  };//end elem loop
  return 0;
  };
#if 0
int ElastOrtho3D::ReadPartFMR( const char* fname, bool is_bin ){
  //FIXME This is not used. It's done in Mesh::ReadPartFMR...
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";}
  if(is_bin){
    std::cout << "ERROR Could not open "<< fname << " for reading." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  }
  std::string fmrstring;
  std::ifstream fmrfile(fname);
  while( fmrfile >> fmrstring ){
    if(fmrstring=="$ElasticProperties"){//FIXME Deprecated
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i]; }
      //this->MtrlProp2MatC();
      s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; mtrl_dirs[i]*=(PI/180.0) ;}
      }
    }
    if(fmrstring=="$Orientation"){// Material orientation (radians)
      int s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; mtrl_dirs[i]*=(PI/180.0) ;}
      }
    }
    //FIXME This parsing requires properties in a specific order
    auto tprop = mtrl_prop; auto tsz=tprop.size();
    if(fmrstring=="$Elastic"){// Elastic Constants
      int s=0; fmrfile >> s;
      mtrl_prop.resize(tsz+s);
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
    if(fmrstring=="$ThermalExpansion"){// Thermal expansion
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s + tprop.size());
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
    if(fmrstring=="$ThermalConductivity"){// Thermal conductivity
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s + tprop.size());
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
  }
  return 0;
}
int ElastOrtho3D::SavePartFMR( const char* fname, bool is_bin ){
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not append "<< fname << "." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  };
  std::ofstream fmrfile;
  fmrfile.open(fname, std::ios_base::app);
  //
  fmrfile << "$ElasticProperties" <<'\n';
  fmrfile << mtrl_prop.size();
  for(uint i=0;i<mtrl_prop.size();i++){ fmrfile <<" "<< mtrl_prop[i]; };
  fmrfile << '\n';
  if(mtrl_dirs.size()>0){
    fmrfile << mtrl_dirs.size();
    for(uint i=0;i<mtrl_dirs.size();i++){ fmrfile <<" "<< mtrl_dirs[i]; };
  }; fmrfile <<'\n';
  return 0;
};
#endif