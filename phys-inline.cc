#ifndef INCLUDED_PHYS_INLINE_C
#define INCLUDED_PHYS_INLINE_C
#include "femera.h"
#include <immintrin.h>
//FIXME These inline intrinsics functions should be in the class where used.
#if 0
static inline void print_m256(const __m256d v){// Debugging register print
  double V[4];
  _mm256_store_pd(&V[0],v);
  printf("%9.2e %9.2e %9.2e %9.2e\n",V[0],V[1],V[2],V[3]);
}
#endif
// Physics Intrinsics ---------------------------------------------------------
static inline void accumulate_f( __m256d* vf,
  const __m256d* vS, const FLOAT_PHYS* G, const int Nc ){
  for(int i= 0; i< 4; i++){
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
// Orthotropic intrinsics -----------------------------------------------------
static inline void rotate_g_h(
  FLOAT_PHYS* G, __m256d* vH,
  const int Nc, const __m256d* J,
  const FLOAT_PHYS* sg, const FLOAT_PHYS* R, const FLOAT_PHYS* u ){
  __m256d a036=_mm256_setzero_pd(), a147=_mm256_setzero_pd(),
    a258=_mm256_setzero_pd();
#if 1
  for(int i= 0; i<Nc; i++){
      const __m256d g
        = J[0] *_mm256_set1_pd( sg[3* i+0 ])
        + J[1] *_mm256_set1_pd( sg[3* i+1 ])
        + J[2] *_mm256_set1_pd( sg[3* i+2 ]);
      a036+= g *_mm256_set1_pd(  u[3* i+0 ]);
      a147+= g *_mm256_set1_pd(  u[3* i+1 ]);
      a258+= g *_mm256_set1_pd(  u[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], g );
#else
  /* Keep line numbers for now to match old intel advisor results.
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   * 
   */
#endif
  }// line 78
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
}
static inline void compute_ort_s_voigt(FLOAT_PHYS* S, const __m256d* vH,
  const __m256d* vC, const FLOAT_PHYS dw){// line 95
  FLOAT_PHYS VECALIGNED H[12];
  _mm256_store_pd( &H[0], vH[0] );
  _mm256_store_pd( &H[4], vH[1] );
  _mm256_store_pd( &H[8], vH[2] );
  //__m256d s048 =//Vectorized calc for diagonal of S
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
// Isotropic intrinsics -------------------------------------------------------
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
  vA[0]=_mm256_permute4x64_pd( vA[0]*z0,_MM_SHUFFLE(0,2,3,1) );
  Ssum+= vA[0]*ml;
  vA[1]=_mm256_permute4x64_pd( vA[1]*z0,_MM_SHUFFLE(1,3,2,0) );
  Ssum+= vA[1]*ml;
  vA[2]=_mm256_permute4x64_pd( vA[2]*z0,_MM_SHUFFLE(2,0,1,3) );
  Ssum+= vA[2]*ml;
  }
  //      3   2   1   0
  //     sxy 0.0 sxz sxx
  //     sxy syz 0.0 syy
  //     0.0 syz sxz szz
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
static inline void compute_g_h( FLOAT_PHYS* G, __m256d* H,
  const int Nc, const __m256d* J, const FLOAT_PHYS* sg, const FLOAT_PHYS* u ){
  H[0]=_mm256_setzero_pd(); H[1]=_mm256_setzero_pd(); H[2]=_mm256_setzero_pd();
  //FIXED Change calls to pass Nc instead of Ne.
  for(int i= 0; i<Nc; i++){// line 277
#if 1
      const __m256d g
        = J[0] *_mm256_set1_pd( sg[3* i+0 ])
        + J[1] *_mm256_set1_pd( sg[3* i+1 ])
        + J[2] *_mm256_set1_pd( sg[3* i+2 ]);
      H[0]+= g *_mm256_set1_pd(  u[3* i+0 ]);
      H[1]+= g *_mm256_set1_pd(  u[3* i+1 ]);
      H[2]+= g *_mm256_set1_pd(  u[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], g );
#else
    { const __m256d g// line 288
        = J[0] *_mm256_set1_pd(isp[i+0])
        + J[1] *_mm256_set1_pd(isp[i+1])
        + J[2] *_mm256_set1_pd(isp[i+2]);
      H[0]+= g *_mm256_set1_pd(  u[i+0]);
      H[1]+= g *_mm256_set1_pd(  u[i+1]);
      H[2]+= g *_mm256_set1_pd(  u[i+2]);
      _mm256_store_pd(&G[ig],g);
      ig+=4;
    }if(i<(Ne-5)){
      const __m256d g
        = J[0] *_mm256_set1_pd(isp[i+3])
        + J[1] *_mm256_set1_pd(isp[i+4])
        + J[2] *_mm256_set1_pd(isp[i+5]);
      H[0]+= g *_mm256_set1_pd(  u[i+3]);
      H[1]+= g *_mm256_set1_pd(  u[i+4]);
      H[2]+= g *_mm256_set1_pd(  u[i+5]);
      _mm256_store_pd(&G[ig],g);
      ig+=4;
    }if(i<(Ne-8)){
      const __m256d g
        = J[0] *_mm256_set1_pd(isp[i+6])
        + J[1] *_mm256_set1_pd(isp[i+7])
        + J[2] *_mm256_set1_pd(isp[i+8]);
      H[0]+= g *_mm256_set1_pd(  u[i+6]);
      H[1]+= g *_mm256_set1_pd(  u[i+7]);
      H[2]+= g *_mm256_set1_pd(  u[i+8]);
      _mm256_store_pd(&G[ig],g);
      ig+=4;
    }
#endif
  }
}// line 320
// Nonlinear isotropic intrinsics ---------------------------------------------
static inline void compute_g_p_h(
  FLOAT_PHYS* G, __m256d* P, __m256d* H,
  const int Nc, const  __m256d* J,
  const FLOAT_PHYS* sg, const FLOAT_PHYS* p, const FLOAT_PHYS* u ){
  H[0]=_mm256_setzero_pd(); H[1]=_mm256_setzero_pd(); H[2]=_mm256_setzero_pd(),
  P[0]=_mm256_setzero_pd(); P[1]=_mm256_setzero_pd(); P[2]=_mm256_setzero_pd();
  //
  for(int i= 0; i<Nc; i++){// line 329
      const __m256d g
        = J[0] *_mm256_set1_pd( sg[3* i+0 ])
        + J[1] *_mm256_set1_pd( sg[3* i+1 ])
        + J[2] *_mm256_set1_pd( sg[3* i+2 ]);
      H[0]+= g *_mm256_set1_pd(  u[3* i+0 ]);
      H[1]+= g *_mm256_set1_pd(  u[3* i+1 ]);
      H[2]+= g *_mm256_set1_pd(  u[3* i+2 ]);
      P[0]+= g *_mm256_set1_pd(  p[3* i+0 ]);
      P[1]+= g *_mm256_set1_pd(  p[3* i+1 ]);
      P[2]+= g *_mm256_set1_pd(  p[3* i+2 ]);
      _mm256_store_pd(& G[4* i ], g );
  }
}
//FIXME Remove these later ====================================================
#if 0
static inline void compute_ort_s_voigt(__m256d* vS, const __m256d* vH,
  const __m256d* vC, const FLOAT_PHYS dw ){
  vS[0] =//Vectorized calc for diagonal of S
    _mm256_mul_pd(_mm256_set1_pd(dw),
      _mm256_add_pd(_mm256_mul_pd(vC[0],
        _mm256_permute4x64_pd( vH[0], _MM_SHUFFLE(0,0,0,0) )),
        _mm256_add_pd(_mm256_mul_pd(vC[1],
          _mm256_permute4x64_pd( vH[1], _MM_SHUFFLE(1,1,1,1) )),
          _mm256_mul_pd(vC[2],
            _mm256_permute4x64_pd( vH[2], _MM_SHUFFLE(2,2,2,2) ))
        )));
  {
  const __m256d d0 =_mm256_set_pd(0.0,dw,dw,dw);
  vS[1] =_mm256_setzero_pd();
  vS[1]+=_mm256_permute4x64_pd( vH[0]*d0, _MM_SHUFFLE(0,2,3,1) )*vC[3];
  vS[1]+=_mm256_permute4x64_pd( vH[1]*d0, _MM_SHUFFLE(1,3,2,0) )*vC[3];
  vS[1]+=_mm256_permute4x64_pd( vH[2]*d0, _MM_SHUFFLE(2,0,1,3) )*vC[3];
  vS[1] =_mm256_permute4x64_pd( vS[1]   , _MM_SHUFFLE(3,1,2,0) );
  }//NOTE The shear terms are arranged differently.
  // sxx syy szz ---   sxy sxz syz ---
}
#endif
#if 0
static inline void rotate_s_voigt( __m256d* vS, const __m256d* vR ){
  const __m256d s0 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(0,0,0,0) );// S[0]
  const __m256d s1 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(0,0,0,0) );// S[4]
  const __m256d s2 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(1,1,1,1) );// S[5]
  const __m256d s4 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(1,1,1,1) );// S[1]
  const __m256d s5 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(2,2,2,2) );// S[6]
  const __m256d s8 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(2,2,2,2) );// S[2]
  vS[0]=_mm256_add_pd(_mm256_mul_pd(vR[0],s0),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s1),
      _mm256_mul_pd(vR[2],s2)));
  vS[1]=_mm256_add_pd(_mm256_mul_pd(vR[0],s1),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s4),
      _mm256_mul_pd(vR[2],s5)));
  vS[2]=_mm256_add_pd(_mm256_mul_pd(vR[0],s2),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s5),
      _mm256_mul_pd(vR[2],s8)));
}
#endif

# if 0
//FIXME Remove these later ====================================================
//FIXME These are used by elas-plkh-iso3-ref
//FIXME Refactor stress and strain tensors from double* to __m256d*
static inline void compute_g_p_h(
  FLOAT_PHYS* G, FLOAT_PHYS* P, FLOAT_PHYS* H,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* p, const FLOAT_PHYS* u ){
  __m256d
    a036=_mm256_setzero_pd(),a147=_mm256_setzero_pd(),a258=_mm256_setzero_pd(),
    b036=_mm256_setzero_pd(),b147=_mm256_setzero_pd(),b258=_mm256_setzero_pd();
  int ig=0;
  for(int i=0; i<Ne; i+=9){
    //__m256d p0,p1,p2,p3,p4,p5,p6,p7,p8;
    //__m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    //__m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    const __m256d is0= _mm256_set1_pd(isp[i+0]);
    const __m256d is1= _mm256_set1_pd(isp[i+1]);
    const __m256d is2= _mm256_set1_pd(isp[i+2]);
    const __m256d g0 = _mm256_add_pd(_mm256_mul_pd(J[0],is0),
      _mm256_add_pd(_mm256_mul_pd(J[1],is1),
        _mm256_mul_pd(J[2],is2)));
    const __m256d u0 = _mm256_set1_pd(  u[i+0]);
    const __m256d u1 = _mm256_set1_pd(  u[i+1]);
    const __m256d u2 = _mm256_set1_pd(  u[i+2]);
    a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0));
    a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1));
    a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
    const __m256d p0 = _mm256_set1_pd(  p[i+0]);
    const __m256d p1 = _mm256_set1_pd(  p[i+1]);
    const __m256d p2 = _mm256_set1_pd(  p[i+2]);
    b036 = _mm256_add_pd(b036, _mm256_mul_pd(g0,p0));
    b147 = _mm256_add_pd(b147, _mm256_mul_pd(g0,p1));
    b258 = _mm256_add_pd(b258, _mm256_mul_pd(g0,p2));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      const __m256d is3= _mm256_set1_pd(isp[i+3]);
      const __m256d is4= _mm256_set1_pd(isp[i+4]);
      const __m256d is5= _mm256_set1_pd(isp[i+5]);
      const __m256d g1 = _mm256_add_pd(_mm256_mul_pd(J[0],is3),
        _mm256_add_pd(_mm256_mul_pd(J[1],is4),
          _mm256_mul_pd(J[2],is5)));
      const __m256d u3 = _mm256_set1_pd(  u[i+3]);
      const __m256d u4 = _mm256_set1_pd(  u[i+4]);
      const __m256d u5 = _mm256_set1_pd(  u[i+5]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
      const __m256d p3 = _mm256_set1_pd(  p[i+3]);
      const __m256d p4 = _mm256_set1_pd(  p[i+4]);
      const __m256d p5 = _mm256_set1_pd(  p[i+5]);
      b036 = _mm256_add_pd(b036, _mm256_mul_pd(g1,p3));
      b147 = _mm256_add_pd(b147, _mm256_mul_pd(g1,p4));
      b258 = _mm256_add_pd(b258, _mm256_mul_pd(g1,p5));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d is6= _mm256_set1_pd(isp[i+6]);
      const __m256d is7= _mm256_set1_pd(isp[i+7]);
      const __m256d is8= _mm256_set1_pd(isp[i+8]);
      const __m256d g2 = _mm256_add_pd(_mm256_mul_pd(J[0],is6),
        _mm256_add_pd(_mm256_mul_pd(J[1],is7),
          _mm256_mul_pd(J[2],is8)));
      const __m256d u6 = _mm256_set1_pd(  u[i+6]);
      const __m256d u7 = _mm256_set1_pd(  u[i+7]);
      const __m256d u8 = _mm256_set1_pd(  u[i+8]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g2,u6));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
      const __m256d p6 = _mm256_set1_pd(  p[i+6]);
      const __m256d p7 = _mm256_set1_pd(  p[i+7]);
      const __m256d p8 = _mm256_set1_pd(  p[i+8]);
      b036 = _mm256_add_pd(b036,_mm256_mul_pd(g2,p6));
      b147 = _mm256_add_pd(b147, _mm256_mul_pd(g2,p7));
      b258 = _mm256_add_pd(b258, _mm256_mul_pd(g2,p8));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  _mm256_store_pd(&H[0],a036);
  _mm256_store_pd(&H[4],a147);
  _mm256_store_pd(&H[8],a258);
  _mm256_store_pd(&P[0],b036);
  _mm256_store_pd(&P[4],b147);
  _mm256_store_pd(&P[8],b258);
}
static inline void compute_iso_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS C2, const __m256d c0,const __m256d c1,const __m256d c2,
  const FLOAT_PHYS dw){
  { // Scope vector registers
    __m256d s048;
    s048= _mm256_mul_pd(_mm256_set1_pd(dw),
      _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])),
        _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[5])),
          _mm256_mul_pd(c2,_mm256_set1_pd(H[10])))));
     _mm256_store_pd(&S[0], s048);
  } // end scoping unit
  S[5]=S[1]; S[10]=S[2];// Move the diagonal to their correct locations
  // Then, do the rest. Dunno if this is faster...
  S[1]=( H[1] + H[4] )*C2*dw;// S[3]= S[1];//Sxy Syx
  S[2]=( H[2] + H[8] )*C2*dw;// S[6]= S[2];//Sxz Szx
  S[6]=( H[6] + H[9] )*C2*dw;// S[7]= S[5];//Syz Szy
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
}
#if 0
static inline void compute_iso_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS lambda, const FLOAT_PHYS mu ){
  // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
#if 0
  for(int i=0; i<3; i++){
    for(int j=0; j<4; j++){ S[4* i+j] = H[4* i+j] * mu * dw; } }
  const FLOAT_PHYS tr = (H[0]+H[5]+H[10]) * lambda * dw;
  S[0]=2.0*S[0]+tr; S[5]=2.0*S[5]+tr; S[10]=2.0*S[10]+tr;
  S[1]+= S[4];
  S[2]+= S[8];
  S[6]+= S[9];
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
#else
  //FIXME These are all about the same speed as above, slower than ortho.
  const __m256d mw= _mm256_set1_pd(mu);
  _mm256_store_pd( &S[0], mw * _mm256_load_pd(&H[0]) );// sxx sxy sxz | sxy
  _mm256_store_pd( &S[4], mw * _mm256_load_pd(&H[4]) );// sxy syy syz | sxz
  _mm256_store_pd( &S[8], mw * _mm256_load_pd(&H[8]) );// sxz syz szz | ---
  const FLOAT_PHYS tr = (H[0]+H[5]+H[10]) * lambda;
  S[0]=2.0*S[0]+tr; S[5]=2.0*S[5]+tr; S[10]=2.0*S[10]+tr;
  S[1]+= S[4];
  S[2]+= S[8];
  S[6]+= S[9];
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
#endif
}
#endif
static inline void compute_g_h(
  FLOAT_PHYS* G, FLOAT_PHYS* H,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* u ){
  __m256d a036=_mm256_setzero_pd(),
    a147=_mm256_setzero_pd(), a258=_mm256_setzero_pd();
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    //__m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    //__m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    const __m256d is0= _mm256_set1_pd(isp[i+0]);
    const __m256d is1= _mm256_set1_pd(isp[i+1]);
    const __m256d is2= _mm256_set1_pd(isp[i+2]);
    const __m256d g0 = _mm256_add_pd(_mm256_mul_pd(J[0],is0),
      _mm256_add_pd(_mm256_mul_pd(J[1],is1),
        _mm256_mul_pd(J[2],is2)));
    const __m256d u0 = _mm256_set1_pd(  u[i+0]);
    const __m256d u1 = _mm256_set1_pd(  u[i+1]);
    const __m256d u2 = _mm256_set1_pd(  u[i+2]);
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0));
    a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1));
    a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      const __m256d is3= _mm256_set1_pd(isp[i+3]);
      const __m256d is4= _mm256_set1_pd(isp[i+4]);
      const __m256d is5= _mm256_set1_pd(isp[i+5]);
      const __m256d g1 = _mm256_add_pd(_mm256_mul_pd(J[0],is3),
        _mm256_add_pd(_mm256_mul_pd(J[1],is4),
          _mm256_mul_pd(J[2],is5)));
      const __m256d u3 = _mm256_set1_pd(  u[i+3]);
      const __m256d u4 = _mm256_set1_pd(  u[i+4]);
      const __m256d u5 = _mm256_set1_pd(  u[i+5]);
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d is6= _mm256_set1_pd(isp[i+6]);
      const __m256d is7= _mm256_set1_pd(isp[i+7]);
      const __m256d is8= _mm256_set1_pd(isp[i+8]);
      const __m256d g2 = _mm256_add_pd(_mm256_mul_pd(J[0],is6),
        _mm256_add_pd(_mm256_mul_pd(J[1],is7),
          _mm256_mul_pd(J[2],is8)));
      const __m256d u6 = _mm256_set1_pd(  u[i+6]);
      const __m256d u7 = _mm256_set1_pd(  u[i+7]);
      const __m256d u8 = _mm256_set1_pd(  u[i+8]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g2,u6));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  _mm256_store_pd(&H[0],a036);
  _mm256_store_pd(&H[4],a147);
  _mm256_store_pd(&H[8],a258);
}
#endif





#endif