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
int ElastPlastJ2Iso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10) ;
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n// Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  //
  this->elem_vars.resize(elem_n*intp_n* 3, 0.0 );
  return 0;
}
int ElastPlastJ2Iso3D::ElemNonLinear( Elem* E,
  const INT_MESH e0,const INT_MESH ee,
  FLOAT_SOLV* sys_f, const FLOAT_SOLV* sys_p, const FLOAT_SOLV* sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const int intp_n = int(E->gaus_n);
  //
  const FLOAT_PHYS youngs_modulus    =this->mtrl_prop[0];
  const FLOAT_PHYS poissons_ratio    =this->mtrl_prop[1];
  const FLOAT_PHYS yield_stress      =this->plas_prop[0];
  const FLOAT_PHYS saturation_stress =this->plas_prop[1];
  const FLOAT_PHYS hardness_modulus  =this->plas_prop[2];
  const FLOAT_PHYS j2_beta           =this->plas_prop[3];
  //
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_PHYS dw, G[Ne], u[Ne], p[Ne], f[Ne];
  FLOAT_PHYS H[Nd*Nf], S[Nd*Nf], P[Nd*Nf];
  //
  const FLOAT_PHYS* RESTRICT intp_shpg = &E->intp_shpg[0];
  const FLOAT_PHYS* RESTRICT       wgt = &E->gaus_weig[0];
  const FLOAT_PHYS* RESTRICT         C = &this->mtrl_matc[0];
#if VERB_MAX>11
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  } printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
  const FLOAT_SOLV* RESTRICT sysp  = &sys_p[0];
        FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  for(INT_MESH ie=e0;ie<ee;ie++){
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Nf); j++){
        u[Nf*i+j] = sysu[conn[i]*Nf+j];
        p[Nf*i+j] = sysp[conn[i]*Nf+j];
    } }
    for(int i=0; i<Ne; i++){ f[i]=0.0; }
    for(int ip=0; ip<intp_n; ip++){
      for(int i=0; i<Ne ; i++){ G[i]=0.0; }
      for(int i=0; i< 9 ; i++){ H[i]=0.0; }
      for(int i=0; i< 9 ; i++){ P[i]=0.0; }
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
            P[Nf* k+j ] += G[Nf* i+k ] * p[Nf* i+j ];
      } } }//---------------------------------------------- N*3*6*2 = 36*N FLOP
#if VERB_MAX>10
      printf( "Small Strains (Elem: %i):", ie );
      for(int j=0;j<H.size();j++){
        if(j%mesh_d==0){printf("\n"); }
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      {// Scope D-matrix and local copy of integration point state variables.
      FLOAT_PHYS D[36]={// Initialize to linear-elastic isotropic.
        C[0],C[1],C[1],0.0 ,0.0 ,0.0,
        C[1],C[0],C[1],0.0 ,0.0 ,0.0,
        C[1],C[1],C[0],0.0 ,0.0 ,0.0,
        0.0 ,0.0 ,0.0 ,C[2],0.0 ,0.0,
        0.0 ,0.0 ,0.0 ,0.0 ,C[2],0.0,
        0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,C[2]
      };
      FLOAT_PHYS plastic_shear_strain[3];
      for(int i=0; i<3; i++){// Make local copy of element state.
        plastic_shear_strain[ i ] = this->elem_vars[3*(intp_n*ie+ip) +i ]; }
      {//====================================================== Scope UMAT calc
      FLOAT_PHYS stress_v[6];// sxx, syy, szz,  sxy, syz, sxz
      FLOAT_PHYS strain_v[6] // exx, eyy, ezz,  exy, eyz, exz
        ={ H[0], H[4], H[8],  H[1]+H[3], H[5]+H[7], H[2]+H[6] };
      //
      // Do some calculations.
      //
      // Calculate stress from strain.
      for(int i=0; i<6; i++){ stress_v[i]=0.0;
        for(int j=0; j<6; j++){
          stress_v[ i ] += D[6* i+j ] * strain_v[ j ];
      } }
      //
      // Do some more calculations.
      //
//------------------------------------------------------------ Debugging output
#if VERB_MAX>10
#pragma omp critical(print)
{
      if( (ie==0) & (ip==0) ){
        // Local copies of material variables are set before the element loop.
        printf("    Young's modulus :%9.2e\n", youngs_modulus    );
        printf("    Poisson's ratio :%9.2e\n", poissons_ratio    );
        printf("       Yield Stress :%9.2e\n", yield_stress      );
        printf("  Saturation Stress :%9.2e\n", saturation_stress );
        printf("   Hardness modulus :%9.2e\n", hardness_modulus  );
        printf(" J2 Plasticity Beta :%9.2e\n", j2_beta           );
      }
}
#endif
#if VERB_MAX>10
#pragma omp critical(print)
{
      printf("el:%u,gp:%i Strain: ",ie,ip);
      for(int i=0; i<6; i++){ printf("%+9.2e ", strain_v[i] ); }
      printf("\n");
}
#endif
#if VERB_MAX>10
#pragma omp critical(print)
{
      printf("el:%u,gp:%i Stress: ",ie,ip);
      for(int i=0; i<6; i++){ printf("%+9.2e ", stress_v[i] ); }
      printf("\n");
}
#endif
      }//======================================================= end UMAT scope
#if VERB_MAX>10
#pragma omp critical(print)
{
      printf("el:%u,gp:%i D:     ",ie,ip);
      for(int i=0; i<36; i++){
        if(!(i%6)&(i>0)){ printf("                 "); }
        printf(" %+9.2e", D[i] );
        if((i%6)==5){ printf("\n"); }
      }
}
#endif
#if VERB_MAX>10
#pragma omp critical(print)
{
      printf("el:%u,gp:%i Plastic Shear Strain:                 ",ie,ip);
      for(int i=0; i<3; i++){ printf("%+9.2e ", plastic_shear_strain[i] ); }
      printf("\n");
}
#endif
//-------------------------------------------------------- End debugging output
      // Save element state.
      for(int i=0; i<3; i++){
        this->elem_vars[3*(intp_n*ie+ip) +i ] = plastic_shear_strain[ i ];
      }
      // Calculate conjugate stress from conjugate strain.
      const FLOAT_PHYS strain_p[6]={ P[0], P[4], P[8],
        P[1]+P[3], P[5]+P[7], P[2]+P[6] };
      FLOAT_PHYS stress_p[6];
      for(int i=0; i<6; i++){ stress_p[i] =0.0;
        for(int j=0; j<6; j++){
          stress_p[i] += D[6* i+j ] * strain_p[ j ];
      } }
      // Convert conjugate stress Voigt vector to conjugate stress tensor.
      S[0]=stress_p[0]; S[4]=stress_p[1]; S[8]=stress_p[2];
      S[1]=stress_p[3]; S[5]=stress_p[4]; S[2]=stress_p[5];
      S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
      }// End D-matrix and local state varible scope.
      // Accumulate elemental nodal forces.
      dw = Ejacs[Nj*ie+ 9] * wgt[ip];
      for(int i=0; i<Nc; i++){
        for(int k=0; k<Nf; k++){
          for(int j=0; j<Nf; j++){
            f[Nf* i+k ] += G[Nf* i+j ] * S[Nf* j+k ] * dw;
      } } }//------------------------------------------------- N*3*6 = 18*N FMA
#if VERB_MAX>10
      printf( "f:");
      for(int j=0;j<Ne;j++){
        if(j%ndof==0){printf("\n"); }
        printf("%+9.2e ",f[j]);
      } printf("\n");
#endif
    }// End intp loop.
    for (uint i=0; i<uint(Nc); i++){
      for (uint j=0; j<uint(Nf); j++){
        sysf[conn[i]*Nf+j] += f[Nf*i+j];
    } }//------------------------------------------------------------- 3*n FLOP
  }// Rnd elem loop.
  return 0;
  }