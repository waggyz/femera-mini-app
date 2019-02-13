#include <ctype.h>
#include <stdio.h>
#include <omp.h>
#include "femera.h"
int Solv::Precond(Elem* E, Phys* Y){// Jacobi Preconditioner
  //this->udof_n = E->node_n * Y->ndof_n;
  //sys_d.resize(udof_n,1.0);// Default Diagonal Preconditioner
  this->sys_d=0.0;//FIXME
  /*
  //FIXME This isn't part of the preconditioner setup...
  uint d=uint(Y->ndof_n);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    //printf("FIX ID %i, DOF %i, val %+9.2e\n",i,E->bcs_vals[i].first,E->bcs_vals[i].second);
    this->sys_u[d* n+uint(f)] = v;
  };
  Y->ElemLinear( E );
  */
  //Y->ElemJacobi( E, this->sys_d );//FIXED Replace above with this
  Y->ElemRowSumAbs( E, this->sys_d );
  //for(uint i=1; i<this->sys_d.size(); i++){
  //    this->sys_d[i]=std::sqrt(this->sys_d[i]); };
  //
  #if VERB_MAX>10
  printf("System Preconditioner [%i]",(int)this->sys_d.size());
  for(uint j=0;j<this->sys_d.size();j++){
    if(j%2 ==0){printf("\n");};
    printf("%+9.2e ",this->sys_d[j]);
  }; printf("\n");
  #endif
  int bad_d=0;
  for(uint i=1; i<this->sys_d.size(); i++){
    if(this->sys_d[i]<0.0){ bad_d++;
      this->sys_d[i]=std::abs(this->sys_d[i]);
    }; };
  #if VERB_MAX>10
  printf("System Preconditioner [%i]",(int)this->sys_d.size());
  for(uint j=0;j<this->sys_d.size();j++){
    if(j%2 ==0){printf("\n");};
    printf("%+9.2e ",this->sys_d[j]);
  }; printf("\n");
  #endif
  #if VERB_MAX>3
  if(bad_d>0){//FIXME Put this in calling method.
    printf("WARNING: %i negative preconditioner values corrected.\n",
      bad_d); };
  #endif
  //this->sys_d=1.0/this->sys_d;
  return( bad_d );
}; 
