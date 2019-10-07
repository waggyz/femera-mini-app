#include <ctype.h>
#include <stdio.h>
#include <omp.h>
#include "femera.h"
int Solv::Precond(Elem* E, Phys* Y){// Jacobi Preconditioner
  int bad_d=0;
  //this->udof_n = E->node_n * Y->node_d;
  //part_d.resize(udof_n,1.0);// Default Diagonal Preconditioner
  //if(this->solv_cond != Solv::COND_NONE){ this->part_d=0.0; };//FIXME
  switch(this->solv_cond){
  case(Solv::COND_NONE):{
    for(uint i=0; i<this->udof_n; i++){ this->part_d[i]=1.0; }; break;}
  case(Solv::COND_JACO):
  case(Solv::COND_TANG):{
    Y->ElemJacobi( E, this->part_d );
    Y->ElemJacobi( E, this->part_d, this->part_u );
    for(uint i=0; i<this->udof_n; i++){
      if(this->part_d[i]<0.0){ bad_d++;
        this->part_d[i]=std::abs(this->part_d[i]);
      } }
    #if VERB_MAX>3
    if(bad_d>0){//FIXME Put this in calling method.
      printf("WARNING: %i negative preconditioner values corrected.\n",
        bad_d); }
    #endif
    break;}
  case(Solv::COND_ROW1):{ Y->ElemRowSumAbs( E, this->part_d ); break;}
  case(Solv::COND_STRA):{ Y->ElemStrain( E, this->part_d ); break;}
  default:{ for(uint i=0; i<this->udof_n; i++){ this->part_d[i]=1.0; }; break;}
  }
  #if VERB_MAX>10
  printf("System Preconditioner [%i]",(int)this->udof_n);
  for(uint j=0;j<this->udof_n;j++){
    if(j%2 ==0){printf("\n");}
    printf("%+9.2e ",this->part_d[j]);
  } printf("\n");
  #endif
  //NOTE part_d contains the inverse of the preconditioner at this point,
  //     because it needs to be synced first.
  return( bad_d );
};
