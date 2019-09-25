#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
inline void accumulate_f( __m256d* vf,
  const __m256d* a, const FLOAT_PHYS* G, const int elem_p ){
  __m256d a036=a[0];__m256d a147=a[1];__m256d a258=a[2];
#if 1
  for(int i=0; i<2; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]); g1 = _mm256_set1_pd(G[8*i+1]); g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]); g4 = _mm256_set1_pd(G[8*i+5]); g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1], _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));
  }
  if(elem_p>1){
    for(int i=2; i<5; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]); g1 = _mm256_set1_pd(G[8*i+1]); g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]); g4 = _mm256_set1_pd(G[8*i+5]); g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1], _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));
    }
  }
  if(elem_p>2){
    for(int i=5; i<10; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]); g1 = _mm256_set1_pd(G[8*i+1]); g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]); g4 = _mm256_set1_pd(G[8*i+5]); g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1], _mm256_add_pd(_mm256_mul_pd(g3,a036), _mm256_add_pd(_mm256_mul_pd(g4,a147),_mm256_mul_pd(g5,a258))));
    }
  }
#else
  for(int i=0; i<4; i++){
    __m256d g0,g1,g2;
    g0 = _mm256_set1_pd(G[4*i  ]); g1 = _mm256_set1_pd(G[4*i+1]); g2 = _mm256_set1_pd(G[4*i+2]);
    vf[i]= _mm256_add_pd(vf[i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
  }
  if(elem_p>1){
    for(int i=4; i<10; i++){
      __m256d g0,g1,g2;
      g0 = _mm256_set1_pd(G[4*i]); g1 = _mm256_set1_pd(G[4*i+1]); g2 = _mm256_set1_pd(G[4*i+2]);
      vf[i]= _mm256_add_pd(vf[i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
    }
  }
  if(elem_p>2){
    for(int i=10; i<20; i++){
      __m256d g0,g1,g2;
      g0 = _mm256_set1_pd(G[4*i]); g1 = _mm256_set1_pd(G[4*i+1]); g2 = _mm256_set1_pd(G[4*i+2]);
      vf[i]= _mm256_add_pd(vf[i], _mm256_add_pd(_mm256_mul_pd(g0,a036), _mm256_add_pd(_mm256_mul_pd(g1,a147),_mm256_mul_pd(g2,a258))));
    }
  }
#endif
}
inline void rotate_s( __m256d* a,
  const FLOAT_PHYS* R, const FLOAT_PHYS* S ){
  __m256d s0,s1,s2,s4,s5,s8;
  __m256d r0,r3,r6;
  r0  = _mm256_loadu_pd(&R[0]); r3 = _mm256_loadu_pd(&R[3]); r6 = _mm256_loadu_pd(&R[6]);
  s0  = _mm256_set1_pd(S[0])  ; s1 = _mm256_set1_pd(S[4])  ; s2 = _mm256_set1_pd(S[5]);
  s4  = _mm256_set1_pd(S[1])  ; s5 = _mm256_set1_pd(S[6])  ; s8 = _mm256_set1_pd(S[2]);
  a[0]=_mm256_add_pd(_mm256_mul_pd(r0,s0), _mm256_add_pd(_mm256_mul_pd(r3,s1),_mm256_mul_pd(r6,s2)));
  a[1]=_mm256_add_pd(_mm256_mul_pd(r0,s1), _mm256_add_pd(_mm256_mul_pd(r3,s4),_mm256_mul_pd(r6,s5)));
  a[2]=_mm256_add_pd(_mm256_mul_pd(r0,s2), _mm256_add_pd(_mm256_mul_pd(r3,s5),_mm256_mul_pd(r6,s8)));
}
inline void compute_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS* C, const __m256d c0,const __m256d c1,const __m256d c2,
  const FLOAT_PHYS dw){
    __m256d s048 =
      _mm256_mul_pd(_mm256_set1_pd(dw),
        _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])),
          _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[5])),
            _mm256_mul_pd(c2,_mm256_set1_pd(H[10])))));
    _mm256_store_pd(&S[0], s048);
    S[4]=(H[1] + H[4])*C[6]*dw; // S[1]
    S[5]=(H[2] + H[8])*C[8]*dw; // S[2]
    S[6]=(H[6] + H[9])*C[7]*dw; // S[5]
}
inline void compute_g_h(
  FLOAT_PHYS* G, FLOAT_PHYS* H,
  const int Ne, const __m256d j0,const __m256d j1,const __m256d j2,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* R, const FLOAT_PHYS* u ){
  //FLOAT_PHYS* RESTRICT isp = &intp_shpg[ip*Ne];
  __m256d a036=_mm256_set1_pd(0.0), a147=_mm256_set1_pd(0.0), a258=_mm256_set1_pd(0.0);
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    __m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    __m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    is0= _mm256_set1_pd(isp[i+0]); is1= _mm256_set1_pd(isp[i+1]); is2= _mm256_set1_pd(isp[i+2]);
    u0 = _mm256_set1_pd(  u[i+0]); u1 = _mm256_set1_pd(  u[i+1]); u2 = _mm256_set1_pd(  u[i+2]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0), _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
          _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      is3= _mm256_set1_pd(isp[i+3]); is4= _mm256_set1_pd(isp[i+4]); is5= _mm256_set1_pd(isp[i+5]);
      u3 = _mm256_set1_pd(  u[i+3]); u4 = _mm256_set1_pd(  u[i+4]); u5 = _mm256_set1_pd(  u[i+5]);
      g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3), _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
            _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      is6= _mm256_set1_pd(isp[i+6]); is7= _mm256_set1_pd(isp[i+7]); is8= _mm256_set1_pd(isp[i+8]);
      u6 = _mm256_set1_pd(  u[i+6]); u7 = _mm256_set1_pd(  u[i+7]); u8 = _mm256_set1_pd(  u[i+8]);
      g2 = _mm256_add_pd(_mm256_mul_pd(j0,is6), _mm256_add_pd(_mm256_mul_pd(j1,is7),_mm256_mul_pd(j2,is8)));
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g2,u6)); a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7)); a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
            _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  {// scope vector registers
  __m256d h036,h147,h258;
  {
  __m256d r0,r1,r2,r3,r4,r5,r6,r7,r8;
  r0 = _mm256_set1_pd(R[0]); r1 = _mm256_set1_pd(R[1]); r2 = _mm256_set1_pd(R[2]);
  r3 = _mm256_set1_pd(R[3]); r4 = _mm256_set1_pd(R[4]); r5 = _mm256_set1_pd(R[5]);
  r6 = _mm256_set1_pd(R[6]); r7 = _mm256_set1_pd(R[7]); r8 = _mm256_set1_pd(R[8]);
  h036 = _mm256_add_pd(_mm256_mul_pd(r0,a036), _mm256_add_pd(_mm256_mul_pd(r1,a147),_mm256_mul_pd(r2,a258)));
  h147 = _mm256_add_pd(_mm256_mul_pd(r3,a036), _mm256_add_pd(_mm256_mul_pd(r4,a147),_mm256_mul_pd(r5,a258)));
  h258 = _mm256_add_pd(_mm256_mul_pd(r6,a036), _mm256_add_pd(_mm256_mul_pd(r7,a147),_mm256_mul_pd(r8,a258)));
  }
  // Take advantage of the fact that the pattern of usage is invariant with respect to transpose _MM256_TRANSPOSE3_PD(h036,h147,h258);
  _mm256_store_pd(&H[0],h036);
  _mm256_store_pd(&H[4],h147);
  _mm256_store_pd(&H[8],h258);
  }
}
int ElastOrtho3D::Setup( Elem* E ){
  JacRot( E );
  JacT  ( E );
  const uint jacs_n = E->elip_jacs.size()/E->elem_n/ 10 ;
  const uint intp_n = E->gaus_n;
  this->tens_flop = uint(E->elem_n) * intp_n *( uint(E->elem_conn_n)* (36+18) + 2*54 + 27 );
  this->tens_band = uint(E->elem_n) *( sizeof(FLOAT_PHYS)*(3*uint(E->elem_conn_n)*3+ jacs_n*10) +sizeof(INT_MESH)*uint(E->elem_conn_n) );
  this->stif_flop = uint(E->elem_n) * 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) );
  this->stif_band = uint(E->elem_n) *( sizeof(FLOAT_PHYS)* 3*uint(E->elem_conn_n) *( 3*uint(E->elem_conn_n) -1+3) +sizeof(INT_MESH) *uint(E->elem_conn_n) );
  return 0;
}
int ElastOrtho3D::ElemLinear( Elem* E, const INT_MESH e0, const INT_MESH ee,
  FLOAT_SOLV *sys_f, const FLOAT_SOLV* sys_u ){
  //FIXME Cleanup local variables.
  const int Nd = 3;//this->node_d
  const int Nf = 3;// this->node_d DOF/node
  const int Nj =10;//,d2=9;//mesh_d*mesh_d;
  const int Nc = E->elem_conn_n;// Number of Nodes/Element
  const int Ne = Nf*Nc;
  const int Nt =  4*Nc;
  const int intp_n = int(E->gaus_n);
  const INT_ORDER elem_p =E->elem_p;
#if VERB_MAX>11
  printf("Elems:%i, IntPts:%i, Nodes/elem:%i\n",
      (int)elem_n,(int)intp_n,(int)Nc);
#endif
  FLOAT_PHYS dw;
  FLOAT_MESH __attribute__((aligned(32))) jac[Nj];
  FLOAT_PHYS __attribute__((aligned(32))) G[Nt], u[Ne];//,f[Nt];
  FLOAT_PHYS __attribute__((aligned(32))) H[12], S[9];//FIXME S[7]
  // Make local copies
  FLOAT_PHYS __attribute__((aligned(32))) intp_shpg[intp_n*Ne];
  FLOAT_PHYS __attribute__((aligned(32))) wgt[intp_n];
  FLOAT_PHYS __attribute__((aligned(32))) C[this->mtrl_matc.size()];
  //
  std::copy( &E->intp_shpg[0], &E->intp_shpg[intp_n*Ne], intp_shpg );
  std::copy( &E->gaus_weig[0], &E->gaus_weig[intp_n], wgt );
  std::copy( &this->mtrl_matc[0], &this->mtrl_matc[this->mtrl_matc.size()], C );
  __m256d c0,c1,c2;//,c3;
  c0 = _mm256_set_pd(0.0,C[5],C[3],C[0]);
  c1 = _mm256_set_pd(0.0,C[4],C[1],C[3]);
  c2 = _mm256_set_pd(0.0,C[2],C[4],C[5]);
  //c3 = _mm256_set_pd(0.,C[7],C[8],C[6]);
  const FLOAT_PHYS __attribute__((aligned(32))) R[9+1] = {
    mtrl_rotc[0],mtrl_rotc[1],mtrl_rotc[2],
    mtrl_rotc[3],mtrl_rotc[4],mtrl_rotc[5],
    mtrl_rotc[6],mtrl_rotc[7],mtrl_rotc[8],0.0};
#if VERB_MAX>10
  printf( "Material [%u]:", (uint)mtrl_matc.size() );
  for(uint j=0;j<mtrl_matc.size();j++){
    //if(j%mesh_d==0){printf("\n");}
    printf("%+9.2e ",C[j]);
  }; printf("\n");
#endif
  const   INT_MESH* RESTRICT Econn = &E->elem_conn[0];
  const FLOAT_MESH* RESTRICT Ejacs = &E->elip_jacs[0];
  const FLOAT_SOLV* RESTRICT sysu  = &sys_u[0];
  //      FLOAT_SOLV* RESTRICT sysf  = &sys_f[0];
  if(e0<ee){
#ifdef FETCH_JAC
    std::memcpy( &jac , &Ejacs[Nj*e0], sizeof(FLOAT_MESH)*Nj);
#endif
    const   INT_MESH* RESTRICT c = &Econn[Nc*e0];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
    for (int i=0; i<Nc; i++){
      std::memcpy( & u[Nf*i], &sysu[c[i]*Nf], sizeof(FLOAT_SOLV)*Nf );
    }
  }
  {// Scope vf registers
  __m256d vf[Nc];
  //for(int i=0; i<Nc; i++){const __m256d zs={0.0,0.0,0.0,0.0}; vf[i]=zs; }
  for(INT_MESH ie=e0;ie<ee;ie++){
#ifndef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*ie], sizeof(FLOAT_MESH)*Nj);
#endif
  const INT_MESH* RESTRICT conn = &Econn[Nc*ie];

  __m256d j0,j1,j2;//FIXME should these be assigns instead?
  j0 = _mm256_load_pd(&jac[0]);  // j0 = [j3 j2 j1 j0]
  j1 = _mm256_loadu_pd(&jac[3]); // j1 = [j6 j5 j4 j3]
  j2 = _mm256_loadu_pd(&jac[6]); // j2 = [j9 j8 j7 j6]
  for(int ip=0; ip<intp_n; ip++){
    //G = MatMul3x3xN( jac,shg );
    //H = MatMul3xNx3T( G,u );// [H] Small deformation tensor
    compute_g_h( &G[0],&H[0], Ne, j0,j1,j2, &intp_shpg[ip*Ne], &R[0], &u[0] );
#if VERB_MAX>12
    printf( "Small Strains (Elem: %i):", ie );
    for(uint j=0;j<H.size();j++){
      if(j%mesh_d==0){printf("\n");}
      printf("%+9.2e ",H[j]);
    }; printf("\n");
#endif
    dw = jac[9] * wgt[ip];
    if(ip==(intp_n-1)){ if((ie+1)<ee){// Fetch stuff for the next iteration
#ifdef FETCH_JAC
      std::memcpy( &jac, &Ejacs[Nj*(ie+1)], sizeof(FLOAT_MESH)*Nj);
#endif
      const   INT_MESH* RESTRICT c = &Econn[Nc*(ie+1)];
#ifdef __INTEL_COMPILER
#pragma vector unaligned
#endif
      for (int i=0; i<Nc; i++){
        std::memcpy( &u[Nd*i], &sysu[c[i]*Nd], sizeof(FLOAT_SOLV)*Nd ); }
    } }
    // [H] Small deformation tensor
    // [H][RT] : matmul3x3x3T
    compute_s( &S[0], &H[0],&C[0],c0,c1,c2, dw );
#if VERB_MAX>12
    printf( "Stress (Natural Coords):");
    for(uint j=0;j<9;j++){
      if(j%3==0){printf("\n");}
      printf("%+9.2e ",S[j]);
    } printf("\n");
#endif
    // [S][R] : matmul3x3x3, R is transposed
    //accumulate_f( &f[0], &sys_f[0], &conn[0], &R[0], &S[0], &G[0] );
    {// begin scoping unit
    __m256d a[3];
    rotate_s( &a[0], &R[0], &S[0] );
    if(ip==0){// initialize element f
      for(int i=0; i<4; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[ i]]); }
      if(elem_p>1){
        for(int i=4; i<10; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[ i]]); }
      }
      if(elem_p>2){
        for(int i=10; i<20; i++){ vf[i]=_mm256_loadu_pd(&sys_f[3*conn[ i]]); }
      }
    }
    accumulate_f( &vf[0], &a[0], &G[0], elem_p );
    } // end variable scope
    }//end intp loop
#if VERB_MAX>12
    printf( "ff:");
    for(uint j=0;j<Ne;j++){
      if(j%mesh_d==0){printf("\n"); }
      printf("%+9.2e ",f[j]);
    } printf("\n");
#endif
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
        sys_f[3*conn[i]+j] = sf[j];
      } }//--------------------------------------------------- N*3 =  3*N FLOP
  }// end elem loop
  }// end f register scope
return 0;
}
