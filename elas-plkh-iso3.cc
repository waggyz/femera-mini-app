#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
int ElastPlastKHIso3D::ElemLinear( Elem* ){ return 1; }//FIXME
int ElastPlastKHIso3D::ElemJacobi( Elem* ){ return 1; }//FIXME
int ElastPlastKHIso3D::BlocLinear( Elem*,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){
  return 1;
  }
int ElastPlastKHIso3D::ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*){
  return 1;
}
int ElastPlastKHIso3D::ElemStiff(Elem* E  ){
  //FIXME Doesn't do rotation yet
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
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2] };
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    if(j%Dm==0){printf("\n"); }
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
        if(j%3==0){printf("\n"); }
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
      for(uint l=0; l<Ne; l++){ elem_stiff[Nk*ie +Nr* i+l ] = 0.0;
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        elem_stiff[Nk*ie +Nr* i+l ]+=B[Ne* i+j ] * D[6* k+j ] * B[Ne* k+l ] * w;
      } } } }
    }// end intp loop
  }// End elem loop
  return 0;
}//============================================================== End ElemStiff
#if 1
int ElastPlastKHIso3D::ElemJacobi(Elem*, FLOAT_SOLV* ){ return 0; }
#else
int ElastPlastKHIso3D::ElemJacobi(Elem* E, FLOAT_SOLV* part_d ){
  //printf("**** Preconditioner 1 ****\n");
  const uint ndof   = 3;//this->node_d
  const uint  Nj = 10,d2=9;
  const uint  Nc = E->elem_conn_n;
  const uint  Ne = uint(ndof*Nc);
  const uint elem_n = E->elem_n;
  const uint intp_n = E->gaus_n;
  //
  FLOAT_PHYS det;
  RESTRICT Phys::vals elem_diag(Ne);
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];//,elem_diag[Ne];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2] };
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij], &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
        for(uint i=0;i< 3;i++){
          for(uint j=0;j< 3;j++){
            G[Nc* i+k] += jac[3* j+i] * E->intp_shpg[ig+3* k+j]; } } }
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n"); }
        printf("%+9.2e",jac[j]);
      } printf(" det:%+9.2e\n",det);
      #endif
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
      for(uint j=0; j<6 ; j++){
        elem_diag[i]+=(B[Ne*j + i] * D[6*j + k] * B[Ne*k + i])*w;
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
#endif
#define COMPRESS_STATE
#define PRECOND_TANGENT
int ElastPlastKHIso3D::ElemJacobi( Elem* E,
  FLOAT_SOLV* part_d, const FLOAT_SOLV* part_u ){
  //printf("**** Preconditioner 2 ****\n");
  const uint ndof   = 3;//this->node_d
  const uint  Nj = 10,d2=9;
  const uint  Nc = E->elem_conn_n;
  const uint  Ne = uint(ndof*Nc);
  const uint elem_n = E->elem_n;
  const uint intp_n = E->gaus_n;
  //
#ifdef COMPRESS_STATE
  const int        Ns           = 5;// Number of state variables/ip
#else
  const int        Ns           = 6;
#endif
  const FLOAT_PHYS youn_modu    = this->mtrl_prop[0];
  const FLOAT_PHYS poiss_ratio  = this->mtrl_prop[1];
  const FLOAT_PHYS bulk_mod3    = youn_modu / (1.0-2.0*poiss_ratio);
#if 0
  const FLOAT_PHYS shear_modu   = 0.5*youn_modu/(1.0+poiss_ratio);
#else
  const FLOAT_PHYS shear_modu   = this->mtrl_matc[2];
#endif
  const FLOAT_PHYS stress_yield = this->plas_prop[0];
  const FLOAT_PHYS hard_modu    = this->plas_prop[1];
  const FLOAT_PHYS yield_tol2   =
    (stress_yield*(1.0+1e-6))*(stress_yield*(1.0+1e-6));
  //
  //FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
#ifdef COMPRESS_STATE
  FLOAT_PHYS VECALIGNED back_v[6];
#else
  FLOAT_PHYS VECALIGNED back_v[8];// back_v padded to 8 doubles?
#endif
  //
  //std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  //
  //const FLOAT_SOLV* RESTRICT C     = &matc[0];
        FLOAT_SOLV* RESTRICT state = &this->elgp_vars[0];
  //
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2] };
  FLOAT_PHYS VECALIGNED Dpl[36];
  FLOAT_PHYS det;
  RESTRICT Phys::vals elem_diag(Ne);
  FLOAT_PHYS B[Ne*6];// 6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];//,elem_diag[Ne];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij], &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint ip=0;ip<intp_n;ip++){
#ifdef COMPRESS_STATE
      std::memcpy( &back_v[1], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#else
      std::memcpy( &back_v, &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#endif
      FLOAT_PHYS elas_part=1.0;
      for(int i=0; i<36; i++){ Dpl[i]=0.0; }
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
        for(uint i=0;i< 3;i++){
          for(uint j=0;j< 3;j++){
            G[Nc* i+k] += jac[3* j+i] * E->intp_shpg[ig+3* k+j]; } } }
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n"); }
        printf("%+9.2e",jac[j]);
      } printf(" det:%+9.2e\n",det);
      #endif
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
#if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n"); }
        printf("%+9.2e ",B[j]);
      } printf("\n");
#endif
      FLOAT_PHYS elas_strain[6],elas_devi_v[6];
      const FLOAT_PHYS one_third = 0.333333333333333;
      //NOTE back_v, elas_devi_v, and plas_flow are deviatoric (trace zero),
      //     with only 5 independent terms.
      //NOTE back_v is only deviatoric,
      //     so only elas_v hydro needs to be removed.
      for(uint i=0;i<6;i++){ elas_strain[ i ]=0.0;
        for(uint j=0;j<Ne;j++){
          elas_strain[ i ]+= B[Ne* i+j ] * part_u[ j ];
        } }
      for(uint i=0;i<6;i++){ elas_devi_v[ i ]=0.0;
        for(uint j=0;j<6;j++){
          elas_devi_v[ i ]+= D[6* i+j ] * elas_strain[ j ];
        } }
      {
      FLOAT_PHYS elas_hydr=0.0;
      for(int i=0;i<3;i++){ elas_hydr+= elas_devi_v[i]*one_third; }
      for(int i=0;i<3;i++){ elas_devi_v[i]-= elas_hydr; }
      }
      FLOAT_PHYS stress_mises=0.0;
      FLOAT_PHYS VECALIGNED plas_flow[6];
#ifdef COMPRESS_STATE
      back_v[0] = -back_v[1]-back_v[2];
#endif
      {
      for(int i=0;i<6;i++){ plas_flow[i] = elas_devi_v[i] - back_v[i]; }
      for(int i=0;i<3;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*1.5; }
      for(int i=3;i<6;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*3.0; }
      }
      //for(int i=0;i<6;i++){ printf("%+9.2e ", elas_v[i]); }
      //printf("Mises: %+9.2e\n",sqrt(stress_mises));
      //printf("*** Nonlinear Preconditioner ***\n");
      if( stress_mises > yield_tol2 ){//---------------------------------------
        //printf("*** Preconditioner Plastic ***\n");
        stress_mises = sqrt(stress_mises);
        const FLOAT_PHYS inv_mises = 1.0/stress_mises;
#if 0
        const FLOAT_PHYS delta_equiv = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu );
        // delta_equiv is always used as delta_equiv * hard_modu
#endif
        const FLOAT_PHYS delta_hard = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu ) * hard_modu;
        const FLOAT_PHYS shear_eff
          = shear_modu * (stress_yield + delta_hard) * inv_mises;
        const FLOAT_PHYS hard_eff = shear_modu * hard_modu
          / (shear_modu + hard_modu*one_third) - 3.0*shear_eff;
        const FLOAT_PHYS lambda_eff = (bulk_mod3 - 2.0*shear_eff)*one_third;
        for(int i=0;i<6;i++){ plas_flow[i]*= inv_mises; }
        FLOAT_PHYS elas_mises=0.0;
        {
        for(int i=0;i<3;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*1.5; }
        for(int i=3;i<6;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*3.0; }
        }
        elas_mises = sqrt(elas_mises);
#ifdef PRECOND_TANGENT
        elas_part=0.0;
#else
        elas_part = stress_yield / elas_mises;
#endif
        // Secant modulus response: this stress plus elastic response at yield
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            Dpl[6* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
          Dpl[6* i+i ]+= shear_eff;
        }
        for(int i=0;i<3;i++){
          Dpl[6* i+i ]+= shear_eff;// + (C[0]-C[1])*elas_part;
          //Dpl[6* (i+3)+i+3 ]+= C[2]*elas_part;
          for(int j=0;j<3;j++){
            Dpl[6* i+j ]+= lambda_eff;// + C[1]*elas_part;
          }
        }
      }//end if plastic -------------------------------------------------------
      const FLOAT_PHYS w = det * E->gaus_weig[ip];
#ifdef PRECOND_TANGENT
      if( elas_part > 0.0 ){// it's actually 1.0
        for(uint i=0; i<Ne; i++){
        for(uint k=0; k<6 ; k++){
        for(uint j=0; j<6 ; j++){
          elem_diag[i]+=( B[Ne* j+i] * D[6* j+k] * B[Ne* k+i] )*w;
      } } }
      }else{// elas_part==0.0
        for(uint i=0; i<Ne; i++){
        for(uint k=0; k<6 ; k++){
        for(uint j=0; j<6 ; j++){
          elem_diag[i]+=( B[Ne* j+i] * Dpl[6* j+k] * B[Ne* k+i] )*w;
        } } }
      }
#else
      for(uint i=0; i<Ne; i++){
      for(uint k=0; k<6 ; k++){
      for(uint j=0; j<6 ; j++){
        elem_diag[i]+=( B[Ne* j+i] * ( elas_part*D[6* j+k] + Dpl[6* j+k] )
          * B[Ne* k+i] )*w;
      } } }
#endif
    }//end intp loop
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        part_d[E->elem_conn[Nc*ie+i]*3+j] += elem_diag[3*i+j];
      } }
    elem_diag=0.0;
  }
  return 0;
}
int ElastPlastKHIso3D::ElemRowSumAbs(Elem* E, FLOAT_SOLV* part_d ){
  const uint ndof   = 3;//this->node_d
  const uint elem_n = E->elem_n;
  const uint  Nc = E->elem_conn_n;
  const uint  Nj = 10,d2=9;
  const uint  Ne = uint(ndof*Nc);
  const uint intp_n = E->gaus_n;
  //
  FLOAT_PHYS det;
  RESTRICT Phys::vals elem_sum(Ne);
  RESTRICT Phys::vals K(Ne*Ne);
  FLOAT_PHYS B[Ne*6];//6 rows, Ne cols
  FLOAT_PHYS G[Ne],jac[Nj];
  for(uint j=0; j<(Ne*6); j++){ B[j]=0.0; }
  const FLOAT_PHYS D[]={
    mtrl_matc[0],mtrl_matc[1],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[0],mtrl_matc[1],0.0,0.0,0.0,
    mtrl_matc[1],mtrl_matc[1],mtrl_matc[0],0.0,0.0,0.0,
    0.0,0.0,0.0,mtrl_matc[2],0.0,0.0,
    0.0,0.0,0.0,0.0,mtrl_matc[2],0.0,
    0.0,0.0,0.0,0.0,0.0,mtrl_matc[2]};
  for(uint ie=0;ie<elem_n;ie++){
    uint ij=Nj*ie;//FIXME only good for tets
    std::copy( &E->elip_jacs[ij],
               &E->elip_jacs[ij+Nj], jac ); det=jac[d2];
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<3;i++){
      for(uint j=0;j<3;j++){
        G[3* i+k] += jac[3* j+i] * E->intp_shpg[ig+3* k+j]; } } }
      #if VERB_MAX>10
      printf( "Jacobian Inverse & Determinant:");
      for(uint j=0;j<d2;j++){
        if(j%3==0){printf("\n");}
        printf("%+9.2e",jac[j]);
      } printf(" det:%+9.2e\n",det);
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
      }
      #if VERB_MAX>10
      printf( "[B]:");
      for(uint j=0;j<B.size();j++){
        if(j%Ne==0){printf("\n");}
        printf("%+9.2e ",B[j]);
      } printf("\n");
      #endif
      FLOAT_PHYS w = det * E->gaus_weig[ip];
      for(uint i=0; i<Ne; i++){
      for(uint l=0; l<Ne; l++){
      for(uint j=0; j<6 ; j++){
      for(uint k=0; k<6 ; k++){
        K[Ne*i+l]+= B[Ne*j + i] * D[6*j + k] * B[Ne*k + l]*w;
      } } } }
    } //end intp loop
    for (uint i=0; i<Ne; i++){
      for(uint j=0; j<Ne; j++){
        elem_sum[i] += std::abs(K[Ne*i+j]);
      } }
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        part_d[E->elem_conn[Nc*ie+i]*3+j] += elem_sum[3*i+j];
      } }
    K=0.0; elem_sum=0.0;
  }
  return 0;
}
int ElastPlastKHIso3D::ElemStrain( Elem* E,FLOAT_SOLV* part_f ){
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
  FLOAT_PHYS H[9], S[9];
  //
  for(uint i=0; i< 9 ; i++){ H[i]=0.0; }
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
  const auto Econn = &E->elem_conn[0];
  const auto Ejacs = &E->elip_jacs[0];
  //
  for(INT_MESH ie=0;ie<elem_n;ie++){
    std::memcpy( &conn, &Econn[Nc*ie], sizeof(  INT_MESH)*Nc);
    std::memcpy( &jac , &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
    //
    for(uint i=0;i<(Ne);i++){ f[i]=0.0; }
    for(uint ip=0; ip<intp_n; ip++){
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      for(uint k=0; k<Nc; k++){
        for(uint i=0; i<3 ; i++){ G[3* k+i ]=0.0;
          for(uint j=0; j<3 ; j++){
            G[(3* k+i) ] += jac[3* j+i ] * intp_shpg[ip*Ne+ 3* k+j ];
          }
        }
      }//-------------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(uint j=0;j<9;j++){
        if(j%mesh_d==0){printf("\n");}
        printf("%+9.2e ",H[j]);
      } printf("\n");
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
      //-------------------------------------------------------- 18+9 = 27 FLOP
      for(uint i=0; i<Nc; i++){
        for(uint k=0; k<3; k++){
          for(uint j=0; j<3; j++){
            f[(3* i+k) ] += G[(3* i+j) ] * S[(3* k+j) ];
      } } }//------------------------------------------------ N*3*6 = 18*N FLOP
#if VERB_MAX>10
      printf( "f:");
      for(uint j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n");}
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }//end intp loop
    for (uint i=0; i<Nc; i++){
      for(uint j=0; j<3; j++){
        //part_f[3*conn[i]+j] += f[(3*i+j)];
        part_f[4*conn[i]+j] += std::abs( f[(3*i+j)] );
    } }//--------------------------------------------------- N*3 =  3*N FLOP
  }//end elem loop
  return 0;
  }
