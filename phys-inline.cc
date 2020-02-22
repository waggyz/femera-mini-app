#ifndef INCLUDED_PHYS_INLINE_C
#define INCLUDED_PHYS_INLINE_C
#include "femera.h"
#include <immintrin.h>
//FIXME These inline functions should be in the class where used.
#if 1
static inline void print_m256(const __m256d v){// Debugging register print
  double V[4];
  _mm256_store_pd(&V[0],v);
  printf("%9.2e %9.2e %9.2e %9.2e\n",V[0],V[1],V[2],V[3]);
}
#endif
// Physics --------------------------------------------------------------------
static inline void accumulate_f(__m256d* vf,
  const __m256d* vS, const FLOAT_PHYS* G, const int Nc ){
  for(int i= 0; i< 4; i++){// This is a little faster unrolled.
    vf[i]
      +=vS[0] *_mm256_set1_pd(G[4*i  ])
      + vS[1] *_mm256_set1_pd(G[4*i+1])
      + vS[2] *_mm256_set1_pd(G[4*i+2]);
  }
  if(Nc>4){
    for(int i= 4; i<10; i++){
    vf[i]
      +=vS[0] *_mm256_set1_pd(G[4*i  ])
      + vS[1] *_mm256_set1_pd(G[4*i+1])
      + vS[2] *_mm256_set1_pd(G[4*i+2]);
    }
    if(Nc>10){
      for(int i=10; i<20; i++){
      vf[i]
        +=vS[0] *_mm256_set1_pd(G[4*i  ])
        + vS[1] *_mm256_set1_pd(G[4*i+1])
        + vS[2] *_mm256_set1_pd(G[4*i+2]);
      }
    }
  }
}
// Orthotropic Elastic --------------------------------------------------------
static inline void rotate_g_h(FLOAT_PHYS* G,__m256d* vH,// was line 40
  const int Nc, const __m256d* vJ,
  const FLOAT_PHYS* sg, const FLOAT_PHYS* R, const FLOAT_PHYS* u ){
  __m256d a036=_mm256_setzero_pd(), a147=_mm256_setzero_pd(),
    a258=_mm256_setzero_pd();
  for(int i= 0; i<Nc; i++){
      const __m256d vg
        = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
        + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
        + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
      a036+= vg *_mm256_set1_pd(  u[3* i+0 ]);
      a147+= vg *_mm256_set1_pd(  u[3* i+1 ]);
      a258+= vg *_mm256_set1_pd(  u[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], vg );
  }// was line 78
  vH[0]
    = a036 * _mm256_set1_pd(R[0])
    + a147 * _mm256_set1_pd(R[1])
    + a258 * _mm256_set1_pd(R[2]);
  vH[1]
    = a036 * _mm256_set1_pd(R[3])
    + a147 * _mm256_set1_pd(R[4])
    + a258 * _mm256_set1_pd(R[5]);
  vH[2]
    = a036 * _mm256_set1_pd(R[6])
    + a147 * _mm256_set1_pd(R[7])
    + a258 * _mm256_set1_pd(R[8]);
  // Take advantage of the fact that the pattern of usage is invariant
  // with respect to transpose _MM256_TRANSPOSE3_PD(h036,h147,h258);
}// was line 93
static inline void compute_ort_s_voigt(FLOAT_PHYS* S, const __m256d* vH,
  const __m256d* vC, const FLOAT_PHYS dw){// was line 95
  FLOAT_PHYS VECALIGNED H[12];
  _mm256_store_pd( &H[0], vH[0] );
  _mm256_store_pd( &H[4], vH[1] );
  _mm256_store_pd( &H[8], vH[2] );
  //Vectorized calc for diagonal of S
  _mm256_store_pd( &S[0],
    _mm256_set1_pd(dw) *
    ( vC[0] *_mm256_set1_pd(H[ 0])
    + vC[1] *_mm256_set1_pd(H[ 5])
    + vC[2] *_mm256_set1_pd(H[10])
    ) );
  {FLOAT_PHYS VECALIGNED c678[4];
  _mm256_store_pd( &c678[0], vC[3] );
  S[4]=(H[1] + H[4])*c678[0]*dw;// S[1]
  S[5]=(H[2] + H[8])*c678[2]*dw;// S[2]
  S[6]=(H[6] + H[9])*c678[1]*dw;// S[5]
  }
}
static inline void rotate_s_voigt( __m256d* vS,
    const FLOAT_PHYS* S, const __m256d* vR ){
  const __m256d s0 = _mm256_set1_pd(S[0]);
  const __m256d s1 = _mm256_set1_pd(S[4]);
  const __m256d s2 = _mm256_set1_pd(S[5]);
  vS[0] = s0 * vR[0] + s1 * vR[1] + s2 * vR[2];
  const __m256d s4 = _mm256_set1_pd(S[1]);
  const __m256d s5 = _mm256_set1_pd(S[6]);
  vS[1] = s1 * vR[0] + s4 * vR[1] + s5 * vR[2];
  const __m256d s8 = _mm256_set1_pd(S[2]);
  vS[2] = s2 * vR[0] + s5 * vR[1] + s8 * vR[2];
}
// Isotropic Thermal ----------------------------------------------------------
static inline void thermal_iso_s(FLOAT_PHYS* A,
  const FLOAT_PHYS Cdw ){
      A[0]*= Cdw;//Sxx
      A[1]*= Cdw;//Syy
      A[2]*= Cdw;//Szz
}
static inline void thermal_iso_s(__m256d* A,
  const FLOAT_PHYS Cdw ){
      A[0]*=_mm256_set1_pd(Cdw);
}
static inline void thermal_g_h( __m256d* vG, __m256d* vH,
  const int Nc, const __m256d* vJ, const FLOAT_PHYS* sg, const FLOAT_PHYS* u ){
  for(int i= 0; i<Nc; i++){
    vG[ i ]
      = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
      + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
      + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
    vH[0]+= vG[ i ] *_mm256_set1_pd( u[ i ]);
  }
}
static inline void accumulate_q( FLOAT_PHYS* q,
  const __m256d vS, const __m256d* vG, const int Nc ){
#if 0
  for(int i= 0; i<Nc; i++){
    FLOAT_PHYS VECALIGNED SG[4];
    _mm256_store_pd( SG, vS * vG[ i ] );
    q[i]+= SG[0]+SG[1]+SG[2];
  }
#else
  FLOAT_PHYS VECALIGNED SG[4];
  for(int i= 0; i< 4; i++){// This may be a tiny bit faster unrolled.
    _mm256_store_pd( SG, vS * vG[ i ] );
    q[i]+= SG[0]+SG[1]+SG[2];
  }
  if(Nc>4){
    for(int i= 4; i<10; i++){
      _mm256_store_pd( SG, vS * vG[ i ] );
      q[i]+= SG[0]+SG[1]+SG[2];
    }
    if(Nc>10){
      for(int i=10; i<20; i++){
        _mm256_store_pd( SG, vS * vG[ i ] );
        q[i]+= SG[0]+SG[1]+SG[2];
      }
    }
  }
#endif
}
// Isotropic Elastic ----------------------------------------------------------
static inline void compute_iso_s(FLOAT_PHYS* S,
  const FLOAT_PHYS* H, const FLOAT_PHYS* C, const FLOAT_PHYS dw ){// Scalar ver.
  const FLOAT_PHYS Cdw[3] = { C[0]*dw, C[1]*dw, C[2]*dw };
  //
  S[0]= Cdw[0]* H[0] + Cdw[1]* H[4] + Cdw[1]* H[8];//Sxx
  S[4]= Cdw[1]* H[0] + Cdw[0]* H[4] + Cdw[1]* H[8];//Syy
  S[8]= Cdw[1]* H[0] + Cdw[1]* H[4] + Cdw[0]* H[8];//Szz
  //
  S[1]=( H[1] + H[3] )*Cdw[2];// S[3]= S[1];//Sxy Syx
  S[5]=( H[5] + H[7] )*Cdw[2];// S[7]= S[5];//Syz Szy
  S[2]=( H[2] + H[6] )*Cdw[2];// S[6]= S[2];//Sxz Szx
  //------------------------------------------------------- 18+9 = 27 FLOP
  S[3]=S[1]; S[7]=S[5]; S[6]=S[2];
}
static inline void compute_iso_s(__m256d* vA,// in-place version
  const FLOAT_PHYS lambda, const FLOAT_PHYS mu ){
#ifdef HAS_AVX2
  // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
  // Structure of S and H
  //  3   2   1     0
  // sxx sxy sxz | sxy
  // sxy syy syz | sxz
  // sxz syz szz | ---
  __m256d Ssum=_mm256_setzero_pd();
  {
  const __m256d z0 =_mm256_set_pd(0.0,1.0,1.0,1.0);
  const __m256d ml =_mm256_set_pd(lambda,mu,mu,mu);
  vA[0]=_mm256_permute4x64_pd( vA[0]*z0,_MM_SHUFFLE(0,2,3,1) ); Ssum+= vA[0]*ml;
  vA[1]=_mm256_permute4x64_pd( vA[1]*z0,_MM_SHUFFLE(1,3,2,0) ); Ssum+= vA[1]*ml;
  vA[2]=_mm256_permute4x64_pd( vA[2]*z0,_MM_SHUFFLE(2,0,1,3) ); Ssum+= vA[2]*ml;
  }
  //      3   2   1   0
  //     sxy 0.0 sxz sxx
  //     syx syz 0.0 syy
  //     0.0 szy szx szz
  //
  // mu*(sxy syz sxz)trace(H)*lambda : Ssum
  {
  const __m256d m2=_mm256_set_pd(2.0*mu,0.0,0.0,0.0);
  //      3   2   1   0
  // mu*(sxy 0.0 sxz)sxx*2*mu+lambda*trace(H)
  // mu*(sxy syz 0.0)syy*2*mu+lambda*trace(H)
  // mu*(0.0 syz sxz)szz*2*mu+lambda*trace(H)
#if 0
  printf("S step 2\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
  vA[0]=_mm256_permute4x64_pd( Ssum + vA[0]*m2,_MM_SHUFFLE(3,2,0,3) );
  vA[1]=_mm256_permute4x64_pd( Ssum + vA[1]*m2,_MM_SHUFFLE(3,1,3,0) );
  vA[2]=_mm256_permute4x64_pd( Ssum + vA[2]*m2,_MM_SHUFFLE(3,3,1,2) );
  }
#if 0
  printf("S step 3\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
#else
// Does not have avx2 support
  FLOAT_PHYS VECALIGNED fS[12];
  _mm256_store_pd(&fS[0],vA[0]);
  _mm256_store_pd(&fS[4],vA[1]);
  _mm256_store_pd(&fS[8],vA[2]);
  {
  const FLOAT_PHYS tr = (fS[0]+fS[5]+fS[10]) * lambda;
  const __m256d mw= _mm256_set1_pd(mu);
  _mm256_store_pd( &fS[0], mw * vA[0] );// sxx sxy sxz | syx
  _mm256_store_pd( &fS[4], mw * vA[1] );// syx syy syz | szx
  _mm256_store_pd( &fS[8], mw * vA[2] );// szx szy szz | ---
  fS[0]=2.0*fS[0]+tr; fS[5]=2.0*fS[5]+tr; fS[10]=2.0*fS[10]+tr;
  }
  fS[1]+= fS[4];
  fS[2]+= fS[8];
  fS[6]+= fS[9];
  fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
  vA[0] = _mm256_load_pd(&fS[0]); // [a3 a2 a1 a0]
  vA[1] = _mm256_load_pd(&fS[4]); // [a6 a5 a4 a3]
  vA[2] = _mm256_load_pd(&fS[8]); // [a9 a8 a7 a6]
#endif
}static inline void compute_iso_s(__m256d* vS, const __m256d* vH,
  const FLOAT_PHYS lambda, const FLOAT_PHYS mu ){
#ifdef HAS_AVX2
  // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
  // Structure of S and H
  //  3   2   1     0
  // sxx sxy sxz | sxy
  // sxy syy syz | sxz
  // sxz syz szz | ---
  __m256d Ssum=_mm256_setzero_pd();
  {
  const __m256d z0 =_mm256_set_pd(0.0,1.0,1.0,1.0);
  const __m256d ml =_mm256_set_pd(lambda,mu,mu,mu);
  vS[0]=_mm256_permute4x64_pd( vH[0]*z0, _MM_SHUFFLE(0,2,3,1) );
  Ssum+= vS[0]*ml;
  vS[1]=_mm256_permute4x64_pd( vH[1]*z0, _MM_SHUFFLE(1,3,2,0) );
  Ssum+= vS[1]*ml;
  vS[2]=_mm256_permute4x64_pd( vH[2]*z0, _MM_SHUFFLE(2,0,1,3) );
  Ssum+= vS[2]*ml;
  }
  //      3   2   1   0
  //     sxy 0.0 sxz sxx
  //     sxy syz 0.0 syy
  //     0.0 syz sxz szz
  //
  // mu*(sxy syz sxz)trace(H)*lambda : Ssum
#if 0
  printf("H\n");
  print_m256(H[0]); print_m256(H[1]); print_m256(H[2]);
  printf("S step 1\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
  printf("Ssum\n");
  print_m256( Ssum );
#endif
  {
  const __m256d m2=_mm256_set_pd(2.0*mu,0.0,0.0,0.0);
#if 0
  vS[0]= Ssum + vS[0]*m2;
  vS[1]= Ssum + vS[1]*m2;
  vS[2]= Ssum + vS[2]*m2;
#endif
  //      3   2   1   0
  // mu*(sxy 0.0 sxz)sxx*2*mu+lambda*trace(H)
  // mu*(sxy syz 0.0)syy*2*mu+lambda*trace(H)
  // mu*(0.0 syz sxz)szz*2*mu+lambda*trace(H)
#if 0
  printf("S step 2\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
  vS[0]=_mm256_permute4x64_pd( Ssum + vS[0]*m2, _MM_SHUFFLE(3,2,0,3) );
  vS[1]=_mm256_permute4x64_pd( Ssum + vS[1]*m2, _MM_SHUFFLE(3,1,3,0) );
  vS[2]=_mm256_permute4x64_pd( Ssum + vS[2]*m2, _MM_SHUFFLE(3,3,1,2) );
  }
#if 0
  printf("S step 3\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
#else
// Does not have avx2 support
  FLOAT_PHYS VECALIGNED fS[12];
  _mm256_store_pd(&fS[0],vH[0]);
  _mm256_store_pd(&fS[4],vH[1]);
  _mm256_store_pd(&fS[8],vH[2]);
  {
  const FLOAT_PHYS tr = (fS[0]+fS[5]+fS[10]) * lambda;
  const __m256d mw= _mm256_set1_pd(mu);
  _mm256_store_pd( &fS[0], mw * vH[0] );// sxx sxy sxz | syx
  _mm256_store_pd( &fS[4], mw * vH[1] );// syx syy syz | szx
  _mm256_store_pd( &fS[8], mw * vH[2] );// szx szy szz | ---
  fS[0]=2.0*fS[0]+tr; fS[5]=2.0*fS[5]+tr; fS[10]=2.0*fS[10]+tr;
  }
  fS[1]+= fS[4];
  fS[2]+= fS[8];
  fS[6]+= fS[9];
  fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
  vS[0] = _mm256_load_pd(&fS[0]); // [a3 a2 a1 a0]
  vS[1] = _mm256_load_pd(&fS[4]); // [a6 a5 a4 a3]
  vS[2] = _mm256_load_pd(&fS[8]); // [a9 a8 a7 a6]
#endif
}
static inline void compute_g_h( FLOAT_PHYS* G, __m256d* vH,// line 273
  const int Nc, const __m256d* vJ, const FLOAT_PHYS* sg, const FLOAT_PHYS* u ){
  vH[0]=_mm256_setzero_pd(); vH[1]=_mm256_setzero_pd(); vH[2]=_mm256_setzero_pd();
#if 1
  for(int i= 0; i<Nc; i++){// line 277
    const __m256d vg
      = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
      + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
      + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
    vH[0]+= vg *_mm256_set1_pd( u[3* i+0 ]);
    vH[1]+= vg *_mm256_set1_pd( u[3* i+1 ]);
    vH[2]+= vg *_mm256_set1_pd( u[3* i+2 ]);
    _mm256_store_pd(& G[4* i ], vg );
  }
#else
  for(int i= 0; i< 4; i++){// Unrolled runs a bit slower.
    const __m256d vg
      = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
      + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
      + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
    vH[0]+= vg *_mm256_set1_pd( u[3* i+0 ]);
    vH[1]+= vg *_mm256_set1_pd( u[3* i+1 ]);
    vH[2]+= vg *_mm256_set1_pd( u[3* i+2 ]);
    _mm256_store_pd(& G[4* i ], vg ); }
  if( Nc> 4 ){
    for(int i= 4; i<10; i++){
      const __m256d vg
        = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
        + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
        + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
      vH[0]+= vg *_mm256_set1_pd( u[3* i+0 ]);
      vH[1]+= vg *_mm256_set1_pd( u[3* i+1 ]);
      vH[2]+= vg *_mm256_set1_pd( u[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], vg ); }
    if( Nc>10 ){
      for(int i=10; i<20; i++){
        const __m256d vg
          = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
          + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
          + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
        vH[0]+= vg *_mm256_set1_pd( u[3* i+0 ]);
        vH[1]+= vg *_mm256_set1_pd( u[3* i+1 ]);
        vH[2]+= vg *_mm256_set1_pd( u[3* i+2 ]);
        _mm256_store_pd(& G[4* i ], vg ); }
    }
  }
#endif
}//was line 320
// DMAT Elastic ---------------------------------------------------------------
static inline void compute_dmv_s(FLOAT_PHYS* S,
  const FLOAT_PHYS* H,const FLOAT_PHYS* D,const FLOAT_PHYS dw ){
  FLOAT_PHYS HV[6], SV[6];
  HV[0]=H[0];
  HV[1]=H[4];
  HV[2]=H[8];
  HV[3]=H[1]+H[3];
  HV[4]=H[2]+H[6];
  HV[5]=H[5]+H[7];
  for(int i=0;i<6;i++){ SV[i]=0.0;
    for(int j=0;j<6;j++){
      SV[i]+= D[6*i+j] * HV[j];
    } SV[i]*= dw;
  }
  //for(int i=0;i<6;i++){ SV[i]*=dw; }
  S[0]=SV[0]; S[1]=SV[3]; S[2]=SV[4];
  S[3]=SV[3]; S[4]=SV[1]; S[5]=SV[5];
  S[6]=SV[4]; S[7]=SV[5]; S[8]=SV[2];
}
static inline void compute_dmv_s(__m256d* vS,
  const __m256d* vH,const FLOAT_PHYS* D,const FLOAT_PHYS dw ){
  __m256d s0=_mm256_setzero_pd(), s1=_mm256_setzero_pd();
  {// Scope H and h.
  FLOAT_PHYS VECALIGNED H[12];
  _mm256_store_pd(&H[0],vH[0]);
  _mm256_store_pd(&H[4],vH[1]);
  _mm256_store_pd(&H[8],vH[2]);
  __m256d h;
  h=_mm256_set1_pd( H[0] );// Sxx
  s0+=_mm256_load_pd(&D[0]) * h; s1+=_mm256_load_pd(&D[4]) * h;
  h=_mm256_set1_pd( H[5] );// Syy
  s0+=_mm256_load_pd(&D[8]) * h; s1+=_mm256_load_pd(&D[12]) * h;
  h=_mm256_set1_pd( H[10] );// Szz
  s0+=_mm256_load_pd(&D[16]) * h; s1+=_mm256_load_pd(&D[20]) * h;
  //
  h=_mm256_set1_pd( H[1] )+_mm256_set1_pd( H[4] );// Sxy + Syx
  s0+=_mm256_load_pd(&D[24]) * h; s1+=_mm256_load_pd(&D[28]) * h;
  h=_mm256_set1_pd( H[2] )+_mm256_set1_pd( H[8] );// Sxz + Szx
  s0+=_mm256_load_pd(&D[32]) * h; s1+=_mm256_load_pd(&D[36]) * h;
  h=_mm256_set1_pd( H[6] )+_mm256_set1_pd( H[9] );// Syz + Szy
  s0+=_mm256_load_pd(&D[40]) * h; s1+=_mm256_load_pd(&D[44]) * h;
  }{//Scope dw1.
  const __m256d dw1=_mm256_set1_pd( dw );
  s0*= dw1; s1*= dw1;
  }{// Scope shfX or V
  // rearrange voigt vector [s0,s1] back to a padded tensor vS
#ifdef HAS_AVX2
  // Sxx Syy Szz Sxy Sxz Syz 0.0 0.0
  //  0   1   2   3   4   5   6   7  : mask
#ifdef __INTEL_COMPILER
  FLOAT_PHYS VECALIGNED V[8];
  _mm256_store_pd(&V[0],s0);
  _mm256_store_pd(&V[4],s1);
  vS[0] =_mm256_set_pd(0.0, V[4],V[3],V[0]);
  vS[1] =_mm256_set_pd(0.0, V[5],V[1],V[3]);
  vS[2] =_mm256_set_pd(0.0, V[2],V[5],V[4]);
#else
  const __m256i shf0 = { 0,3,4, 7};
  const __m256i shf1 = { 3,1,5, 7};
  const __m256i shf2 = { 4,5,2, 7};
  vS[0] =__builtin_shuffle( s0,s1,shf0);
  vS[1] =__builtin_shuffle( s0,s1,shf1);
  vS[2] =__builtin_shuffle( s0,s1,shf2);
#endif
#else
  FLOAT_PHYS VECALIGNED V[8];
  _mm256_store_pd(&V[0],s0);
  _mm256_store_pd(&V[4],s1);
  vS[0] =_mm256_set_pd(0.0, V[4],V[3],V[0]);
  vS[1] =_mm256_set_pd(0.0, V[5],V[1],V[3]);
  vS[2] =_mm256_set_pd(0.0, V[2],V[5],V[4]);
#endif
#if 0
  // Should work for clang
  vS[0] =__builtin_shuffle_vector( s0,s1,0,3,4, 7);
  vS[1] =__builtin_shuffle_vector( s0,s1,3,1,5, 7);
  vS[2] =__builtin_shuffle_vector( s0,s1,4,5,2, 7);
#endif
  }
}
static inline void compute_dmv_s(__m256d* vA,// H in, S out
  const FLOAT_PHYS* D,const FLOAT_PHYS dw ){
  __m256d s0=_mm256_setzero_pd(), s1=_mm256_setzero_pd();
  {// Scope H and h.
  FLOAT_PHYS VECALIGNED H[12];
  _mm256_store_pd(&H[0],vA[0]);
  _mm256_store_pd(&H[4],vA[1]);
  _mm256_store_pd(&H[8],vA[2]);
  __m256d h;
  h=_mm256_set1_pd( H[0] );// Sxx
  s0+=_mm256_load_pd(&D[0]) * h; s1+=_mm256_load_pd(&D[4]) * h;
  h=_mm256_set1_pd( H[5] );// Syy
  s0+=_mm256_load_pd(&D[8]) * h; s1+=_mm256_load_pd(&D[12]) * h;
  h=_mm256_set1_pd( H[10] );// Szz
  s0+=_mm256_load_pd(&D[16]) * h; s1+=_mm256_load_pd(&D[20]) * h;
  //
  h=_mm256_set1_pd( H[1] )+_mm256_set1_pd( H[4] );// Sxy + Syx
  s0+=_mm256_load_pd(&D[24]) * h; s1+=_mm256_load_pd(&D[28]) * h;
  h=_mm256_set1_pd( H[2] )+_mm256_set1_pd( H[8] );// Sxz + Szx
  s0+=_mm256_load_pd(&D[32]) * h; s1+=_mm256_load_pd(&D[36]) * h;
  h=_mm256_set1_pd( H[6] )+_mm256_set1_pd( H[9] );// Syz + Szy
  s0+=_mm256_load_pd(&D[40]) * h; s1+=_mm256_load_pd(&D[44]) * h;
  }{//Scope dw1.
  const __m256d dw1=_mm256_set1_pd( dw );
  s0*= dw1; s1*= dw1;
  }{// Scope shfX or V.
  // rearrange voigt vector [s0,s1] back to a padded tensor vA
#ifdef HAS_AVX2
  // Sxx Syy Szz Sxy Sxz Syz 0.0 0.0
  //  0   1   2   3   4   5   6   7  : mask
#ifdef __INTEL_COMPILER
  FLOAT_PHYS VECALIGNED V[8];
  _mm256_store_pd(&V[0],s0);
  _mm256_store_pd(&V[4],s1);
  vA[0] =_mm256_set_pd(0.0, V[4],V[3],V[0]);
  vA[1] =_mm256_set_pd(0.0, V[5],V[1],V[3]);
  vA[2] =_mm256_set_pd(0.0, V[2],V[5],V[4]);
#else
  const __m256i shf0 = { 0,3,4, 7};
  const __m256i shf1 = { 3,1,5, 7};
  const __m256i shf2 = { 4,5,2, 7};
  vA[0] =__builtin_shuffle( s0,s1,shf0);
  vA[1] =__builtin_shuffle( s0,s1,shf1);
  vA[2] =__builtin_shuffle( s0,s1,shf2);
#endif
#else
  FLOAT_PHYS VECALIGNED V[8];
  _mm256_store_pd(&V[0],s0);
  _mm256_store_pd(&V[4],s1);
  vA[0] =_mm256_set_pd(0.0, V[4],V[3],V[0]);
  vA[1] =_mm256_set_pd(0.0, V[5],V[1],V[3]);
  vA[2] =_mm256_set_pd(0.0, V[2],V[5],V[4]);
#endif
#if 0
  // Should work for clang
  vS[0] =__builtin_shuffle_vector( s0,s1,0,3,4, 7);
  vS[1] =__builtin_shuffle_vector( s0,s1,3,1,5, 7);
  vS[2] =__builtin_shuffle_vector( s0,s1,4,5,2, 7);
#endif
  }
}
// Isotropic Thermoelastic ----------------------------------------------------
static inline void thermelast_g_h_t( FLOAT_PHYS* G, __m256d* vH,
  const int Nc, const __m256d* vJ, const FLOAT_PHYS* sg, const FLOAT_PHYS* sf,
  const FLOAT_PHYS* u ){
  vH[0]=_mm256_setzero_pd(); vH[1]=_mm256_setzero_pd();
  vH[2]=_mm256_setzero_pd(); vH[3]=_mm256_setzero_pd();
  for(int i= 0; i<Nc; i++){
    const __m256d vg
      = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
      + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
      + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
    vH[0]+= vg *_mm256_set1_pd( u[4* i+0 ]);
    vH[1]+= vg *_mm256_set1_pd( u[4* i+1 ]);
    vH[2]+= vg *_mm256_set1_pd( u[4* i+2 ]);
    _mm256_store_pd(& G[4* i ], vg );
    //vH[3]+= _mm256_set_pd( G[4*i],G[4*i+1],G[4*i+2], sf[i] )
    vH[3]+= _mm256_set_pd( sf[i], G[4*i+2],G[4*i+1],G[4*i] )
          *_mm256_set1_pd( u[4* i+3 ] );// ( Tx,Ty,Tz, T )
  }
}
// Nonlinear isotropic Elastic ------------------------------------------------
static inline void compute_g_p_h(
  FLOAT_PHYS* G, __m256d* vP, __m256d* vH,
  const int Nc, const  __m256d* vJ,
  const FLOAT_PHYS* sg, const FLOAT_PHYS* p, const FLOAT_PHYS* u ){
  vH[0]=_mm256_setzero_pd(); vH[1]=_mm256_setzero_pd(); vH[2]=_mm256_setzero_pd(),
  vP[0]=_mm256_setzero_pd(); vP[1]=_mm256_setzero_pd(); vP[2]=_mm256_setzero_pd();
  //
  for(int i= 0; i<Nc; i++){//was line 329
      const __m256d vg
        = vJ[0] *_mm256_set1_pd( sg[3* i+0 ])
        + vJ[1] *_mm256_set1_pd( sg[3* i+1 ])
        + vJ[2] *_mm256_set1_pd( sg[3* i+2 ]);
      vH[0]+= vg *_mm256_set1_pd(  u[3* i+0 ]);
      vH[1]+= vg *_mm256_set1_pd(  u[3* i+1 ]);
      vH[2]+= vg *_mm256_set1_pd(  u[3* i+2 ]);
      vP[0]+= vg *_mm256_set1_pd(  p[3* i+0 ]);
      vP[1]+= vg *_mm256_set1_pd(  p[3* i+1 ]);
      vP[2]+= vg *_mm256_set1_pd(  p[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], vg );
  }
}
#endif
