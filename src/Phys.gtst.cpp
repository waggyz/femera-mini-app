#include "gtest/gtest.h"
#include "base.h"

#include <stdio.h>
#if 0
//include "Phys.hpp"
int main(){
  //using namespace Femera;
  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      int v = fmr_phys_voigt_2d_index( i,j ); printf("%i ",v); }
  }
  printf("\n");
  for(int i=0; i<3; i++){
    for(int j=0; j<3; j++){
      int v = fmr_phys_voigt_3d_index( i,j ); printf("%i ",v); }
  }
  printf("\n");
  return 0;
}
#else
int main(int, char**){return 0; }
#endif
