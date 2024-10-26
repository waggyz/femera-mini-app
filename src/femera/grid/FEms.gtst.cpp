#include "../femera.hpp"
#include "FEms-elem.h"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

float tet_vol_ref = float (1.0 / 6.0);
fmr::Phys_float wsum1=0.0, wsum4=0.0, wsum5=0.0, wsum10=0.0, wsum11=0.0;
const fmr::Phys_float zero = 0.0;

fmr::Geom_float tet4_ntrl_jac3_trace  = -999.9;
//fmr::Geom_float tet4_ntrl_jac3_volume = -999.9;//TODO determinant for volume?

inline
std::string tri2_norm_str (const fmr::Phys_float* p1,
  const fmr::Phys_float* p2, const fmr::Phys_float* p3) {
  // Use this to check correct orientation of element surfaces.
  const auto Ax = p2[0] - p1[0], Ay =  p2[1] - p1[1];
  const auto Bx = p3[0] - p2[0], By =  p3[1] - p2[1];
  auto Nz = float(Ax * By - Ay * Bx);
  const auto len = std::sqrt (Nz*Nz);
  Nz /= len;
  if (std::abs (Nz - float(1.0)) < float(1.0e-6)) {return std::string ("+z");}
  if (std::abs (Nz + float(1.0)) < float(1.0e-6)) {return std::string ("-z");}
  return std::string ("["
    + std::to_string (0.0) + ", "
    + std::to_string (0.0) + ", "
    + std::to_string (Nz) + "]");
}
inline
std::string tri3_norm_str (const fmr::Phys_float* p1,
  const fmr::Phys_float* p2, const fmr::Phys_float* p3) {
  // Use this to check correct orientation of element surfaces.
  const auto Ax = p2[0] - p1[0], Ay =  p2[1] - p1[1], Az =  p2[2] - p1[2];
  const auto Bx = p3[0] - p2[0], By =  p3[1] - p2[1], Bz =  p3[2] - p2[2];
  auto Nx = float(Ay * Bz - Az * By);
  auto Ny = float(Az * Bx - Ax * Bz);
  auto Nz = float(Ax * By - Ay * Bx);
  const auto len = std::sqrt (Nx*Nx + Ny*Ny + Nz*Nz);
  Nx /= len; Ny /= len; Nz /= len;
  if (std::abs (Nx - float(1.0)) < float(1.0e-6)) {return std::string ("+x");}
  if (std::abs (Nx + float(1.0)) < float(1.0e-6)) {return std::string ("-x");}
  if (std::abs (Ny - float(1.0)) < float(1.0e-6)) {return std::string ("+y");}
  if (std::abs (Ny + float(1.0)) < float(1.0e-6)) {return std::string ("-y");}
  if (std::abs (Nz - float(1.0)) < float(1.0e-6)) {return std::string ("+z");}
  if (std::abs (Nz + float(1.0)) < float(1.0e-6)) {return std::string ("-z");}
  return std::string ("["
    + std::to_string (Nx) + ", "
    + std::to_string (Ny) + ", "
    + std::to_string (Nz) + "]");
}
inline
void tet4_jac3 ( fmr::Geom_float jac3[9],
  const fmr::Geom_float shpg[4 *3], const fmr::Geom_float coor[4 *3]) {
  //NOTE Zero shpg before calling this.
  //coor = femera::grid::fems::tets_vert_coor;// 4x3
  //coor = femera::grid::fems::tets_coor_vert;// 3x4 **Use this
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      for (int k=0; k<4; ++k) {
        jac3 [3*i +j]+= shpg [4*i +k] * coor [4*j +k];
  } } }
  return;
}
//============================================================================
fmr::Exit_int main (int argc, char** argv) {
  mini.init (& argc, argv);
  //
  for (int i=0; i< 1; ++i) {
    wsum1  += femera::grid::fems::tets_intg_1_ptwt  [4*i +3];}
  for (int i=0; i< 4; ++i) {
    wsum4  += femera::grid::fems::tets_intg_4_ptwt  [4*i +3];}
  for (int i=0; i< 5; ++i) {
    wsum5  += femera::grid::fems::tets_intg_5_ptwt  [4*i +3];}
  for (int i=0; i<10; ++i) {
    wsum10 += femera::grid::fems::tets_intg_10_ptwt [4*i +3];}
  for (int i=0; i<11; ++i) {
    wsum11 += femera::grid::fems::tets_intg_11_ptwt [4*i +3];}
  //
  // Calculate the Jacbian of a natural tet, which should be the 3x3 identity,
  // and is constant, independent of the integration point coordinates.
  // The one-point tet integration rule is sufficient.
  fmr::Geom_float intp [ 3];
  for (int i=0; i<3;++i){intp[i] = femera::grid::fems::tets_intg_1_ptwt[i];}
  //const auto intw = femera::grid::fems::tets_intg_1_ptwt [3];
  fmr::Geom_float shpg [12]
     = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};;
  femera::grid::fems::tets_shap_grad_4 (&shpg[0], &intp[0]);
  fmr::Geom_float coor [12];
  for (int i=0; i<12;++i){coor[i] = femera::grid::fems::tets_coor_vert[i];}
  fmr::Geom_float jac3 [ 9] = {0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0};
  //
  tet4_jac3 (&jac3[0], &shpg[0], &coor[0]);
  tet4_ntrl_jac3_trace = jac3[0] + jac3[4] + jac3[8];// 3 for 3x3 identity
  //
  return mini.exit ();
}
TEST( GridCellFEms, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
// Check stuff in FEms-elem.h.
TEST( GridCellFEmsElem, TetsVolOneSixth ){
  EXPECT_FLOAT_EQ( float(femera::grid::fems::tets_meas), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsIntWgtsSumVol1 ){
  EXPECT_FLOAT_EQ( float(wsum1), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsIntWgtsSumVol4 ){
  EXPECT_FLOAT_EQ( float(wsum4), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsIntWgtsSumVol5 ){
  EXPECT_FLOAT_EQ( float(wsum5), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsIntWgtsSumVol10 ){
  EXPECT_FLOAT_EQ( float(wsum10), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsIntWgtsSumVol11 ){
  EXPECT_FLOAT_EQ( float(wsum11), tet_vol_ref );
}
TEST( GridCellFEmsElem, TetsJacTraceThree ){
  EXPECT_FLOAT_EQ( float(tet4_ntrl_jac3_trace), float (3.0) );
}
#if 0
TEST( GridCellFEmsElem, TetsJacDetVolOneSixth ){
  EXPECT_FLOAT_EQ( float(tet4_jac3_volume), tet_vol_ref );
}
#endif
namespace femera { namespace grid { namespace fems {

TEST( GridCellFEmsTet, FaceNormal1 ){
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* 0],
    &tets_vert_coor [3* 1],
    &tets_vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* 0],
    &tets_vert_coor [3* 3],
    &tets_vert_coor [3* 1]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* 0],
    &tets_vert_coor [3* 2],
    &tets_vert_coor [3* 3]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* 1],
    &tets_vert_coor [3* 3],
    &tets_vert_coor [3* 2]),
    "[-0.577350, -0.577350, -0.577350]" );// -sqrt(1/3)
}
TEST( GridCellFEmsTet, FaceNormal2 ){
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* tets_vert_face_tris [0]],
    &tets_vert_coor [3* tets_vert_face_tris [1]],
    &tets_vert_coor [3* tets_vert_face_tris [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* tets_vert_face_tris [3]],
    &tets_vert_coor [3* tets_vert_face_tris [4]],
    &tets_vert_coor [3* tets_vert_face_tris [5]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* tets_vert_face_tris [6]],
    &tets_vert_coor [3* tets_vert_face_tris [7]],
    &tets_vert_coor [3* tets_vert_face_tris [8]]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    &tets_vert_coor [3* tets_vert_face_tris [ 9]],
    &tets_vert_coor [3* tets_vert_face_tris [10]],
    &tets_vert_coor [3* tets_vert_face_tris [11]]),
    "[-0.577350, -0.577350, -0.577350]" );
}
// Check Cube element surfaces as triangles
TEST( GridCellFEmsCube, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [0]],
    & cube_vert_coor [3*  cube_vert_face_quad [1]],
    & cube_vert_coor [3*  cube_vert_face_quad [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [2]],
    & cube_vert_coor [3*  cube_vert_face_quad [3]],
    & cube_vert_coor [3*  cube_vert_face_quad [0]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [4]],
    & cube_vert_coor [3*  cube_vert_face_quad [5]],
    & cube_vert_coor [3*  cube_vert_face_quad [6]]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [6]],
    & cube_vert_coor [3*  cube_vert_face_quad [7]],
    & cube_vert_coor [3*  cube_vert_face_quad [4]]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [ 8]],
    & cube_vert_coor [3*  cube_vert_face_quad [ 9]],
    & cube_vert_coor [3*  cube_vert_face_quad [10]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [10]],
    & cube_vert_coor [3*  cube_vert_face_quad [11]],
    & cube_vert_coor [3*  cube_vert_face_quad [ 8]]), "+y" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [12]],
    & cube_vert_coor [3*  cube_vert_face_quad [13]],
    & cube_vert_coor [3*  cube_vert_face_quad [14]]), "-x" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [14]],
    & cube_vert_coor [3*  cube_vert_face_quad [15]],
    & cube_vert_coor [3*  cube_vert_face_quad [12]]), "-x" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [16]],
    & cube_vert_coor [3*  cube_vert_face_quad [17]],
    & cube_vert_coor [3*  cube_vert_face_quad [18]]), "-y" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [18]],
    & cube_vert_coor [3*  cube_vert_face_quad [19]],
    & cube_vert_coor [3*  cube_vert_face_quad [16]]), "-y" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [20]],
    & cube_vert_coor [3*  cube_vert_face_quad [21]],
    & cube_vert_coor [3*  cube_vert_face_quad [22]]), "+x" );
  EXPECT_EQ( tri3_norm_str (
    & cube_vert_coor [3*  cube_vert_face_quad [22]],
    & cube_vert_coor [3*  cube_vert_face_quad [23]],
    & cube_vert_coor [3*  cube_vert_face_quad [20]]), "+x" );
}
TEST( GridCellFEmsFac3, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &fac3_vert_coor [3* fac3_vert_face_tris [0]],
    &fac3_vert_coor [3* fac3_vert_face_tris [1]],
    &fac3_vert_coor [3* fac3_vert_face_tris [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &fac3_vert_coor [3* fac3_vert_face_tris [3]],
    &fac3_vert_coor [3* fac3_vert_face_tris [4]],
    &fac3_vert_coor [3* fac3_vert_face_tris [5]]), "-z" );
}
TEST( GridCellFEmsFac4, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &fac4_vert_coor [3* fac4_vert_face_quad [0]],
    &fac4_vert_coor [3* fac4_vert_face_quad [1]],
    &fac4_vert_coor [3* fac4_vert_face_quad [2]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &fac4_vert_coor [3* fac4_vert_face_quad [2]],
    &fac4_vert_coor [3* fac4_vert_face_quad [3]],
    &fac4_vert_coor [3* fac4_vert_face_quad [0]]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &fac4_vert_coor [3* fac4_vert_face_quad [4]],
    &fac4_vert_coor [3* fac4_vert_face_quad [5]],
    &fac4_vert_coor [3* fac4_vert_face_quad [6]]), "-z" );
  EXPECT_EQ( tri3_norm_str (
    &fac4_vert_coor [3* fac4_vert_face_quad [6]],
    &fac4_vert_coor [3* fac4_vert_face_quad [7]],
    &fac4_vert_coor [3* fac4_vert_face_quad [4]]), "-z" );
}
TEST( GridCellFEmsTris, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &tris_vert_coor [3* 0],
    &tris_vert_coor [3* 1],
    &tris_vert_coor [3* 2]), "+z" );
}
TEST( GridCellFEmsQuad, FaceNormal ){
  EXPECT_EQ( tri3_norm_str (
    &quad_vert_coor [3* 0],
    &quad_vert_coor [3* 1],
    &quad_vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri3_norm_str (
    &quad_vert_coor [3* 2],
    &quad_vert_coor [3* 3],
    &quad_vert_coor [3* 0]), "+z" );
}

} } }//end femera::grid::fems:: namespace
