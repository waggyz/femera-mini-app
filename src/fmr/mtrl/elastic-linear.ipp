namespace fmr { namespace mtrl { namespace elastic {
//
  template <typename F,// 75 FLOP
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void elastic::linear_3d_dmat_base
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
#if 0
//TODO general AVX/AVX2 6x6 matrix-vector multiply
  template <typename F,// XXX FLOP
  typename std::enable_if<std::is_same<F, double>::value>::type* = nullptr>
  // for F==__m256d
  void linear_3d_isotropic_dmat_avx
    (F* S, const F* D, volatile F* H) {
    // Structure of S and H
    //  3   2   1  0       3   2   1  0
    // sxx sxy sxz 0 -or- sxx sxy sxz 0
    // sxy syy syz 0 -or- syx syy syz 0
    // sxz syz szz 0 -or- szx szy szz 0
    //
    // Structure of vS and vH
    //  3   2   1   0 |   3    2     1    0
    // sxx syy szz  0 | 2*sxz 2*syz 2*sxy 0
    //               -or-
    //  3   2   1   0 |    3       2       1    0
    // sxx syy szz  0 | sxz+szx syz+sxy sxy+syx 0
    // = sum:
    // sxx syy szz  0 |   sxz     syz     sxy   0
    //                |   szx     szy     syx   0
    //
    // Structure of D
    //  3   2   1   0 | 3   2   1   0
    // d11 d12 d13  0 |d14 d15 d16  0
    // d12 d22 d23  0 |d24 d25 d26  0
    // d13 d23 d33  0 |d34 d35 d36  0
    // d14 d24 d34  0 |d44 d45 d46  0
    // d15 d25 d35  0 |d45 d55 d56  0
    // d16 d26 d36  0 |d46 d56 d66  0
  }
//
// by NASA LunAI, Gemini 1.5 Pro, 2024-10-19
// [This should work for both AVX and AVX2]
//
// Define a structure for a 6x6 matrix aligned to 32 bytes
typedef struct {
    __m256d data[6][2];
} Matrix6x6;

// Define a structure for a conformal vector aligned to 32 bytes
typedef struct {
    double x;
    double y;
    double z;
    double w;
    double p;
    double d;
} ConformalVector;

// Function to multiply a 6x6 matrix by a conformal vector using AVX2 intrinsics
ConformalVector multiplyMatrixVector(const Matrix6x6 *matrix, const ConformalVector *vector) {
    ConformalVector result;

    // Load vector elements into AVX registers
    __m256d vec0 = _mm256_set_pd(vector->x, vector->y, vector->z, vector->w);
    __m256d vec1 = _mm256_set_pd(vector->p, vector->d, 0.0, 0.0);

    // Perform matrix multiplication using AVX2 intrinsics
    for (int i = 0; i < 6; ++i) {
        __m256d sum = _mm256_mul_pd(matrix->data[i][0], vec0);
        sum = _mm256_fmadd_pd(matrix->data[i][1], vec1, sum);
        
        // Extract and accumulate results
        if (i < 4) {
            (&result.x)[i] = sum[0] + sum[2]; 
        } else {
            (&result.p)[i - 4] = sum[0] + sum[2];
        }
    }

    return result;
}
Exp
#endif
#ifdef FMR_HAS_MKL
  template <typename F,// assume 75 FLOP
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void elastic::linear_3d_dmat_symm
    (F* stress, const F* D, volatile F* H, F* strain_voigt, F* stress_v) {
    //
    strain_voigt [0] = H[0];
    strain_voigt [1] = H[4];
    strain_voigt [2] = H[8];
    strain_voigt [3] = H[5] + H[7];// 1 FLOP
    strain_voigt [4] = H[2] + H[6];// 1 FLOP
    strain_voigt [5] = H[1] + H[3];// 1 FLOP
    //
    //TODO placeholder for Intel MKL symmetric function call
    //
    stress[0] = stress_v[0]; stress[1] = stress_v[5]; stress[2] = stress_v[4];
    stress[3] = stress_v[5]; stress[4] = stress_v[1]; stress[5] = stress_v[3];
    stress[6] = stress_v[4]; stress[7] = stress_v[3]; stress[8] = stress_v[2];
  }
#endif
  template <typename F,// 24 FLOP
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr >
  static inline
  void linear_3d_isotropic_lame
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
  template <typename F,// 21 FLOP
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_cubic_scalar_a
    (F* stress, const F c1,const F c2,const F c3, volatile F* H) {
    //
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
  template <typename F,// 21 FLOP
  typename std::enable_if<std::is_floating_point<F>::value>::type* = nullptr>
  static inline
  void linear_3d_cubic_scalar_b
    (F* stress, const F c1,const F c2,const F c3, volatile F* H, F* stress_v) {
    //
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
#ifdef FMR_HAS_AVX
  template <typename F,
  typename std::enable_if<std::is_same<F, double>::value>::type* = nullptr>
  static inline
  void linear_3d_isotropic_avx
    (F* fS, const F lambda, const F mu, volatile __m256d* vH) {
    //
    _mm256_store_pd (&fS[0], vH[0]);
    _mm256_store_pd (&fS[4], vH[1]);
    _mm256_store_pd (&fS[8], vH[2]);
    {// Scope variables tr and mw.
      const fmr::Phys_float tr = (fS[0] + fS[5] + fS[10]) * lambda;// 3 FLOP
      const __m256d         mw =_mm256_set1_pd (mu);
      _mm256_store_pd (&fS[0], mw * vH[0]);// sxx sxy sxz | syx    // 4 FLOP
      _mm256_store_pd (&fS[4], mw * vH[1]);// syx syy syz | szx    // 4 FLOP
      _mm256_store_pd (&fS[8], mw * vH[2]);// szx szy szz | ---    // 4 FLOP
      //
      fS[ 0] = 2.0*fS[ 0] + tr;// 2 FLOP
      fS[ 5] = 2.0*fS[ 5] + tr;// 2 FLOP
      fS[10] = 2.0*fS[10] + tr;// 2 FLOP
    }
    fS[1]+=fS[4];
    fS[2]+=fS[8];
    fS[6]+=fS[9];
    fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
  }
#endif
#ifdef FMR_HAS_AVX2
  template <typename F,
  typename std::enable_if<std::is_same<F, double>::value>::type* = nullptr>
  static inline//NOTE vA volatile for performance testing
  void linear_3d_isotropic_avx2 //FIXME only for F=double
    (volatile __m256d* vA, const F lambda, const F mu) {
    //
    //vA is strain coming in and stress going out.
    //
    // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
    // Structure of S and H
    //  3   2   1     0
    // sxx sxy sxz | sxy
    // sxy syy syz | sxz
    // sxz syz szz | ---
    __m256d Ssum=_mm256_setzero_pd();
    {// Scope variables z0 and ml.
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
#endif

} } }//end namespace fmr::mtrl::elastic
