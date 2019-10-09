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
#define TEST_AVX
#define TEST_AVX2
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
  FLOAT_PHYS VECALIGNED H[Nd*4],P[Nd*4], S[Nd*4];//FIXME S size
  //
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
#ifdef COMPRESS_STATE
      std::memcpy( &back_v[1], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#else
      std::memcpy( &back_v[0], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
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
#ifdef TEST_AVX
      FLOAT_PHYS VECALIGNED elas_devi_v[8]={
        S[0], S[5], S[10],0.0, S[1], S[6], S[2],0.0 };
#else
      FLOAT_PHYS VECALIGNED elas_devi_v[6]={
        S[0], S[5], S[10], S[1], S[6], S[2] };// sxx, syy, szz,  sxy, syz, sxz
#endif
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
#ifdef TEST_AVX
      back_v[7]=0.0;
      back_v[6]=back_v[5]; back_v[5]=back_v[4]; back_v[4]=back_v[3];
      back_v[3]=0.0;
#endif
      {
      for(int i=0 ;i<Nv;i++){ plas_flow[i] = elas_devi_v[i] - back_v[i]; }
      for(int i=0 ;i<Nw;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*1.5; }
      for(int i=Nw;i<Nv;i++){ stress_mises+= plas_flow[i]*plas_flow[i]*3.0; }
      }
      //for(int i=0;i<6;i++){ printf("%+9.2e ", elas_v[i]); }
      //printf("mises: %+9.2e\n",sqrt(stress_mises));
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
#ifdef TEST_AVX
          back_v[3]=back_v[4]; back_v[4]=back_v[5]; back_v[5]=back_v[6];
          //back_t[0]=back_v[0]; back_t[1]=back_v[1]; back_t[2]=back_v[2];
          //back_t[3]=back_v[4]; back_t[4]=back_v[5]; back_t[5]=back_v[6];
//#else
//          for(int i=0; i<6; i++){ back_t[i]=back_v[i]; }
#endif
#ifdef COMPRESS_STATE
          std::memcpy(&state[Ns*(intp_n*ie+ip)],&back_v[1],sizeof(FLOAT_SOLV)*Ns);
#else
          std::memcpy(&state[Ns*(intp_n*ie+ip)],&back_v[0],sizeof(FLOAT_SOLV)*Ns);
#endif
        }// done if save_state
        // Secant modulus response: this stress plus elastic response at yield
#ifdef TEST_AVX
        __m256d d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11;
        {
        const __m256d he=_mm256_set1_pd( hard_eff );
        const __m256d f0=_mm256_load_pd( &plas_flow[0] ) * he;
        const __m256d f1=_mm256_load_pd( &plas_flow[4] ) * he;
        __m256d frow;
        frow=_mm256_set1_pd( plas_flow[0] );
        d0 = f0 * frow; d1 = f1 * frow ;
        frow=_mm256_set1_pd( plas_flow[1] );
        d2 = f0 * frow; d3 = f1 * frow;
        frow=_mm256_set1_pd( plas_flow[2] );
        d4 = f0 * frow; d5 = f1 * frow;
        frow=_mm256_set1_pd( plas_flow[4] );
        d6 = f0 * frow; d7 = f1 * frow;
        frow=_mm256_set1_pd( plas_flow[5] );
        d8 = f0 * frow; d9 = f1 * frow;
        frow=_mm256_set1_pd( plas_flow[6] );
        d10 = f0 * frow; d11 = f1 * frow;
        }
        {
        const __m256d le=_mm256_set_pd( 0.0,lambda_eff,lambda_eff,lambda_eff );
        d0+=le; d2+=le; d4+=le;
        }
#ifdef TEST_AVX2
        d7+=_mm256_set_pd( 0.0,0.0,0.0,shear_eff );
        d9+=_mm256_permute4x64_pd( d7, _MM_SHUFFLE(3,2,0,1) );
        d11+=_mm256_permute4x64_pd( d7, _MM_SHUFFLE(3,0,2,1) );
        d0+=d7*2.0;
        d2+=d9*2.0;
        d4+=d11*2.0;
#else
        {
        const FLOAT_PHYS VECALIGNED se[12]={
          shear_eff,0.0,0.0,0.0,
          0.0,shear_eff,0.0,0.0,
          0.0,0.0,shear_eff,0.0 };
        d0+=_mm256_load_pd( &se[0] )*2.0;
        d2+=_mm256_load_pd( &se[4] )*2.0;
        d4+=_mm256_load_pd( &se[8] )*2.0;
        d7+=_mm256_load_pd( &se[0] );
        d9+=_mm256_load_pd( &se[4] );
        d11+=_mm256_load_pd(&se[8] );
        }
#endif
#if 0
        _mm256_store_pd( &D[ 0], d0 ); _mm256_store_pd( &D[ 4], d1 );
        _mm256_store_pd( &D[ 8], d2 ); _mm256_store_pd( &D[12], d3 );
        _mm256_store_pd( &D[16], d4 ); _mm256_store_pd( &D[20], d5 );
        _mm256_store_pd( &D[24], d6 ); _mm256_store_pd( &D[28], d7 );
        _mm256_store_pd( &D[32], d8 ); _mm256_store_pd( &D[36], d9 );
        _mm256_store_pd( &D[40], d10); _mm256_store_pd( &D[44], d11);
#endif
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
#else
        FLOAT_PHYS VECALIGNED D[6*Nv];
        for(int i=0;i<3;i++){
          for(int j=0;j<Nw;j++){// top left side
            D[Nv* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
          for(int j=Nw;j<Nv;j++){// top right side
            D[Nv* i+j ] = hard_eff * plas_flow[i] * plas_flow[j];
          }
        }
        for(int i=0;i<3;i++){
          for(int j=0;j<Nw;j++){// bottom left side
            D[Nv* i+j +3*Nv] = hard_eff * plas_flow[i+Nw] * plas_flow[j];
          }
          for(int j=Nw;j<Nv;j++){// bottom right side
            D[Nv* i+j +3*Nv] = hard_eff * plas_flow[i+Nw] * plas_flow[j];
          }
        }
        for(int i=0 ;i<3;i++){ D[Nv* i+i   ]+= shear_eff*2.0; }
        for(int i=0 ;i<3;i++){ D[3*Nv+Nw + (Nv+1)*i]+= shear_eff; }
        for(int i=0 ;i<3;i++){
          for(int j=0 ;j<3;j++){
            D[Nv* i+j ]+= lambda_eff;
          }
        }
#endif
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
#ifdef TEST_AVX
        FLOAT_PHYS VECALIGNED stress_p[8];//={0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0};
#else
        FLOAT_PHYS VECALIGNED stress_p[6];//={0.0,0.0,0.0, 0.0,0.0,0.0};
#endif
        // Compute the plastic conjugate response.
        {
#ifdef TEST_AVX
        {
        const __m256d w =_mm256_set1_pd( dw );
        __m256d s0,s1;
        __m256d erow;
#if 1
        erow=_mm256_set1_pd( P[ 0] ); s0 =d0*erow; s1 =d1*erow;
        erow=_mm256_set1_pd( P[ 5] ); s0+=d2*erow; s1+=d3*erow;
        erow=_mm256_set1_pd( P[10] ); s0+=d4*erow; s1+=d5*erow;
#if 1
        erow=_mm256_set1_pd( P[1] )+_mm256_set1_pd( P[4] );
        s0+=d6*erow; s1+=d7*erow;
        erow=_mm256_set1_pd( P[6] )+_mm256_set1_pd( P[9] );
        s0+=d8*erow; s1+=d9*erow;
        erow=_mm256_set1_pd( P[2] )+_mm256_set1_pd( P[8] );
        s0+=d10*erow;s1+=d11*erow;
#else
        erow=_mm256_set1_pd( P[1]+P[4] ); s0+=d6*erow; s1+=d7*erow;
        erow=_mm256_set1_pd( P[6]+P[9] ); s0+=d8*erow; s1+=d9*erow;
        erow=_mm256_set1_pd( P[2]+P[8] ); s0+=d10*erow;s1+=d11*erow;
#endif
#else
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          P[0], P[5], P[10], P[1]+P[4], P[6]+P[9], P[2]+P[8] };
        erow=_mm256_set1_pd( strain_p[0] ); s0 =d0*erow; s1 =d1*erow;
        erow=_mm256_set1_pd( strain_p[1] ); s0+=d2*erow; s1+=d3*erow;
        erow=_mm256_set1_pd( strain_p[2] ); s0+=d4*erow; s1+=d5*erow;
        erow=_mm256_set1_pd( strain_p[3] ); s0+=d6*erow; s1+=d7*erow;
        erow=_mm256_set1_pd( strain_p[4] ); s0+=d8*erow; s1+=d9*erow;
        erow=_mm256_set1_pd( strain_p[5] ); s0+=d10*erow;s1+=d11*erow;
#endif
        _mm256_store_pd( &stress_p[0],s0*w);
        _mm256_storeu_pd(&stress_p[3],s1*w);
        }
#else
#if 0
        const FLOAT_PHYS VECALIGNED strain_p[8]={
          P[0], P[5], P[10],0.0, P[1]+P[4], P[6]+P[9], P[2]+P[8],0.0 };
#endif
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          P[0], P[5], P[10], P[1]+P[4], P[6]+P[9], P[2]+P[8] };
        for(int i=0; i<6; i++){ stress_p[i]=0.0;
          for(int j=0; j<Nv; j++){
            stress_p[i] += D[Nv* i+j ] * strain_p[ j ]; }
          stress_p[i] *= dw;
        }
#endif
        }
        // Compute the linear-elastic conjugate response, scaled by elas_part.
        compute_iso_s( &S[0], &P[0],C[2],c0,c1,c2, dw * elas_part );
        // Sum them.
#if 0
        S[0]+=stress_p[0]; S[1]+=stress_p[4]; S[2]+=stress_p[6];// S[3]+=stress_p[3];
        S[4]+=stress_p[4]; S[5]+=stress_p[1]; S[6]+=stress_p[5];// S[7]+=stress_p[5];
        S[8]+=stress_p[6]; S[9]+=stress_p[5]; S[10]+=stress_p[2];// S[11]=0.0;
#else
        S[0]+=stress_p[0]; S[1]+=stress_p[3]; S[2]+=stress_p[5];// S[3]+=stress_p[3];
        S[4]+=stress_p[3]; S[5]+=stress_p[1]; S[6]+=stress_p[4];// S[7]+=stress_p[5];
        S[8]+=stress_p[5]; S[9]+=stress_p[4]; S[10]+=stress_p[2];// S[11]=0.0;
#endif
      }// if plastic ----------------------------------------------------------
      else{// Linear-elastic conj response only
        compute_iso_s( &S[0], &P[0],C[2],c0,c1,c2, dw );
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
      __m256d s[3];
      s[0] = _mm256_load_pd(&S[0]);// sxx sxy sxz | x
      s[1] = _mm256_load_pd(&S[4]);// sxy syy syz | x
      s[2] = _mm256_load_pd(&S[8]);// sxz syz szz | x
      accumulate_f( &vf[0], &s[0], &G[0], elem_p );
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
  FLOAT_PHYS VECALIGNED H[Nd*4], S[Nd*4];//FIXME S size
  //
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
#ifdef COMPRESS_STATE
      std::memcpy( &back_v[1], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#else
      std::memcpy( &back_v[0], &state[Ns*(intp_n*ie+ip)], sizeof(FLOAT_PHYS)*Ns );
#endif
      //G = MatMul3x3xN( jac,shg );
      //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
      compute_g_h( &G[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne],&u[0] );
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
      printf("el:%u,gp:%i D:     \n",ie,ip);
      for(int i=0; i<36; i++){
        if(!(i%6)&(i>0)){ printf("                 "); }
        printf(" %+9.2e", D[i] );
        if((i%6)==5){ printf("\n"); }
      }
}
#endif
        // Calculate stress from strain.
        const FLOAT_PHYS VECALIGNED strain_p[6]={
          H[0], H[5], H[10], H[1]+H[4], H[6]+H[9], H[2]+H[8] };
        FLOAT_PHYS VECALIGNED stress_p[6];
        // Compute the plastic response.
        for(int i=0; i<6; i++){ stress_p[i] =0.0;
          for(int j=0; j<6; j++){ stress_p[i] += D[6* i+j ] * strain_p[ j ]; }
          stress_p[i]*= dw;
        }
        // Compute the linear-elastic response, scaled by elas_part.
        compute_iso_s( &S[0], &H[0],C[2],c0,c1,c2, dw * elas_part );
        // Sum them.
        S[0]+=stress_p[0]; S[1]+=stress_p[3]; S[2]+=stress_p[5];// S[3]=stress_p[3];
        S[4]+=stress_p[3]; S[5]+=stress_p[1]; S[6]+=stress_p[4];// S[7]=stress_p[5];
        S[8]+=stress_p[5]; S[9]+=stress_p[4]; S[10]+=stress_p[2];// S[11]=0.0;
      }// if plastic ----------------------------------------------------------
      else{// Linear-elastic response only
        for(int i=0; i<12; i++){ S[i]*= dw; }
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
      __m256d s[3];
      s[0] = _mm256_load_pd(&S[0]);// sxx sxy sxz | x
      s[1] = _mm256_load_pd(&S[4]);// sxy syy syz | x
      s[2] = _mm256_load_pd(&S[8]);// sxz syz szz | x
      accumulate_f( &vf[0], &s[0], &G[0], elem_p );
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
