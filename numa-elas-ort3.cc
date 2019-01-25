#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
int ElastOrtho3D::ElemLinear( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ElemJacobi( Elem* ){ return 1; };//FIXME
int ElastOrtho3D::ScatStiff( Elem* ){ return 1; };//FIXME
//
int ElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (33+18) + 2*45 + 27 );;
  this->tens_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    *(3*uint(E->elem_conn_n)*3+ jacs_n*10);
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) -1+2);
  return 0;
};
//int ElastOrtho3D::ElemLinear( std::vector<Elem*> list_elem,
//FIXME add the above variant as an option...
int ElastOrtho3D::ElemLinear( Elem* E,
  RESTRICT Phys::vals &sys_f, const RESTRICT Phys::vals &sys_u ){
  //FIXME Don't need these local variables anymore?
  static const int ndof   = 3;//this->ndof_n
  //static const int mesh_d = 3;//E->elem_d;
  static const uint  Nj =10;//,d2=9;//mesh_d*mesh_d;
  uint ij=0;
  //
  const INT_MESH elem_n = E->elem_n;
  //const int    intp_n = E->elip_dets.size()/elem_n;
  const uint     Nc = E->elem_conn_n;// Number of Nodes/Element
  const uint     Ne = ndof*Nc;//, nej=E->elip_jacs.size();
  INT_MESH         Ng;
  const int intp_n = E->gaus_n;//E->elip_jacs.size()/elem_n/Nj;
  #if VERB_MAX>11
  printf("Dim: %i, Elems:%i, IntPts:%i, Nodes/elem:%i\n",
    (int)mesh_d,(int)elem_n,(int)intp_n,(int)Nc);
  #endif
  //RESTRICT Phys::vals u(Ne),f(Ne), G(Ne);
  FLOAT_PHYS G[Ne], u[Ne],f[Ne];//,shg[Ne] //FIXME Should f[] be Phys::vals?
  //RESTRICT Phys::vals jac(9),sV(6),H(9);
  FLOAT_PHYS det, jac[Nj], A[9], B[9];
  FLOAT_PHYS intp_shpg[intp_n*Ne];
  std::copy( &E->intp_shpg[0],// Prefetch seems to help
             &E->intp_shpg[intp_n*Ne], intp_shpg );
  FLOAT_PHYS wgt[intp_n];
  std::copy( &E->gaus_weig[0],
             &E->gaus_weig[intp_n], wgt );
  const FLOAT_PHYS R[9] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8]};
  //int imfirst=0;
  #if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",mtrl_matc[j]);
  }; printf("\n");
  #endif
  //
  INT_MESH e0=0, ee=elem_n;
  if(E->do_halo==true){ e0=0; ee=E->halo_elem_n;
  }else{ e0=E->halo_elem_n; ee=elem_n; };
  //
  for(uint ie=e0;ie<ee;ie++){
  //for(INT_MESH ie=0;ie<elem_n;ie++){
    //if(imfirst==0){//FIXME Need another buffer to prefetch this
    //  std::copy( &elem_inout[ie*Ne],
    //             &elem_inout[ie*Ne+Ne], u );//};
    ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij],
               &E->elip_jacs[ij+Nj], jac ); det=jac[9];
    for (uint i=0; i<Nc; i++){//FIXME replace elem_d with dofs_n
      std::memcpy( &u[ndof*i], &sys_u[E->elem_conn[Nc*ie+i]*ndof],
                    sizeof(FLOAT_SOLV)*ndof ); };
    for(uint j=0;j<Ne;j++){ f[j]=0.0; };// Seems faster here instead of after.
    for(int ip=0; ip<intp_n; ip++){
      //if(imfirst==0){ //ij=Nj*ie*intp_n;
      //  ij=Nj*ie*intp_n+Nj*ip;//FIXME Figure out how to put these below...
      //  std::copy( &E->elip_jacs[ij],
      //             &E->elip_jacs[ij+Nj], jac );};
      //imfirst=1;
      Ng = ip*Ne;
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      // Unroll G & H together here.
      for(int i=0; i<9; i++){ A[i]=0.0; };
      for(uint k=0; k<Nc; k++){//for(uint i=0; i<3 ; i++){//for(uint j=0; j<3 ; j++){
        G[Nc* 0+k ]  = jac[3* 0+0 ] * intp_shpg[Ng+ Nc* 0+k ]
                     + jac[3* 0+1 ] * intp_shpg[Ng+ Nc* 1+k ]
                     + jac[3* 0+2 ] * intp_shpg[Ng+ Nc* 2+k ];// Nc* 5 FLOP
      //};for(uint k=0; k<Nc; k++){
        A[ 3* 0+0 ] += G[Nc* 0+k ] * u[ndof* k+0 ];
        A[ 3* 0+1 ] += G[Nc* 0+k ] * u[ndof* k+1 ];
        A[ 3* 0+2 ] += G[Nc* 0+k ] * u[ndof* k+2 ];// Nc* 6 FLOP
      };for(uint k=0; k<Nc; k++){//NOTE Time these with and without these
        G[Nc* 1+k ]  = jac[3* 1+0 ] * intp_shpg[Ng+ Nc* 0+k ]
                     + jac[3* 1+1 ] * intp_shpg[Ng+ Nc* 1+k ]
                     + jac[3* 1+2 ] * intp_shpg[Ng+ Nc* 2+k ];
      //};for(uint k=0; k<Nc; k++){
        A[ 3* 1+0 ] += G[Nc* 1+k ] * u[ndof* k+0 ];
        A[ 3* 1+1 ] += G[Nc* 1+k ] * u[ndof* k+1 ];
        A[ 3* 1+2 ] += G[Nc* 1+k ] * u[ndof* k+2 ];
      };for(uint k=0; k<Nc; k++){
        G[Nc* 2+k ]  = jac[3* 2+0 ] * intp_shpg[Ng+ Nc* 0+k ]
                     + jac[3* 2+1 ] * intp_shpg[Ng+ Nc* 1+k ]
                     + jac[3* 2+2 ] * intp_shpg[Ng+ Nc* 2+k ];
      //};for(uint k=0; k<Nc; k++){
        A[ 3* 2+0 ] += G[Nc* 2+k ] * u[ndof* k+0 ];
        A[ 3* 2+1 ] += G[Nc* 2+k ] * u[ndof* k+1 ];
        A[ 3* 2+2 ] += G[Nc* 2+k ] * u[ndof* k+2 ];
      };//};//};//------------------------------------------ N* 3*11 = 33*N FLOP
      #if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(uint j=0;j<HH.size();j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      }; printf("\n");
      #endif
      //det = jac[9];// Save for later
      //ij+=Nj;//FIXME Prefetch the next jac&det...
      //ij=Nj*ie*intp_n +Nj*(ip+1);
      //if(ij<nej){
      //  std::copy( &E->elip_jacs[ij],
      //             &E->elip_jacs[ij+Nj], jac );
      //};
      //for(int i=0; i<9; i++){ A[i]=0.0; };
      for(int i=0; i<3; i++){//for(uint k=0; k<3; k++){//for(uint j=0; j<3; j++){
        // [H][RT] : matmul3x3x3T
        //  A[3* i+k ]+= HH[3* i+j ] * mtrl_rotc[3* k+j ];
        B[3* i+0 ] = A[3*i+0]*R[3*0+0] + A[3*i+1]*R[3*0+1] + A[3*i+2]*R[3*0+2];
        //};for(uint i=0; i<3; i++){
        B[3* i+1 ] = A[3*i+0]*R[3*1+0] + A[3*i+1]*R[3*1+1] + A[3*i+2]*R[3*1+2];
        //};for(uint i=0; i<3; i++){
        B[3* i+2 ] = A[3*i+0]*R[3*2+0] + A[3*i+1]*R[3*2+1] + A[3*i+2]*R[3*2+2];
      };//};//};//---------------------------------------------- 3*3*5 = 45 FLOP
      FLOAT_PHYS w = det * wgt[ip];//0.25;
      A[0]=(mtrl_matc[0]* B[0] + mtrl_matc[3]* B[4] + mtrl_matc[5]* B[8])*w;//Sxx
      A[4]=(mtrl_matc[3]* B[0] + mtrl_matc[1]* B[4] + mtrl_matc[4]* B[8])*w;//Syy
      A[8]=(mtrl_matc[5]* B[0] + mtrl_matc[4]* B[4] + mtrl_matc[2]* B[8])*w;//Szz
      //
      A[1]=( B[1] + B[3])*mtrl_matc[6]*w; A[3]= A[1];//Sxy Syx
      A[5]=( B[5] + B[7])*mtrl_matc[7]*w; A[7]= A[5];//Syz Szy
      A[2]=( B[2] + B[6])*mtrl_matc[8]*w; A[6]= A[2];//Sxz Szx
      //--------------------------------------------------------- 18+9= 27 FLOP
      for(int i=0; i<3; i++){// [S][R] : matmul3x3x3, R is transposed
        B[3* i+0 ]= A[3*i+0]*R[3*0+0] + A[3*i+1]*R[3*1+0] + A[3*i+2]*R[3*2+0];
        B[3* i+1 ]= A[3*i+0]*R[3*0+1] + A[3*i+1]*R[3*1+1] + A[3*i+2]*R[3*2+1];
        B[3* i+2 ]= A[3*i+0]*R[3*0+2] + A[3*i+1]*R[3*1+2] + A[3*i+2]*R[3*2+2];
      };//------------------------------------------------------ 3*3*5 = 45 FLOP
      for(uint i=0; i<Nc; i++){//for(uint k=0; k<3 ; k++){//for(uint j=0; j<3 ; j++){
        //f[mesh_d* i+k ]+= G[Nc* j+i ] * B[mesh_d*j + k];
        f[3* i+0 ]+=G[Nc*0+i]* B[3*0+0] + G[Nc*1+i]* B[3*1+0] + G[Nc*2+i]* B[3*2+0];
      };for(uint i=0; i<Nc; i++){//NOTE Time this with and without these
        f[3* i+1 ]+=G[Nc*0+i]* B[3*0+1] + G[Nc*1+i]* B[3*1+1] + G[Nc*2+i]* B[3*2+1];
      };for(uint i=0; i<Nc; i++){//NOTE Time this with and without these
        f[3* i+2 ]+=G[Nc*0+i]* B[3*0+2] + G[Nc*1+i]* B[3*1+2] + G[Nc*2+i]* B[3*2+2];
      };//};//};//------------------------------------------- N* 3*6 = 18*N FLOP
      #if VERB_MAX>10
      printf( "ff:");
      for(uint j=0;j<Ne;j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      }; printf("\n");
      #endif
    };//end intp loop
    for (int i=0; i<int(Nc); i++){
      //int c=E->elem_conn[Nc*ie+i]*3;
      for(int j=0; j<3; j++){
        sys_f[E->elem_conn[Nc*ie+i]*3+j] += f[3*i+j];
      }; };
    //
    //std::memcpy( &elem_inout[ie*Ne], &f, sizeof(f) );
    //for (int i=0; i<int(Nc); i++){
    //  sys_f[std::slice(E->elem_conn[Nc*ie+i]*ndof,ndof,1)]
    //    += f[std::slice(i*ndof,ndof,1)]; }
    //
    //FIXME Write elem_f into a buffer to accumulate node_f
  };//end elem loop
  return 0;
  };
int ElastOrtho3D::ElemJacobi(Elem* E, RESTRICT Phys::vals &sys_d ){//FIXME Doesn't do rotation yet
  const int ndof   = 3;//this->ndof_n
  //const int mesh_d = E->elem_d;
  const int elem_n = E->elem_n;
  //const int intp_n = E->elip_dets.size()/elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = ndof*Nc;// printf("GAUSS PTS: %i\n",(int)E->gaus_n);
  const int intp_n = E->gaus_n;//E->elip_jacs.size()/elem_n/Nj;
  //
  FLOAT_PHYS det;
  RESTRICT Phys::vals elem_diag(Ne);//,  shg(Ne), G(Ne),jac(d2);
  //RESTRICT Phys::vals B(Ne*6);// 6 rows, Ne cols
  FLOAT_PHYS B[Ne*6];
  FLOAT_PHYS G[Ne],jac[Nj];//,elem_diag[Ne];// 6 rows, Ne cols
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; };
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[3],mtrl_matc[5],0.0,0.0,0.0,
    mtrl_matc[3],mtrl_matc[1],mtrl_matc[4],0.0,0.0,0.0,
    mtrl_matc[5],mtrl_matc[4],mtrl_matc[2],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[6],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[7],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[8]};
    // Does the following wrong one actually converge faster?
    //mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    //mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    //mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    //0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    //0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    //0.0,0.0,0.0,0.0,0.0,mtrl_matc[2]};
  //elem_inout=0.0;
  for(int ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;
    std::copy( &E->elip_jacs[ij],
               &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(int ip=0;ip<intp_n;ip++){
      //uint ij=Nj*ie*intp_n +Nj*ip;
      //std::copy( &E->elip_jacs[ij],
      //           &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
      //jac = E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,d2,1)];
      //det = E->elip_jacs[ie*intp_n*Nj+ip*Nj +d2];
      //shg = E->intp_shpg[std::slice(ip*Ne,Ne,1)];
      //G   = MatMul3x3xN(jac,shg);
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; };
      for(uint i=0;i<3;i++){ for(uint j=0;j<3;j++){ for(uint k=0;k<Nc;k++){
        G[3* i+k] += jac[3* i+j] * E->intp_shpg[ig+Nc* j+k]; }; }; };
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      }; printf(" det:%+9.2e\n",det);
      #endif
      // xx yy zz
      //B[std::slice(Ne*0 + 0,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
      //B[std::slice(Ne*1 + 1,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
      //B[std::slice(Ne*2 + 2,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
      for(uint j=0; j<Nc; j++){
        B[Ne*0 + 0+j*ndof] = G[Nc*0+j];
        B[Ne*1 + 1+j*ndof] = G[Nc*1+j];
        B[Ne*2 + 2+j*ndof] = G[Nc*2+j];
      // xy yx
      //B[std::slice(Ne*3 + 0,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
      //B[std::slice(Ne*3 + 1,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
        B[Ne*3 + 0+j*ndof] = G[Nc*1+j];
        B[Ne*3 + 1+j*ndof] = G[Nc*0+j];
      // yz zy
      //B[std::slice(Ne*4 + 1,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
      //B[std::slice(Ne*4 + 2,Nc,ndof)] = G[std::slice(Nc*1,Nc,1)];
        B[Ne*4 + 1+j*ndof] = G[Nc*2+j];
        B[Ne*4 + 2+j*ndof] = G[Nc*1+j];
      // xz zx
      //B[std::slice(Ne*5 + 0,Nc,ndof)] = G[std::slice(Nc*2,Nc,1)];
      //B[std::slice(Ne*5 + 2,Nc,ndof)] = G[std::slice(Nc*0,Nc,1)];
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
    for (int i=0; i<int(Nc); i++){
      //int c=E->elem_conn[Nc*ie+i]*3;
      for(int j=0; j<3; j++){
        sys_d[E->elem_conn[Nc*ie+i]*3+j] += elem_diag[3*i+j];
      }; };
    elem_diag=0.0;
  };
  return 0;
};
