#include "../femera.hpp"
#include "FEms-elem.h"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

float tet_vol_ref = float (1.0 / 6.0);
fmr::Phys_float wsum1=0.0, wsum4=0.0, wsum5=0.0, wsum10=0.0, wsum11=0.0;

inline
std::string tri_norm_str
  (const fmr::Phys_float* p1, const fmr::Phys_float* p2, const fmr::Phys_float* p3) {
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
  return mini.exit ();
}

TEST( GridCellFEms, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
// Check stuff in FEms-elem.h.
TEST( GridCellFEmsElem, TetsVolOneSixth ){
  EXPECT_FLOAT_EQ( float(femera::grid::fems::tets_meas), tet_vol_ref );
}
TEST( GridCellFEmsElem, IntWgtsSumVol1 ){
  EXPECT_FLOAT_EQ( float(wsum1), tet_vol_ref );
}
TEST( GridCellFEmsElem, IntWgtsSumVol4 ){
  EXPECT_FLOAT_EQ( float(wsum4), tet_vol_ref );
}
TEST( GridCellFEmsElem, IntWgtsSumVol5 ){
  EXPECT_FLOAT_EQ( float(wsum5), tet_vol_ref );
}
TEST( GridCellFEmsElem, IntWgtsSumVol10 ){
  EXPECT_FLOAT_EQ( float(wsum10), tet_vol_ref );
}
TEST( GridCellFEmsElem, IntWgtsSumVol11 ){
  EXPECT_FLOAT_EQ( float(wsum11), tet_vol_ref );
}

TEST( GridCellFEmsTet, TetsNormalFaces ){
  EXPECT_EQ( tri_norm_str (//TODO get indices from tets_vert_face_tris
    &femera::grid::fems::tets_vert_coor [3* 0],
    &femera::grid::fems::tets_vert_coor [3* 1],
    &femera::grid::fems::tets_vert_coor [3* 2]), "+z" );
  EXPECT_EQ( tri_norm_str (
    &femera::grid::fems::tets_vert_coor [3* 0],
    &femera::grid::fems::tets_vert_coor [3* 3],
    &femera::grid::fems::tets_vert_coor [3* 1]), "+y" );
  EXPECT_EQ( tri_norm_str (
    &femera::grid::fems::tets_vert_coor [3* 0],
    &femera::grid::fems::tets_vert_coor [3* 2],
    &femera::grid::fems::tets_vert_coor [3* 3]), "+x" );
  EXPECT_EQ( tri_norm_str (
    &femera::grid::fems::tets_vert_coor [3* 1],
    &femera::grid::fems::tets_vert_coor [3* 3],
    &femera::grid::fems::tets_vert_coor [3* 2]),
    "[-0.577350, -0.577350, -0.577350]" );
}
