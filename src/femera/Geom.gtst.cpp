#include "femera.hpp"
#include "Geom.hpp"
#include <cstring>      // std::memcpy

#include <gtest/gtest.h>

auto mini = fmr::new_jobs ();
#if 0
auto jac_det_float  = femera::geom::Jac_det_pad<float> ();
auto jac_det_double = femera::geom::Jac_det_pad<double> ();
const auto jdf = std::valarray<float>
  ({2.0,0.0,0.0,0.0,
    0.0,2.0,0.0,0.0,
    0.0,0.0,2.0,2.0*2.0*2.0});// det is 8.0, inv_det ==1.0/8.0
const auto jdd = std::valarray<double>
  ({2.0,0.0,0.0,0.0,
    0.0,2.0,0.0,0.0,
    0.0,0.0,2.0,2.0*2.0*2.0});// det is 8.0, inv_det ==1.0/8.0
inline
int fill_jac_dets () {
  std::memcpy (&jac_det_float.buf[0],  &jdf[0], jdf.size()*sizeof(jdf[0]));
  std::memcpy (&jac_det_double.buf[0], &jdd[0], jdd.size()*sizeof(jdd[0]));
  double det_inv = 1.0 / 8.0;
  char buf_inv [sizeof(double)];
  std::memcpy (&buf_inv[0], &det_inv, sizeof(double));
  std::memcpy
    (&jac_det_double.buf[3*sizeof(double)], &buf_inv[0], sizeof(double));
  std::memcpy
    (&jac_det_float.buf[3*sizeof(float)],&buf_inv[0], sizeof(float));
  std::memcpy
    (&jac_det_float.buf[5*sizeof(float)],&buf_inv[sizeof(float)], sizeof(float));
  return 0;
}
#endif
TEST( Sims, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}
#if 0
TEST( Sims, JacDet ){
  EXPECT_EQ( 0, fill_jac_dets () );
  EXPECT_EQ( 2.0f, jac_det_float.get_jac  ()[ 0] );
  EXPECT_EQ( 2.0 , jac_det_double.get_jac ()[ 0] );
  EXPECT_EQ( 8.0f, jac_det_float.get_jac  ()[11] );
  EXPECT_EQ( 8.0 , jac_det_double.get_jac ()[11] );
  EXPECT_EQ( 8.0f , jac_det_float.get_det ()[ 0] );
  EXPECT_EQ( 8.0 , jac_det_double.get_det ()[ 0] );
  EXPECT_EQ( 1.0/8.0 , jac_det_double.get_jac ()[3] );
  EXPECT_EQ( 1.0/8.0 , jac_det_double.get_inv ()[0] );
}
#endif
fmr::Exit_int main (int argc, char** argv) {
#pragma GCC diagnostic ignored "-Winline"
  return mini->exit (mini->init (&argc,argv));
#pragma GCC diagnostic warning "-Winline"
}
