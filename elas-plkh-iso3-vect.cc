#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
#define COMPRESS_STATE
//
#undef TEST_AVX
//NOTE Prefetch state only works when compressed.
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
#ifdef COMPRESS_STATE
  // The state variable is deviatoric, so drop the first component.
  // Load this starting at state[1], then state[0]=0.0-state[1]-state[2]
  this->gvar_d = 5;
#else
  this->gvar_d = 6;
#endif
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
#ifdef TEST_AVX
  const int Nv = 8;
  const int Nw = 4;
#else
  const int Nv = 6;
  const int Nw = 3;
#endif
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
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
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne], p[Ne];
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
  FLOAT_PHYS VECALIGNED back_v[Nv];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT C     = &matc[0];
        FLOAT_SOLV* RESTRICT state = &this->elgp_vars[0];
#if 0
  const __m256d c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  const __m256d c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
#endif
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
      __m256d vH[Nd], vP[Nd], vS[Nd];
      FLOAT_PHYS VECALIGNED P[Nd*4], S[Nd*4];//FIXME Factor these out
#ifdef COMPRESS_STATE
      std::memcpy( &back_v[1], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#else
      std::memcpy( &back_v[0], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#endif
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      compute_g_p_h( &G[0],&vP[0],&vH[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne],
                     &p[0],&u[0] );
#if 0
      _mm256_store_pd(&H[0],vH[0]);
      _mm256_store_pd(&H[4],vH[1]);
      _mm256_store_pd(&H[8],vH[2]);
#endif
      _mm256_store_pd(&P[0],vP[0]);
      _mm256_store_pd(&P[4],vP[1]);
      _mm256_store_pd(&P[8],vP[2]);
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
      compute_iso_s( &vS[0], &vH[0], C[1],C[2] );// Linear stress response
      _mm256_store_pd(&S[0],vS[0]);
      _mm256_store_pd(&S[4],vS[1]);
      _mm256_store_pd(&S[8],vS[2]);
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
      //     with only 5 independent terms.
      //NOTE back_v is only deviatoric,
      //     so only elas_v hydro needs to be removed.
      {
      FLOAT_PHYS elas_hydr=0.0;
      for(int i=0;i<Nw;i++){ elas_hydr+= elas_devi_v[i]*one_third; }
      for(int i=0;i<3;i++){ elas_devi_v[i]-= elas_hydr; }
      }
      FLOAT_PHYS stress_mises=0.0;
      FLOAT_PHYS VECALIGNED plas_flow[Nv];
#ifdef COMPRESS_STATE
      back_v[0] = -back_v[1]-back_v[2];
#endif
      {
      for(int i=0 ;i<Nv;i++){ plas_flow[i] = elas_devi_v[i] - back_v[i]; }
      for(int i=0 ;i<Nw;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*1.5; }
      for(int i=Nw;i<Nv;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*3.0; }
      }
      if( stress_mises > yield_tol2 ){
        FLOAT_PHYS elas_mises=0.0;
        {
        for(int i=0 ;i<Nw;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*1.5; }
        for(int i=Nw;i<Nv;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*3.0; }
        }
        // scalar ops ------------------------------
        elas_mises = sqrt(elas_mises);
        const FLOAT_PHYS elas_part = stress_yield / elas_mises;
        stress_mises = sqrt(stress_mises);
        const FLOAT_PHYS inv_mises = 1.0/stress_mises;
#if 0
        const FLOAT_PHYS delta_equiv = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu );
        // delta_equiv is only used as delta_equiv * hard_modu
#endif
        const FLOAT_PHYS delta_hard = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu ) * hard_modu;
        const FLOAT_PHYS shear_eff
          = shear_modu * (stress_yield + delta_hard) * inv_mises;
        const FLOAT_PHYS hard_eff = shear_modu * hard_modu
          / ( shear_modu + hard_modu*one_third ) - 3.0*shear_eff;
        const FLOAT_PHYS lambda_eff = ( bulk_mod3 - 2.0*shear_eff )*one_third;
        // end scalar ops -------------------------
        for(int i=0;i<Nv;i++){ plas_flow[i] *= inv_mises; }
        if( save_state ){// Update state variable back_v.
          for(int i=0; i<Nv; i++){//FIXME can be from i=1
            back_v[i] += plas_flow[i] * delta_hard; }
          //FLOAT_PHYS back_t[6];
#ifdef COMPRESS_STATE
          std::memcpy(&state[Ns*(intp_n*ie+ip)],&back_v[1],sizeof(FLOAT_SOLV)*Ns);
#else
          std::memcpy(&state[Ns*(intp_n*ie+ip)],&back_v[0],sizeof(FLOAT_SOLV)*Ns);
#endif
        }// done if save_state
        // Secant modulus response: this stress plus elastic response at yield
#if VERB_MAX>10
#pragma omp critical(print)
{
        printf("el:%u,gp:%i d:     \n",ie,ip);
        for(int i=0; i<(6*Nv); i++){
          //if(!(i%6)&(i>0)){ printf("                 "); }
          printf(" %+9.2e",D[i] );
          if((i%Nv)==(Nv-1)){ printf("\n"); }
        }
}
#endif
#if 0
        FLOAT_PHYS VECALIGNED D[6*Nv];
#if 0
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            D[6* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
          D[6* i+i ]+= shear_eff;
        }
        for(int i=0;i<3;i++){
          D[6* i+i ]+= shear_eff;
          for(int j=0;j<3;j++){
            D[6* i+j ]+= lambda_eff;
          }
        }
#else
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            D[6* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
        }
#endif
#endif
        // Pass lambda_eff,shear_eff out
        //===================================================== end UMAT
#if VERB_MAX>10
#pragma omp critical(print)
{
        if( (ie==0) & (ip==0) ){
        printf("el:%u,gp:%i back_v: ",ie,ip);
#ifdef COMPRESS_STATE
        printf("%+9.2e ", 0.0 -this->elgp_vars[gvar_d*(intp_n*ie+ip) +0 ]
          -this->elgp_vars[gvar_d*(intp_n*ie+ip) +1 ]
        );
#endif
        for(int i=0; i<gvar_d; i++){
          printf("%+9.2e ", this->elgp_vars[gvar_d*(intp_n*ie+ip) +i ] ); }
        printf("\n");
        }
}
#endif
#if VERB_MAX>10
#pragma omp critical(print)
{
        printf("el:%u,gp:%i D:     \n",ie,ip);
        for(int i=0; i<(6*Nv); i++){
          //if(!(i%6)&(i>0)){ printf("                 "); }
          printf(" %+9.2e", D[i] );
          if((i%Nv)==(Nv-1)){ printf("\n"); }
        }
}
#endif
        // Calculate conjugate stress from conjugate strain.
        // Compute the linear-elastic conjugate response, scaled by elas_part.
        compute_iso_s( &vS[0], &vP[0], C[1]*elas_part*dw, C[2]*elas_part*dw );
        // Compute the plastic conjugate response.
        __m256d vT[Nd];
        compute_iso_s( &vT[0], &vP[0], lambda_eff*dw, shear_eff*dw );
#if 0
      _mm256_store_pd(&S[0],vS[0]);
      _mm256_store_pd(&S[4],vS[1]);
      _mm256_store_pd(&S[8],vS[2]);
      _mm256_store_pd(&T[0],vT[0]);
      _mm256_store_pd(&T[4],vT[1]);
      _mm256_store_pd(&T[8],vT[2]);
#endif
#if 0
        FLOAT_PHYS VECALIGNED stress_p[6];
        {
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          P[0], P[5], P[10], P[1]+P[4], P[6]+P[9], P[2]+P[8] };
        for(int i=0; i<6; i++){ stress_p[i]=0.0;
          for(int j=0; j<Nv; j++){
            stress_p[i] += D[Nv* i+j ] * strain_p[ j ]; }
          stress_p[i] *= dw; }
        }
        for(int i=0;i<(Nd*4);i++){ S[i]+=T[i]; }
        // Sum them.
        S[0]+=stress_p[0]; S[1]+=stress_p[3]; S[2]+=stress_p[5];// S[3]+=stress_p[3];
        S[4]+=stress_p[3]; S[5]+=stress_p[1]; S[6]+=stress_p[4];// S[7]+=stress_p[5];
        S[8]+=stress_p[5]; S[9]+=stress_p[4]; S[10]+=stress_p[2];// S[11]=0.0;
#else
        FLOAT_PHYS F[12]={
          plas_flow[0], plas_flow[3], plas_flow[5], 0.0,
          plas_flow[3], plas_flow[1], plas_flow[4], 0.0,
          plas_flow[5], plas_flow[4], plas_flow[2], 0.0 };
#if 0
        for(int i=0;i<(Nd*4);i++){
          S[i] += T[i];
          for(int k=0;k<(Nd*4);k++){
            S[i] += F[i] * F[k] * P[k] * hard_eff*dw; } }
#else
#if 0
      {//FIXME Double or halve shear terms?
      const __m256d he=_mm256_set1_pd( hard_eff * dw );
      for(int i=0;i<12;i+=4){
        const __m256d f = _mm256_load_pd( &F[i] );
        __m256d s = _mm256_load_pd( &S[i] ) + _mm256_load_pd( &T[i] );
        for(int j=0;j<11;j++){
          s += f *_mm256_set1_pd( F[j] * P[j] ) * he; }
        _mm256_store_pd( &S[i], s ); }
      }
      vS[0]= _mm256_load_pd( &S[0] );
      vS[1]= _mm256_load_pd( &S[4] );
      vS[2]= _mm256_load_pd( &S[8] );
#else
      {//FIXME Double or halve shear terms?
      const __m256d he=_mm256_set1_pd( hard_eff * dw );
      for(int i=0;i<3;i++){
        const __m256d f = _mm256_load_pd( &F[4*i] );
        __m256d s = vS[i] + vT[i];
        for(int j=0;j<11;j++){
          s += f *_mm256_set1_pd( F[j] * P[j] ) * he; }
        vS[i] = s; }
      }
#endif
#endif
        //for(int i=0;i<(Nd*4);i++){ S[i]+=T[i]; }
#endif
      }// if plastic ----------------------------------------------------------
      else{// Linear-elastic conj response only
        compute_iso_s( &vS[0], &vP[0], C[1]*dw,C[2]*dw );
      }
      if(ip==0){
        for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(elem_p>1){
          for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        if(elem_p>2){
          for(int i=10;i<20; i++){ vf[i]=_mm256_loadu_pd(&part_f[3*conn[i]]); }
        }
        }
      }
      {
      accumulate_f( &vf[0], &vS[0], &G[0], elem_p );
      }
      }//======================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
        FLOAT_SOLV VECALIGNED sf[4];
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
int ElastPlastKHIso3D::ElemLinear( Elem* E,
  const INT_MESH e0,const INT_MESH ee, FLOAT_SOLV* RESTRICT part_f,
  const FLOAT_SOLV* RESTRICT part_u ){
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
  //const int        Ns           = this->gvar_d;// Number of state variables/ip
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
#if VERB_MAX>11
  printf("DOF: %u, Elems:%u, IntPts:%u, Nodes/elem:%u\n",
    (uint)ndof,(uint)elem_n,(uint)intp_n,(uint)Nc );
#endif
  FLOAT_MESH VECALIGNED jac[Nj];
  FLOAT_PHYS VECALIGNED G[Nt], u[Ne];
  FLOAT_PHYS VECALIGNED intp_shpg[intp_n*Ne];
  FLOAT_PHYS VECALIGNED wgt[intp_n];
  FLOAT_PHYS VECALIGNED matc[this->mtrl_matc.size()];
#if 1
  FLOAT_PHYS VECALIGNED back_v[6];
#else
  FLOAT_PHYS VECALIGNED back_v[8];// back_v padded to 8 doubles?
#endif
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[mtrl_matc.size()], matc );
  //std::copy( &this->elgp_vars[0], 6, back_v );
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT C     = &matc[0];
        FLOAT_SOLV* RESTRICT state = &this->elgp_vars[0];
#if 0
  const __m256d c0 = _mm256_set_pd(0.0,C[1],C[1],C[0]);
  const __m256d c1 = _mm256_set_pd(0.0,C[1],C[0],C[1]);
  const __m256d c2 = _mm256_set_pd(0.0,C[0],C[1],C[1]);
#endif
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
      std::memcpy( & u[Nf*i],&part_u[c[i]*Nf],sizeof(FLOAT_SOLV)*Nf ); }
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
      __m256d vH[Nd], vS[Nd];
      FLOAT_PHYS VECALIGNED H[Nd*4], S[Nd*4];
#ifdef COMPRESS_STATE
      std::memcpy( &back_v[1], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#else
      std::memcpy( &back_v[0], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#endif
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      compute_g_h( &G[0],&vH[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne],&u[0] );
      _mm256_store_pd(&H[0],vH[0]);
      _mm256_store_pd(&H[4],vH[1]);
      _mm256_store_pd(&H[8],vH[2]);
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
          std::memcpy(& u[Nf*i],& part_u[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf ); }
#endif
#ifdef FETCH_JAC
          std::memcpy(& jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj );
#endif
      } }
      compute_iso_s( &vS[0], &vH[0], C[1],C[2] );// Linear stress response
      _mm256_store_pd(&S[0],vS[0]);
      _mm256_store_pd(&S[4],vS[1]);
      _mm256_store_pd(&S[8],vS[2]);
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
      //     with only 5 independent terms.
      //NOTE back_v is incompressive (deviatoric),
      //     so only elas_v hydro needs to be removed.
      {
      FLOAT_PHYS elas_hydr=0.0;
      for(int i=0;i<3;i++){ elas_hydr+= elas_devi_v[i]*one_third; }
      for(int i=0;i<3;i++){ elas_devi_v[i]-= elas_hydr; }
      }
      FLOAT_PHYS VECALIGNED plas_flow[6];
      FLOAT_PHYS stress_mises=0.0;
#ifdef COMPRESS_STATE
      back_v[0] = -back_v[1]-back_v[2];
#endif
      {
      for(int i=0;i<6;i++){ plas_flow[i] = elas_devi_v[i] - back_v[i]; }
      for(int i=0;i<3;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*1.5; }
      for(int i=3;i<6;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*3.0; }
      }
      //for(int i=0;i<6;i++){ printf("%+9.2e ", elas_v[i]); }
      //printf("mises: %+9.2e\n",sqrt(stress_mises));
      if( stress_mises > yield_tol2 ){
        FLOAT_PHYS elas_part;
        {
        FLOAT_PHYS elas_mises=0.0;
        for(int i=0;i<3;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*1.5; }
        for(int i=3;i<6;i++){ elas_mises += elas_devi_v[i]*elas_devi_v[i]*3.0; }
        // only scalar ops -----------------
        elas_mises = sqrt(elas_mises);
        elas_part = stress_yield / elas_mises;//FIXME max 1.0?
        }
        stress_mises = sqrt(stress_mises);
        const FLOAT_PHYS inv_mises = 1.0/stress_mises;
#if 0
        const FLOAT_PHYS delta_equiv = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu );
        // delta_equiv is only used as delta_equiv * hard_modu
#endif
        const FLOAT_PHYS delta_hard = ( stress_mises - stress_yield )
          / ( 3.0*shear_modu + hard_modu ) * hard_modu;
        const FLOAT_PHYS shear_eff
          = shear_modu * ( stress_yield + delta_hard ) * inv_mises;
        const FLOAT_PHYS hard_eff = shear_modu * hard_modu
          / ( shear_modu + hard_modu*one_third ) - 3.0*shear_eff;
        const FLOAT_PHYS lambda_eff = ( bulk_mod3 - 2.0*shear_eff )*one_third;
        // end scalar ops -------------------// 20 scalar FLOP
        for(int i=0;i<6;i++){ plas_flow[i]*= inv_mises; }
        // Add elas_part * elastic D-matrix?
        // Secant modulus response: this stress plus elastic response at yield
#if 0
        FLOAT_PHYS VECALIGNED D[36];
        for(int i=0;i<6;i++){
          for(int j=0;j<6;j++){
            D[6* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];// 72 FLOP
          }
          D[6* i+i ]+= shear_eff;// 6 FLOP
        }
        for(int i=0;i<3;i++){
          D[6* i+i ]+= shear_eff;// + (C[0]-C[1])*elas_part;// 3 FLOP
          //D[6* (i+3)+i+3 ]+= C[2]*elas_part;
          for(int j=0;j<3;j++){
            D[6* i+j ]+= lambda_eff;// + C[1]*elas_part; // 9 FLOP
          }
        }// TOTAL: 90 scalar FLOP
#endif
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
#if VERB_MAX>11
#pragma omp critical(print)
{
        printf("el:%u,gp:%i D:     \n",ie,ip);
        for(int i=0; i<36; i++){
          if(!(i%6)&(i>0)){ printf("                 "); }
          printf(" %+9.2e", D[i] );
          if((i%6)==5){ printf("\n"); }
        }
}
#endif
        // Calculate stress from strain.
#if 0
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          H[0], H[5], H[10], H[1]+H[4], H[6]+H[9], H[2]+H[8] };
        FLOAT_PHYS VECALIGNED stress_p[6];
        // Compute the plastic response.
        for(int i=0; i<6; i++){ stress_p[i] =0.0;
          for(int j=0; j<6; j++){ stress_p[i] += D[6* i+j ] * strain_p[ j ]; }
          stress_p[i]*= dw;
        }
        // Compute the linear-elastic response, scaled by elas_part.
        //compute_iso_s( &S[0], &H[0], C[1],C[2], dw * elas_part );
        for(int i=0; i<12; i++){ S[i]*= dw * elas_part; }
        // Sum them.
        S[0]+=stress_p[0]; S[1]+=stress_p[3]; S[2]+=stress_p[5];// S[3]=stress_p[3];
        S[4]+=stress_p[3]; S[5]+=stress_p[1]; S[6]+=stress_p[4];// S[7]=stress_p[5];
        S[8]+=stress_p[5]; S[9]+=stress_p[4]; S[10]+=stress_p[2];// S[11]=0.0;
#else
        // Calculate stress from strain.
        // Compute the plastic response.
        __m256d VECALIGNED vT[Nd];
        compute_iso_s( &vT[0], &vH[0], lambda_eff*dw, shear_eff*dw );
#if 0
      _mm256_store_pd(&S[0],vS[0]);
      _mm256_store_pd(&S[4],vS[1]);
      _mm256_store_pd(&S[8],vS[2]);
      _mm256_store_pd(&T[0],vT[0]);
      _mm256_store_pd(&T[4],vT[1]);
      _mm256_store_pd(&T[8],vT[2]);
#endif
        FLOAT_PHYS F[12]={
          plas_flow[0], plas_flow[3], plas_flow[5], 0.0,
          plas_flow[3], plas_flow[1], plas_flow[4], 0.0,
          plas_flow[5], plas_flow[4], plas_flow[2], 0.0 };
#if 0
        for(int i=0;i<(Nd*4);i++){
          // Scale the linear-elastic  response by elas_part.
          S[i] = S[i] * dw * elas_part + T[i];
          for(int k=0;k<(Nd*4);k++){
            S[i] += F[i] * F[k] * H[k] * hard_eff * dw;
          } }
#else
      for(int i=0;i<3;i++){//FIXME Double shear terms?
        const __m256d ep=_mm256_set1_pd( elas_part * dw );
        const __m256d he=_mm256_set1_pd( hard_eff * dw );
        const __m256d f =_mm256_load_pd( &F[4*i] );
        __m256d s=_mm256_setzero_pd();
        for(int j=0;j<11;j++){
          s += f *_mm256_set1_pd( F[j] * H[j] );
        }
        vS[i]= s*he + vS[i]*ep + vT[i] ;
      }
#endif
#endif
      }// if plastic ----------------------------------------------------------
      else{// Linear-elastic response only
        //for(int i=0; i<12; i++){ S[i]*= dw; }
        for(int i=0; i<3; i++){ vS[i]*= dw; }
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
      {
      accumulate_f( &vf[0], &vS[0], &G[0], elem_p );
      }
      }//======================================================== end intp loop
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
        FLOAT_SOLV VECALIGNED sf[4];
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
//
