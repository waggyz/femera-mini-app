#include "../femera.hpp"
#include "FEms-elem.h"

#include <gtest/gtest.h>

auto  mini_ptr = fmr::new_jobs ();
auto& mini = *mini_ptr;

fmr::Phys_float wsum1=0, wsum4=0, wsum5=0, wsum10=0, wsum11=0;

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

TEST( GridCellFEmsElem, TetsVolOneSixth ){
  EXPECT_FLOAT_EQ( float(1.0/6.0), float(femera::grid::fems::tets_meas) );
}
TEST( GridCellFEmsElem, IntWgtsSumVol1 ){
  EXPECT_FLOAT_EQ( float(wsum1), float(femera::grid::fems::tets_meas) );
}
TEST( GridCellFEmsElem, IntWgtsSumVol4 ){
  EXPECT_FLOAT_EQ( float(wsum4), float(femera::grid::fems::tets_meas) );
}
TEST( GridCellFEmsElem, IntWgtsSumVol5 ){
  EXPECT_FLOAT_EQ( float(wsum5), float(femera::grid::fems::tets_meas) );
}
TEST( GridCellFEmsElem, IntWgtsSumVol10 ){
  EXPECT_FLOAT_EQ( float(wsum10), float(femera::grid::fems::tets_meas) );
}
TEST( GridCellFEmsElem, IntWgtsSumVol11 ){
  EXPECT_FLOAT_EQ( float(wsum11), float(femera::grid::fems::tets_meas) );
}

