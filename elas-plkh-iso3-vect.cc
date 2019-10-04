#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
#undef PREFETCH_STATE
//
int ElastPlastKHIso3D::Setup( Elem* E ){
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
  this->gvar_d = 1*6-1;
  this->elgp_vars.resize(elem_n*intp_n* this->gvar_d, 0.0 );
  return 0;
}
int ElastPlastKHIso3D::ElemNonlinear( Elem* E,
  const INT_MESH e0,const INT_MESH ee, FLOAT_SOLV* RESTRICT part_f,
  const FLOAT_SOLV* RESTRICT part_p,
  const FLOAT_SOLV* RESTRICT part_u, bool save_state ){
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
  const int        Ns           = this->gvar_d;// Number of state variables/ip
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
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne], p[Ne];
  FLOAT_PHYS VECALIGNED H[Nd*4],P[Nd*4], S[Nd*4];//FIXME S size
  //
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
  FLOAT_PHYS VECALIGNED back_v[Ns+3], bac5_v[Ns];// back_v padded to 8 doubles
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  //std::copy( &this->elgp_vars[0], 6, back_v );
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT C     = &matc[0];
        FLOAT_SOLV* RESTRICT state = &this->elgp_vars[0];
  const __m256d c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  const __m256d c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
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
      std::memcpy( & u[Nf*i],&part_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf );
      std::memcpy( & p[Nf*i],&part_p[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
#ifdef PREFETCH_STATE
    std::memcpy(&bac5_v, &state[Ns*(intp_n*e0+0)], sizeof(FLOAT_PHYS)*Ns);
#endif
  }
  for(INT_MESH ie=e0;ie<ee;ie++){//================================== Elem loop
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
    const INT_MESH* RESTRICT conn = &Econn[Nc*ie];
    const __m256d j0 = _mm256_load_pd (&jac[0]);  // j0 = [j3 j2 j1 j0]
    const __m256d j1 = _mm256_loadu_pd(&jac[3]);  // j1 = [j6 j5 j4 j3]
    const __m256d j2 = _mm256_loadu_pd(&jac[6]);  // j2 = [j9 j8 j7 j6]
  {// Scope vf registers
  __m256d vf[Nc];
    for(int ip=0; ip<intp_n; ip++){//============================== Int pt loop
#ifndef PREFETCH_STATE
      std::memcpy( &bac5_v, &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#endif
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      compute_g_p_h( &G[0],&P[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne],
                     &p[0],&u[0] );
#if VERB_MAX>10
      printf( "Small Strains Transposed (Elem: %i):", ie );
      for(int j=0;j<12;j++){
        if(j%4==0){printf("\n"); }
        printf("%+9.2e ",H[j]);
      } printf("\n");
#endif
      const FLOAT_PHYS dw = jac[9] * wgt[ip];
      if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifndef FETCH_U_EARLY
        const INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#else
//#pragma omp simd
#endif
        for (int i=0; i<Nc; i++){
          std::memcpy(& u[Nf*i],& part_u[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf );
          std::memcpy(& p[Nf*i],& part_p[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef FETCH_JAC
          std::memcpy(& jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
      compute_iso_s( &S[0], &H[0],C[2],c0,c1,c2, 1.0 );// Linear stress response
      //NOTE Only the deviatoric part of elas_v is used.
      FLOAT_PHYS VECALIGNED elas_devi_v[6]={
        S[0], S[5], S[10], S[1], S[6], S[2] };// sxx, syy, szz,  sxy, syz, sxz
#if VERB_MAX>10
      printf( "Stress Voigt Vector (Elem: %i):\n", ie );
      for(int j=0;j<6;j++){
        printf("%+9.2e ",elas_devi_v[j]);
      } printf("\n");
#endif
      //====================================================== UMAT calc
        const FLOAT_PHYS one_third = 0.333333333333333;
      //NOTE back_v, elas_devi_v, and plas_flow are deviatoric (trace zero),
      // with only 5 independent terms.
      //NOTE back_v is only deviatoric,
      //     so only elas_v hydro needs to be removed.
      {
      FLOAT_PHYS elas_hydr=0.0;
      for(int i=0;i<3;i++){ elas_hydr+= elas_devi_v[i]*one_third; }
      for(int i=0;i<3;i++){ elas_devi_v[i]-= elas_hydr; }
      }
      FLOAT_PHYS stress_mises=0.0;
      FLOAT_PHYS VECALIGNED plas_flow[6];
      for(int i=0; i<5; i++){ back_v[i+1]=bac5_v[i]; }
      back_v[0] = 0.0-bac5_v[0]-bac5_v[1];
#ifdef PREFETCH_STATE
      if( ((ip+1)<intp_n) | ((ie+1)<ee) ){
        std::memcpy(&bac5_v, &state[Ns*(intp_n*ie+ip+1)], sizeof(FLOAT_PHYS)*Ns);
      }
#endif
      {
      for(int i=0;i<6;i++){ plas_flow[i] = elas_devi_v[i] - back_v[i]; }
      for(int i=0;i<3;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*1.5; }
      for(int i=3;i<6;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*3.0; }
      }
      //for(int i=0;i<6;i++){ printf("%+9.2e ", elas_v[i]); }
      //printf("mises: %+9.2e\n",std::sqrt(stress_mises));
      __m256d s[3];
      if( stress_mises > yield_tol2 ){
        stress_mises = std::sqrt(stress_mises);
        const FLOAT_PHYS delta_equiv = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu );
        const FLOAT_PHYS inv_mises = 1.0/stress_mises;
        const FLOAT_PHYS shear_eff
          = shear_modu * (stress_yield + hard_modu*delta_equiv) * inv_mises;
        const FLOAT_PHYS hard_eff = shear_modu * hard_modu
          / (shear_modu + hard_modu*one_third) - 3.0*shear_eff;
        const FLOAT_PHYS lambda_eff = (bulk_mod3 - 2.0*shear_eff)*one_third;
        for(int i=0;i<6;i++){ plas_flow[i]*= inv_mises; }
        if( save_state ){// Update state variable back_v.
#if 1
          for(int i=0; i<5; i++){
            back_v[i] += plas_flow[i+1] * hard_modu * delta_equiv; }
          std::memcpy(&state[Ns*(intp_n*ie+ip)],&back_v,sizeof(FLOAT_SOLV)*Ns);
#else
          for(int i=0; i<6; i++){
            state[Ns*(intp_n*ie+ip) +i ]
              += plas_flow[i] * hard_modu * delta_equiv; }
#endif
        }
        FLOAT_PHYS elas_mises=0.0;
        {
        for(int i=0;i<3;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*1.5; }
        for(int i=3;i<6;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*3.0; }
        }
        elas_mises = std::sqrt(elas_mises);
        const FLOAT_PHYS elas_part = stress_yield / elas_mises;
        // Add elas_part * elastic D-matrix?
        // Secant modulus response: this stress plus elastic response at yield
        FLOAT_PHYS VECALIGNED D[36];
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            D[6* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
          D[6* i+i ]+= shear_eff;
        }
        for(int i=0;i<3;i++){
          D[6* i+i ]+= shear_eff;// + (C[0]-C[1])*elas_part;
          //D[6* (i+3)+i+3 ]+= C[2]*elas_part;
          for(int j=0;j<3;j++){
            D[6* i+j ]+= lambda_eff;// + C[1]*elas_part;
          }
        }
        //===================================================== end UMAT
#if VERB_MAX>10
#pragma omp critical(print)
{
      if( (ie==0) & (ip==0) ){
      printf("el:%u,gp:%i Back stress (alpha):",ie,ip);
      for(int i=0; i<6; i++){
        printf("%+9.2e ", this->elgp_vars[gvar_d*(intp_n*ie+ip) +i ] ); }
      printf("\n");
      }
}
#endif
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
        // Calculate conjugate stress from conjugate strain.
        // Compute the plastic response.
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          P[0], P[5], P[10], P[1]+P[4], P[6]+P[9], P[2]+P[8] };
        FLOAT_PHYS VECALIGNED stress_p[6];
        for(int i=0; i<6; i++){ stress_p[i] =0.0;
          for(int j=0; j<6; j++){ stress_p[i] += D[6* i+j ] * strain_p[ j ]; }
          stress_p[i]*= dw;
        }
        // Compute the linear-elastic response, scaled by elas_part.
        compute_iso_s( &S[0], &P[0],C[2],c0,c1,c2, dw * elas_part );
        // Sum them.
        S[0]+=stress_p[0]; S[1]+=stress_p[3]; S[2]+=stress_p[5];// S[3]=stress_p[3];
        S[4]+=stress_p[3]; S[5]+=stress_p[1]; S[6]+=stress_p[4];// S[7]=stress_p[5];
        S[8]+=stress_p[5]; S[9]+=stress_p[4]; S[10]+=stress_p[2];// S[11]=0.0;
        s[0] = _mm256_load_pd(&S[0]);// sxx sxy sxz | x
        s[1] = _mm256_load_pd(&S[4]);// sxy syy syz | x
        s[2] = _mm256_load_pd(&S[8]);// sxz syz szz | x
      }// if plastic ----------------------------------------------------------
      else{// Linear-elastic conj response only
        compute_iso_s( &S[0], &P[0],C[2],c0,c1,c2, dw );
        s[0] = _mm256_load_pd(&S[0]);
        s[1] = _mm256_load_pd(&S[4]);
        s[2] = _mm256_load_pd(&S[8]);
      }
      if(ip==0){
        for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(elem_p>1){
          for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(elem_p>2){
          for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        }
        }
      }
      accumulate_f( &vf[0], &s[0], &G[0], elem_p );
      }//======================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
        double VECALIGNED sf[4];
        _mm256_store_pd(&sf[0],vf[i]);
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
        for(int j=0; j<3; j++){
          part_f[3*conn[i]+j] = sf[j]; } }
  }// end vf register scope
  }//============================================================ end elem loop
  return 0;
}
#undef VECTORIZED
