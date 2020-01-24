#include <ctype.h>
#include <stdio.h>
#include <omp.h>
#include "femera.h"
int Solv::Precond(Elem* E, Phys* Y){// Jacobi Preconditioner
  int bad_d=0;
  switch(this->solv_cond){
  case(Solv::COND_NONE):{
    part_d=align_resize( data_d, udof_n+1, valign_byte );
    for(uint i=0; i<this->udof_n; i++){ this->part_d[i]=1.0; };
    break; }
  case(Solv::COND_JACO):
  case(Solv::COND_TANG):{
    part_d=align_resize( data_d, udof_n+1, valign_byte );
    Y->ElemJacobi( E, this->part_d );
    Y->ElemJacobi( E, this->part_d, this->part_u );
    for(uint i=0; i<this->udof_n; i++){
      if(this->part_d[i]<0.0){ bad_d++;
        this->part_d[i]=std::abs(this->part_d[i]);
      } }
#if VERB_MAX>3
    if( bad_d > 0 ){//FIXME Put this in calling method.
      printf("WARNING: %i negative preconditioner values corrected.\n",
        bad_d); }
#endif
    break; }
  case(Solv::COND_JAC3):{
    this->cond_bloc_n = 3;
    part_d = align_resize( data_d, 3*udof_n+1, valign_byte );
    //FIXME Resized after init
    for(INT_MESH i=0; i<data_d.size(); i++){ data_d[i]=0.0; }
    Y->ElemJacNode( E, this->part_d );
#if 0
    Y->ElemJacNode( E, this->part_d, this->part_u );
#endif
    //FIXME Handle negative diagonals.
    break; }
  case(Solv::COND_ROW1):{
    part_d=align_resize( data_d, udof_n+1, valign_byte );
    Y->ElemRowSumAbs( E, this->part_d ); break;}
  case(Solv::COND_STRA):{ 
    part_d=align_resize( data_d, udof_n+1, valign_byte );
    Y->ElemStrain( E, this->part_d ); break;}
  default:{ 
    part_d=align_resize( data_d, udof_n+1, valign_byte );
    for(uint i=0; i<this->udof_n; i++){ this->part_d[i]=1.0; }; break;}
  }
#if VERB_MAX>10
  printf("System Preconditioner [%i]",(int)this->udof_n);
  for(uint j=0;j<this->udof_n;j++){
    if(j%2 ==0){printf("\n");}
    printf("%+9.2e ",this->part_d[j]);
  } printf("\n");
#endif
  //NOTE part_d contains the inverse of the preconditioner at this point,
  //     because it needs to be synced before inverting.
  return( bad_d );
};
