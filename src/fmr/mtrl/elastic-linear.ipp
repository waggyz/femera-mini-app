namespace fmr { namespace mtrl { namespace elastic {
//
  template <typename F> static inline// 75 FLOP
  void elastic::linear_dmat_3d_base
    (F* stress, const F* D, volatile F* H, F* strain_voigt, F* stress_v) {
    //
    strain_voigt [0] = H[0];
    strain_voigt [1] = H[4];
    strain_voigt [2] = H[8];
    strain_voigt [3] = H[5] + H[7];// 1 FLOP
    strain_voigt [4] = H[2] + H[6];// 1 FLOP
    strain_voigt [5] = H[1] + H[3];// 1 FLOP
    //
    for (fmr::Local_int i=0; i<6; ++i) { stress_v[i] = 0.0; }
    for (fmr::Local_int i=0; i < 6; ++i) {
      for (fmr::Local_int j=0; j < 6; ++j) {
        stress_v[i] += D [6*i+j] * strain_voigt [j];// 2*36 = 72
    } }
    stress[0] = stress_v[0]; stress[1] = stress_v[5]; stress[2] = stress_v[4];
    stress[3] = stress_v[5]; stress[4] = stress_v[1]; stress[5] = stress_v[3];
    stress[6] = stress_v[4]; stress[7] = stress_v[3]; stress[8] = stress_v[2];
  }
  template <typename F> static inline// 24 FLOP
  void linear_isotropic_3d_lame
    (F* stress, const F lambda, const F mu, volatile F* H, F* HT) {
    //
    HT [0] = H [0]; HT [1] = H [3]; HT [2] = H [6];
    HT [3] = H [1]; HT [4] = H [4]; HT [5] = H [7];
    HT [6] = H [2]; HT [7] = H [5]; HT [8] = H [8];
    for (fmr::Local_int i=0; i < 9; ++i) {
      stress[i] = mu * (H[i] + HT[i]);// 18 FLOP
    }
    const F lambda_trace = lambda * (H[0] + H[4] + H[8]);//3 FLOP
    //
    stress [0] += lambda_trace;// 1 FLOP
    stress [4] += lambda_trace;// 1 FLOP
    stress [8] += lambda_trace;// 1 FLOP
  }
  template <typename F> static inline// 21 FLOP
  void linear_cubic_3d_scalar_a
    (F* stress, const F c1,const F c2,const F c3, volatile F* H) {
    stress [0] = c1 * H[0] + c2 * H[4] + c2 * H[8];// 5 FLOP
    stress [4] = c2 * H[0] + c1 * H[4] + c2 * H[8];// 5 FLOP
    stress [8] = c2 * H[0] + c2 * H[4] + c1 * H[8];// 5 FLOP
    stress [5] = (H[5] + H[7]) * c3;// 2 FLOP
    stress [2] = (H[2] + H[6]) * c3;// 2 FLOP
    stress [1] = (H[1] + H[3]) * c3;// 2 FLOP
    //
    stress [7] = stress [5];
    stress [6] = stress [2];
    stress [3] = stress [1];
  }
  template <typename F> static inline// 21 FLOP
  void linear_cubic_3d_scalar_b
    (F* stress, const F c1,const F c2,const F c3, volatile F* H, F* stress_v) {
    stress_v [0] = c1 * H[0] + c2 * H[4] + c2 * H[8];// 5 FLOP
    stress_v [1] = c2 * H[0] + c1 * H[4] + c2 * H[8];// 5 FLOP
    stress_v [2] = c2 * H[0] + c2 * H[4] + c1 * H[8];// 5 FLOP
    stress_v [3] = (H[5] + H[7]) * c3;// 2 FLOP
    stress_v [4] = (H[2] + H[6]) * c3;// 2 FLOP
    stress_v [5] = (H[1] + H[3]) * c3;// 2 FLOP
    //
    stress[0] = stress_v[0]; stress[1] = stress_v[5]; stress[2] = stress_v[4];
    stress[3] = stress_v[5]; stress[4] = stress_v[1]; stress[5] = stress_v[3];
    stress[6] = stress_v[4]; stress[7] = stress_v[3]; stress[8] = stress_v[2];
  }
  //TODO Need macros defined: FMR_HAS_AVX and FMR_HAS_AVX2
  //
  template <typename F> static inline//NOTE vH volatile for performance testing
  void linear_isotropic_3d_avx
    (F* fS, const F lambda, const F mu, volatile __m256d* vH) {
    //
    _mm256_storeu_pd(&fS[0],vH[0]);//TODO change storeu to store using VECALIGN from Femera v0.1.
    _mm256_storeu_pd(&fS[4],vH[1]);
    _mm256_storeu_pd(&fS[8],vH[2]);
    {
    const fmr::Phys_float tr = (fS[0]+fS[5]+fS[10]) * lambda;// 3 FLOP
    const __m256d mw= _mm256_set1_pd(mu);
    _mm256_storeu_pd( &fS[0], mw * vH[0] );// sxx sxy sxz | syx   // 4 FLOP
    _mm256_storeu_pd( &fS[4], mw * vH[1] );// syx syy syz | szx   // 4 FLOP
    _mm256_storeu_pd( &fS[8], mw * vH[2] );// szx szy szz | ---   // 4 FLOP
    //
    fS[0]=2.0*fS[0]+tr; fS[5]=2.0*fS[5]+tr; fS[10]=2.0*fS[10]+tr;// 6 FLOP
    }
    fS[1]+= fS[4];
    fS[2]+= fS[8];
    fS[6]+= fS[9];
    fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
  }
  //
  template <typename F> static inline//NOTE vA volatile for performance testing
  void linear_isotropic_3d_avx2
    (volatile __m256d* vA, const F lambda, const F mu) {
    //vA is strain coming in and stress going out.
    //
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
    Ssum+= vA[0]*ml;// 4 FLOP
    vA[1]=_mm256_permute4x64_pd( vA[1]*z0,_MM_SHUFFLE(1,3,2,0) );
    Ssum+= vA[1]*ml;// 4 FLOP
    vA[2]=_mm256_permute4x64_pd( vA[2]*z0,_MM_SHUFFLE(2,0,1,3) );
    Ssum+= vA[2]*ml;// 4 FLOP
    }
    //      3   2   1   0
    //     sxy 0.0 sxz sxx
    //     syx syz 0.0 syy
    //     0.0 szy szx szz
    //
    // mu*(sxy syz sxz)trace(H)*lambda : Ssum
    {
    const __m256d m2=_mm256_set_pd(2.0*mu,0.0,0.0,0.0);// 1 FLOP
    //      3   2   1   0
    // mu*(sxy 0.0 sxz)sxx*2*mu+lambda*trace(H)
    // mu*(sxy syz 0.0)syy*2*mu+lambda*trace(H)
    // mu*(0.0 syz sxz)szz*2*mu+lambda*trace(H)
#if 0
    printf("S step 2\n");
    print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
    vA[0]=_mm256_permute4x64_pd( Ssum + vA[0]*m2,_MM_SHUFFLE(3,2,0,3) );
    // 8 FLOP
    vA[1]=_mm256_permute4x64_pd( Ssum + vA[1]*m2,_MM_SHUFFLE(3,1,3,0) );
    // 8 FLOP
    vA[2]=_mm256_permute4x64_pd( Ssum + vA[2]*m2,_MM_SHUFFLE(3,3,1,2) );
    // 8 FLOP
    }
#if 0
    printf("S step 3\n");
    print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
  }



} } }//end namespace fmr::mtrl::elastic
