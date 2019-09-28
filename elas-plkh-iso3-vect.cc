#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>

// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
//
int ElastPlastJ2Iso3D::Setup( Elem* E ){
  JacT  ( E );
  const uint elem_n = uint(E->elem_n);
  const uint jacs_n = uint(E->elip_jacs.size()/elem_n/ 10);
  const uint intp_n = uint(E->gaus_n);
  const uint conn_n = uint(E->elem_conn_n);
  this->tens_flop = uint(E->elem_n) * intp_n
    *( uint(E->elem_conn_n)* (54) + 27 );
  this->tens_band = elem_n *(
     sizeof(FLOAT_SOLV)*(3*conn_n*3+ jacs_n*10)// Main mem
    +sizeof(INT_MESH)*conn_n // Main mem ints
    +sizeof(FLOAT_PHYS)*(3*intp_n*conn_n +3+1 ) );// Stack (assumes read once)
  this->stif_flop = uint(E->elem_n)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) * sizeof(FLOAT_PHYS)
    * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) +2);
  //
  this->gvar_d = 1*6;
  this->elgp_vars.resize(elem_n*intp_n* this->gvar_d, 0.0 );
  return 0;
}
int ElastPlastJ2Iso3D::ElemNonlinear( Elem* E,
  const INT_MESH e0,const INT_MESH ee,
  FLOAT_SOLV* sys_f, const FLOAT_SOLV* sys_p, const FLOAT_SOLV* sys_u ){
  //FIXME Clean up local variables.
  //const int De = 3;// Element Dimension
  const int Nd = 3;// Node (mesh) Dimension
  const int Nf = 3;// this->node_d DOF/node
  const int Nj = Nd*Nd+1;
  const int Nc = E->elem_conn_n;// Number of nodes/element
  const int Ne = Nf*Nc;
  const int Nt = 4*Nc;
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
  //
  const FLOAT_PHYS youn_modu    = this->mtrl_prop[0];
  const FLOAT_PHYS poiss_ratio  = this->mtrl_prop[1];
  const FLOAT_PHYS stress_yield = this->plas_prop[0];
  const FLOAT_PHYS hard_modu    = this->plas_prop[1];
  const FLOAT_PHYS bulk_modu    = youn_modu / (1.0-2.0*poiss_ratio);
  const FLOAT_PHYS shear_modu   = 2.0*mtrl_matc[2];
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_PHYS dw;
  FLOAT_MESH __attribute__((aligned(32))) jac[Nj];
  FLOAT_PHYS __attribute__((aligned(32))) G[Nt], u[Ne], p[Ne];
  FLOAT_PHYS __attribute__((aligned(32))) H[Nd*4],P[Nd*4], S[Nd*4];//FIXME S size
  //
  FLOAT_PHYS __attribute__((aligned(32))) intp_shpg[intp_n*Ne];
  FLOAT_PHYS __attribute__((aligned(32))) wgt[intp_n];
  FLOAT_PHYS __attribute__((aligned(32))) C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C );
  const __m256d c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  const __m256d c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
  const FLOAT_SOLV* RESTRICT sysp  = &sys_p[0];
  {// Scope vf registers
  __m256d vf[Nc];
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i],&sysu[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf );
      std::memcpy( & p[Nf*i],&sysp[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    const __m256d j0 = _mm256_load_pd (&jac[0]);  // j0 = [j3 j2 j1 j0]
    const __m256d j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    const __m256d j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
      FLOAT_PHYS alpha[6];//FIXME Consider prefetching...
      for(int i=0; i<6; i++){// Copy initial element state.
        alpha[ i ] = this->elgp_vars[this->gvar_d*(intp_n*ie+ip) +i ]; }
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      compute_g_p_h( &G[0],&P[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne],
                     &p[0], &u[0] );
#if VERB_MAX>10
      printf( "Small Strains Transposed (Elem: %i):", ie );
      for(int j=0;j<12;j++){
        if(j%4==0){printf("\n"); }
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifndef FETCH_U_EARLY
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf );
          std::memcpy(& p[Nf*i],& sysp[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef FETCH_JAC
          std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
      compute_iso_s( &S[0], &H[0],C[2],c0,c1,c2, 1.0 );// Linear stress response
#if VERB_MAX>10
      printf( "Cauchy Stress (Elem: %i):", ie );
      for(int j=0;j<12;j++){
        if(j%4==0){printf("\n"); }
        printf("%+9.2e ",S[j]);
      } printf("\n");
#endif
      const FLOAT_PHYS stress_v[6]={// sxx, syy, szz,  sxy, syz, sxz
        S[0], S[5], S[10], S[1], S[6], S[2] };
#if VERB_MAX>10
      printf( "Stress Voigt Vector (Elem: %i):\n", ie );
      for(int j=0;j<6;j++){
        printf("%+9.2e ",stress_v[j]);
      } printf("\n");
#endif
      //====================================================== Scope UMAT calc
      FLOAT_PHYS stress_mises=0.0;
      {
      FLOAT_PHYS m[3];//FIXME Loop and vectorize
      m[0] = stress_v[0] - alpha[0] - stress_v[1] + alpha[1];
      m[1] = stress_v[1] - alpha[1] - stress_v[2] + alpha[2];
      m[2] = stress_v[2] - alpha[2] - stress_v[0] + alpha[0];
      for(int i=0;i<3;i++){ stress_mises += m[i]*m[i]; }
      }
      for(int i=3;i<6;i++){
        const FLOAT_PHYS m = stress_v[i] - alpha[i];
        stress_mises+= 6.0*m*m;
      }
      __m256d s[3];
      if( stress_mises > (stress_yield*stress_yield) ){
        stress_mises = std::sqrt(stress_mises);
        const FLOAT_PHYS delta_equiv = ( stress_mises - stress_yield )
          / ( 1.5*shear_modu + hard_modu );
        const FLOAT_PHYS inv_mises = 1.0/stress_mises;
        const FLOAT_PHYS shear_eff
          = shear_modu * (stress_yield + hard_modu*delta_equiv) * inv_mises;
        const FLOAT_PHYS third = 0.333333333333333;
        const FLOAT_PHYS hard_eff = shear_modu * hard_modu
          / (shear_modu + hard_modu*third) -3.0*shear_eff;
        const FLOAT_PHYS lambda_eff = (bulk_modu - 2.0*shear_eff)*third;
        FLOAT_PHYS plas_flow[6];
        {
        FLOAT_PHYS stress_hydro=0.0;
        for(int i=0;i<3;i++){ stress_hydro+= stress_v[i]*third; }
        for(int i=0;i<6;i++){ plas_flow[i] = stress_v[i]-alpha[i]; }
        for(int i=0;i<3;i++){ plas_flow[i]-= stress_hydro; }
        for(int i=0;i<6;i++){ plas_flow[i]*= inv_mises; }
        }
        FLOAT_PHYS D[36]={// Initialize to linear-elastic isotropic.
          C[0],C[1],C[1],0.0 ,0.0 ,0.0,
          C[1],C[0],C[1],0.0 ,0.0 ,0.0,
          C[1],C[1],C[0],0.0 ,0.0 ,0.0,
          0.0 ,0.0 ,0.0 ,C[2]*1.0,0.0 ,0.0,
          0.0 ,0.0 ,0.0 ,0.0 ,C[2]*1.0,0.0,
          0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,C[2]*1.0
        };
        for(int i=0;i<3;i++){
          for(int j=0;j<3;j++){
            D[6* i+j ] = lambda_eff;
          }
          D[6* i+i ]+= 2.0*shear_eff;
        }
        for(int i=3;i<6;i++){
          D[6* i+i ] = shear_eff;
        }
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            D[6* i+j ]+= hard_eff * plas_flow[i] * plas_flow[j];
          }
        }
        //------------------------------------------------- Save element state.
        // Update state variable alpha.
        for(int i=0; i<6; i++){
          this->elgp_vars[gvar_d*(intp_n*ie+ip) +i ]
            += hard_modu * plas_flow[i] * delta_equiv;
        }
        //======================================================= end UMAT scope
        // Calculate conjugate stress from conjugate strain.
        const FLOAT_PHYS strain_p[6]={
          P[0], P[5], P[10], P[1]+P[4], P[6]+P[9], P[2]+P[8] };
        FLOAT_PHYS stress_p[6];
        for(int i=0; i<6; i++){ stress_p[i] =0.0;
          for(int j=0; j<6; j++){
            stress_p[i] += D[6* i+j ] * strain_p[ j ];
            }
          stress_p[i]*= dw;
        }
        S[0]=stress_p[0]; S[1]=stress_p[3]; S[2]=stress_p[5]; S[3]=stress_p[3];
        S[4]=stress_p[3]; S[5]=stress_p[1]; S[6]=stress_p[4]; S[7]=stress_p[5];
        S[8]=stress_p[5]; S[9]=stress_p[4]; S[10]=stress_p[2];S[11]=0.0;
        s[0] = _mm256_load_pd(&S[0]);
        s[1] = _mm256_load_pd(&S[4]);
        s[2] = _mm256_load_pd(&S[8]);
      }// if plastic
      else{
        compute_iso_s( &S[0], &P[0],C[2],c0,c1,c2, dw );// Linear conj response
        s[0] = _mm256_load_pd(&S[0]);
        s[1] = _mm256_load_pd(&S[4]);
        s[2] = _mm256_load_pd(&S[8]);
      }
      if(ip==0){
        for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
        if(elem_p>1){
          for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
        if(elem_p>2){
          for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[i]]); }
        }
        }
      }
      accumulate_f( &vf[0], &s[0], &G[0], elem_p );
      }//======================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for(int j=0; j<3; j++){
          double __attribute__((aligned(32))) sf[4];
          _mm256_store_pd(&sf[0],vf[i]);
          sys_f[3*conn[i]+j] = sf[j]; } }
  }//============================================================ end elem loop
  }// end vf register scope
  return 0;
}
#undef VECTORIZED
