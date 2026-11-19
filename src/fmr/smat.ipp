#ifndef FMR_HAS_SMAT_IPP
#define FMR_HAS_SMAT_IPP

//#define FMR_DEBUG

// Generalized functions call the most performant implementation. =============
template <fmr::Dim_int D, typename F> static inline
F fmr::math::smat_det (const F* mat){
  static_assert(D >= 1 && D <= 4,
                "smat_det supports only dimensions 1, 2, 3, and 4");
  if (D == 1) return mat[0];// too trivial to test
  if (D == 2) return mat[0]*mat[3] - mat[1]*mat[2];// too trivial to test
  if (D == 3) return fmr::math::m3x3_det_ref(mat);//TODO Test variants?
  return fmr::math::m4x4_det_ref(mat);//TODO Test variants?
}
template <fmr::Dim_int D, typename F> static inline
int fmr::math::smat_invt_det (F* invt_det, const F* mat){
  static_assert(D >= 1 && D <= 4,
                "smat_det supports only dimensions 1, 2, 3, and 4");
  if (D == 1) return fmr::math::m1x1_invt_det_ref(invt_det, mat);
  if (D == 2) return fmr::math::m2x2_invt_det_ref(invt_det, mat);
FMR_WARN_INLINE_OFF
  if (D == 3) return fmr::math::m3x3_invt_det_ref(invt_det, mat);
FMR_WARN_INLINE_ON
  // So far, tested variants for D = 1, 2, 3 all seem like they will
  // perform about the same. No m4x4 variants tested yet.
  return fmr::math::m4x4_invt_det_ref(invt_det, mat);//TODO test variants.
}
template <fmr::Dim_int D, typename F> static inline
int fmr::math::smat_pinv_col_ref (F* pinv, const F* col){
  static_assert(D >= 1 && D <= 4,
                "smat_pinv_col_ref supports dimensions 1 through 4");
  F norm_sq = F(0);
  for (fmr::Dim_int i = 0; i < D; ++i) norm_sq += col[i]*col[i];
  if (::fmr::math::are_equal(norm_sq, F(0))) return 1;
  const F inv_norm_sq = F(1)/norm_sq;
  for (fmr::Dim_int i = 0; i < D; ++i) pinv[i] = col[i]*inv_norm_sq;
  return 0;
}
#if 0
// Default variants call the most performant implementation. ==================
template <typename F> static inline
F fmr::math::m3x3_det (const F* mat){// 12 FLOP
	return fmr::math::m3x3_det_ref (mat);
}
template <typename F> static inline
int fmr::math::m3x3_invt_det (F* invt_det, const F* mat){
  return fmr::math::m3x3_invt_det_ref(invt_det, mat);
}
#endif
// Helpers ====================================================================

template <typename F> static inline
void smat_cofactors3_ref (F* cofactors, const F* mat)// 27 FLOP
{
	cofactors[0] = mat[4]*mat[8] - mat[5]*mat[7];
	cofactors[1] = mat[5]*mat[6] - mat[3]*mat[8];
	cofactors[2] = mat[3]*mat[7] - mat[4]*mat[6];
	cofactors[3] = mat[2]*mat[7] - mat[1]*mat[8];
	cofactors[4] = mat[0]*mat[8] - mat[2]*mat[6];
	cofactors[5] = mat[1]*mat[6] - mat[0]*mat[7];
	cofactors[6] = mat[1]*mat[5] - mat[2]*mat[4];
	cofactors[7] = mat[2]*mat[3] - mat[0]*mat[5];
	cofactors[8] = mat[0]*mat[4] - mat[1]*mat[3];
}

template <typename F> static inline
void smat_cofactors4_ref (F* cofactors, const F* mat)
{
	F minor[9];
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			int minor_ix = 0;
			for (int minor_row = 0; minor_row < 4; ++minor_row) {
				if (minor_row == row) continue;
				for (int minor_col = 0; minor_col < 4; ++minor_col) {
					if (minor_col != col)
						minor[minor_ix++] = mat[minor_row*4 + minor_col];
				}
			}
			const F sign = (row + col) % 2 == 0 ? F(1) : F(-1);
			cofactors[row*4 + col] = sign*fmr::math::smat_det<3>(minor);
		}
	}
}
// Test variants ==============================================================
template <typename F> static inline
F fmr::math::m3x3_det_ref (const F* mat){// 12 FLOP
	return mat[0]*(mat[4]*mat[8] - mat[5]*mat[7])
	     - mat[1]*(mat[3]*mat[8] - mat[5]*mat[6])
	     + mat[2]*(mat[3]*mat[7] - mat[4]*mat[6]);
}
template <typename F> static inline
F fmr::math::m4x4_det_ref (const F* mat){// 64 FLOP
	F cof[16];
	smat_cofactors4_ref(cof, mat);
	return mat[0]*cof[0] + mat[1]*cof[1] + mat[2]*cof[2] + mat[3]*cof[3];
}

template <typename F> static inline
int ::fmr::math::m1x1_inv_ref (F* inv, const F* mat)
{
	if (::fmr::math::are_equal(mat[0], F(0))) return 1;// FLOP not counted
	inv[0] = F(1)/mat[0];
	return mat[0] < F(0) ? -1 : 0;
}
template <typename F> static inline
int ::fmr::math::m1x1_inv_det_ref (F* inv_det, const F* mat)
{
	inv_det[1] = mat[0];
	if (::fmr::math::are_equal(inv_det[1], F(0))) return 1;// FLOP not counted
	inv_det[0] = F(1)/inv_det[1];
	return mat[0] < F(0) ? -1 : 0;
}
template <typename F> static inline
int ::fmr::math::m1x1_invt_det_ref (F* invt_det, const F* mat)
{
	invt_det[1] = mat[0];
	if (::fmr::math::are_equal(invt_det[1], F(0))) return 1;// FLOP not counted
	invt_det[0] = F(1)/invt_det[1];
	return mat[0] < F(0) ? -1 : 0;
}
template <typename F> static inline
int ::fmr::math::m2x2_inv_ref (F* inv, const F* mat)
{
	const F det = mat[0]*mat[3] - mat[1]*mat[2];
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det = F(1)/det;
	inv[0] =  mat[3]*inv_det;
	inv[1] = -mat[1]*inv_det;
	inv[2] = -mat[2]*inv_det;
	inv[3] =  mat[0]*inv_det;
	return det < F(0) ? -1 : 0;
}
template <typename F> static inline
int ::fmr::math::m2x2_inv_a (F* inv, const F* mat)
{
	const F det = mat[0]*mat[3] - mat[1]*mat[2];
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det = F(1)/det;
  for (int i=0; i<4; ++i) { inv[i] = inv_det; }// broadcast det inverse
	inv[0] *=  mat[3];
	inv[1] *= -mat[1];
	inv[2] *= -mat[2];
	inv[3] *=  mat[0];
	return det < F(0) ? -1 : 0;
}
template <typename F> static inline
int ::fmr::math::m2x2_inv_b (F* inv, const F* mat)
{
	inv[0] =  mat[3];
	inv[1] = -mat[1];
	inv[2] = -mat[2];
	inv[3] =  mat[0];
	const F det = mat[0]*inv[0] + mat[1]*inv[2];
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det = F(1)/det;
  for (int i=0; i<4; ++i) { inv[i] *= inv_det; }
	return det < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse & determinant
int ::fmr::math::m2x2_inv_det_ref (F* inv_det, const F* mat)
// returns false if not invertible (determinant is zero)
{
	inv_det[4] = mat[0]*mat[3] - mat[1]*mat[2];// det
	if (::fmr::math::are_equal(inv_det[4], F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/inv_det[4];
	inv_det[0] =  mat[3]*inv_det_val;
	inv_det[1] = -mat[1]*inv_det_val;
	inv_det[2] = -mat[2]*inv_det_val;
	inv_det[3] =  mat[0]*inv_det_val;
	return inv_det[4] < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse transposed & determinant
int ::fmr::math::m2x2_invt_det_ref (F* invt_det, const F* mat)
// returns false if not invertible (determinant is zero)
{
	const F det = mat[0]*mat[3] - mat[1]*mat[2];
	invt_det[4] = det;
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/det;
	invt_det[0] =  mat[3]*inv_det_val;
	invt_det[1] = -mat[2]*inv_det_val;
	invt_det[2] = -mat[1]*inv_det_val;
	invt_det[3] =  mat[0]*inv_det_val;
	return det < F(0) ? -1 : 0;
}

template <typename F> static inline
int ::fmr::math::m3x3_inv_ref (F* inv, const F* mat)// 41 FLOP
{
	F cofactors[9];
	smat_cofactors3_ref(cofactors, mat);// 27 FLOP
	const F det = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	            + mat[2]*cofactors[2];// 4 FLOP
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det = F(1)/det;// 1 FLOP
	for (int row = 0; row < 3; ++row)
		for (int col = 0; col < 3; ++col)
			inv[row*3 + col] = cofactors[col*3 + row]*inv_det;// 9 * 1 FLOP
	return det < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse & determinant
int ::fmr::math::m3x3_inv_det_ref (F* inv_det, const F* mat)
{
	F cofactors[9];
	smat_cofactors3_ref(cofactors, mat);
	inv_det[9] = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	           + mat[2]*cofactors[2];
	if (::fmr::math::are_equal(inv_det[9], F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/inv_det[9];
	for (int row = 0; row < 3; ++row)
		for (int col = 0; col < 3; ++col)
			inv_det[row*3 + col] = cofactors[col*3 + row]*inv_det_val;
	return inv_det[9] < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse transposed & determinant
int ::fmr::math::m3x3_invt_det_ref (F* invt_det, const F* mat)
{
	F cofactors[9];
	smat_cofactors3_ref(cofactors, mat);
	invt_det[9] = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	            + mat[2]*cofactors[2];
	if (::fmr::math::are_equal(invt_det[9], F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/invt_det[9];
	for (int i = 0; i < 9; ++i)
		invt_det[i] = cofactors[i]*inv_det_val;
	return invt_det[9] < F(0) ? -1 : 0;
}

template <typename F> static inline
int ::fmr::math::m3x3_inv_a (F* inv, const F* mat)// 36 + 12 +1 = 49 FLOP
{
  const F det = fmr::math::smat_det<3>(mat);// 12 FLOP
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
  const F inv_det = F(1)/det;// 1 FLOP
  for (int i = 0; i < 9; ++i) {inv[i] = inv_det;}// 36 FLOP
  //
  inv[3*0+ 0]*= (mat[3*1+ 1] * mat[3*2+ 2] - mat[3*2+ 1] * mat[3*1+ 2]) ;
  inv[3*0+ 1]*= (mat[3*0+ 2] * mat[3*2+ 1] - mat[3*0+ 1] * mat[3*2+ 2]) ;
  inv[3*0+ 2]*= (mat[3*0+ 1] * mat[3*1+ 2] - mat[3*0+ 2] * mat[3*1+ 1]) ;
  //
  inv[3*1+ 0]*= (mat[3*1+ 2] * mat[3*2+ 0] - mat[3*1+ 0] * mat[3*2+ 2]) ;
  inv[3*1+ 1]*= (mat[3*0+ 0] * mat[3*2+ 2] - mat[3*0+ 2] * mat[3*2+ 0]) ;
  inv[3*1+ 2]*= (mat[3*1+ 0] * mat[3*0+ 2] - mat[3*0+ 0] * mat[3*1+ 2]) ;
  //
  inv[3*2+ 0]*= (mat[3*1+ 0] * mat[3*2+ 1] - mat[3*2+ 0] * mat[3*1+ 1]) ;
  inv[3*2+ 1]*= (mat[3*2+ 0] * mat[3*0+ 1] - mat[3*0+ 0] * mat[3*2+ 1]) ;
  inv[3*2+ 2]*= (mat[3*0+ 0] * mat[3*1+ 1] - mat[3*1+ 0] * mat[3*0+ 1]) ;
  //
	return inv_det < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse transposed & determinant
int ::fmr::math::m3x3_invt_det_a (F* invt_det, const F* mat)
{
  invt_det[9] = fmr::math::smat_det<3>(mat);// 12 FLOP
	if (::fmr::math::are_equal(invt_det[9], F(0))) return 1;// FLOP not counted
  const F inv_det = F(1)/invt_det[9];// 1 FLOP
  for (int i = 0; i < 9; ++i) {invt_det[i] = inv_det;}// 36 FLOP
  //
  invt_det[3*0+ 0]*= (mat[3*1+ 1] * mat[3*2+ 2] - mat[3*2+ 1] * mat[3*1+ 2]) ;
  invt_det[3*0+ 1]*= (mat[3*1+ 2] * mat[3*2+ 0] - mat[3*1+ 0] * mat[3*2+ 2]) ;
  invt_det[3*0+ 2]*= (mat[3*1+ 0] * mat[3*2+ 1] - mat[3*2+ 0] * mat[3*1+ 1]) ;
  //
  invt_det[3*1+ 0]*= (mat[3*0+ 2] * mat[3*2+ 1] - mat[3*0+ 1] * mat[3*2+ 2]) ;
  invt_det[3*1+ 1]*= (mat[3*0+ 0] * mat[3*2+ 2] - mat[3*0+ 2] * mat[3*2+ 0]) ;
  invt_det[3*1+ 2]*= (mat[3*2+ 0] * mat[3*0+ 1] - mat[3*0+ 0] * mat[3*2+ 1]) ;
  //
  invt_det[3*2+ 0]*= (mat[3*0+ 1] * mat[3*1+ 2] - mat[3*0+ 2] * mat[3*1+ 1]) ;
  invt_det[3*2+ 1]*= (mat[3*1+ 0] * mat[3*0+ 2] - mat[3*0+ 0] * mat[3*1+ 2]) ;
  invt_det[3*2+ 2]*= (mat[3*0+ 0] * mat[3*1+ 1] - mat[3*1+ 0] * mat[3*0+ 1]) ;
  //
	return invt_det[9] < F(0) ? -1 : 0;
}


template <typename F> static inline
int ::fmr::math::m4x4_inv_ref (F* inv, const F* mat)
{
	F cofactors[16];
	smat_cofactors4_ref(cofactors, mat);// 6 FLOP
	const F det = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	            + mat[2]*cofactors[2] + mat[3]*cofactors[3];
	if (::fmr::math::are_equal(det, F(0))) return 1;// FLOP not counted
	const F inv_det = F(1)/det;
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			inv[row*4 + col] = cofactors[col*4 + row]*inv_det;
	return det < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse & determinant
int ::fmr::math::m4x4_inv_det_ref (F* inv_det, const F* mat)
{
	F cofactors[16];
	smat_cofactors4_ref(cofactors, mat);
	inv_det[16] = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	            + mat[2]*cofactors[2] + mat[3]*cofactors[3];
	if (::fmr::math::are_equal(inv_det[16], F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/inv_det[16];
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			inv_det[row*4 + col] = cofactors[col*4 + row]*inv_det_val;
	return inv_det[16] < F(0) ? -1 : 0;
}
template <typename F> static inline// matrix inverse transposed & determinant
int ::fmr::math::m4x4_invt_det_ref (F* invt_det, const F* mat)
{
	F cofactors[16];
	smat_cofactors4_ref(cofactors, mat);
	invt_det[16] = mat[0]*cofactors[0] + mat[1]*cofactors[1]
	             + mat[2]*cofactors[2] + mat[3]*cofactors[3];
	if (::fmr::math::are_equal(invt_det[16], F(0))) return 1;// FLOP not counted
	const F inv_det_val = F(1)/invt_det[16];
	for (int i = 0; i < 16; ++i)
		invt_det[i] = cofactors[i]*inv_det_val;
	return invt_det[16] < F(0) ? -1 : 0;
}

#undef FMR_DEBUG
//end FMR_HAS_SMAT_IPP
#endif
