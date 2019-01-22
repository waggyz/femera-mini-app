#include <utility>//std::pair
#include <vector>
#include <set>// This is ordered
#include <algorithm>    // std::copy
#include <unordered_map>
#include <vector>
#include <tuple>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <omp.h>
#include "femera.h"

int PCG::BC (Mesh* ){return 0;};
int PCG::RHS(Mesh* ){return 0;};
//
int PCG::RHS(Elem* E, Phys* Y ){
  uint d=uint(Y->ndof_n);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->rhs_vals ){ std::tie(n,f,v)=t;
    this->sys_r[d* n+uint(f)]+= v;
  };
  return(0);
};
int PCG::BCS(Elem* E, Phys* Y ){
  uint d=uint(Y->ndof_n);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    //printf("FIX ID %i, DOF %i, val %+9.2e\n",i,E->bcs_vals[i].first,E->bcs_vals[i].second);
    this->sys_u[d* n+uint(f)] = v;
  };
  return(0);
};
int PCG::BC0(Elem* E, Phys* Y ){
  uint d=uint(Y->ndof_n);
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    this->sys_d[d* n+uint(f)]=0.0;
    this->sys_f[d* n+uint(f)]=0.0;
  };
  for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
    this->sys_d[d* n+uint(f)]=0.0;
    #if VERB_MAX>10
    printf("BC0: [%i]:0\n",E->bc0_nf[i]);
    #endif
  };
  return(0);
};
int PCG::Setup( Elem* E, Phys* Y ){
  //this->meth_name="preconditioned conjugate gradient";
  this->halo_loca_0 = E->halo_remo_n * Y->ndof_n;
  //this->udof_n = E->node_n * Y->ndof_n;
  //this->udof_flop = 12;//*elem_n
  //this->udof_band = 14*sizeof(FLOAT_SOLV);//*udof_n + 2
  //this->Setup();
  //sys_u.resize(udof_n,0.0);// Initial Solution Guess
  //sys_f.resize(udof_n,0.0);// f=Au
  //if(sys_d.size()<udof_n){ sys_d.resize(udof_n,1.0); };
  // Default Diagonal Preconditioner
  //sys_r.resize(udof_n,0.0);// Residuals
  //sys_p.resize(udof_n,0.0);// CG working vector
  //sys_z.resize(udof_n,0.0);// CG working vector [no longer needed]
  //sys_b.resize(udof_n,0.0);// RHS [no longer needed]
  //Y->elem_inout.resize(uint(Y->ndof_n)*uint(E->elem_conn_n)*E->elem_n);
  //FIXED The above won't be needed anymore
  //this->Precond( E,Y );//FIXME Precond doesn't rotate yet
  //this->sys_d=FLOAT_SOLV(1.0)/this->sys_d;//NOTE sync sys_d before inverting it!
  //
  this->RHS( E,Y );
  this->BCS( E,Y );
  //Y->ScatterNode2Elem(E,this->sys_u,Y->elem_inout);
  //Y->ElemLinear( E );
  //Y->GatherElem2Node(E,Y->elem_inout,this->sys_f);
  E->do_halo=true;
  Y->ElemLinear( E,this->sys_f,this->sys_u );
  E->do_halo=false;
  Y->ElemLinear( E,this->sys_f,this->sys_u );
  return(0);
  //FIXME HALO SYNC this->sys_f (can be after exit) *****************
};
int PCG::Init( Elem* E, Phys* Y ){
  this->BC0( E,Y );
  return 0;
};
int PCG::Init(){
  const uint n=sys_u.size();// loca_res2 is a member variable.
  const uint sumi0=this->halo_loca_0;// *this->ndof_n;
  this->sys_r -= this->sys_f;
  //FLOAT_PHYS avg_d=S->sys_d.sum()/S->sys_d.size();
  //sys_r  = sys_b - sys_f;// This is done sparsely now.
  //sys_z  = sys_d * sys_r;
  //sys_p  = sys_z;
  //loca_res2    = inner_product( sys_r,sys_z );
  //FIXED Try inlining sys_z into next two
  sys_p  = sys_d * sys_r;
  //loca_res2    = inner_product( sys_r,sys_d * sys_r );
  //FLOAT_SOLV r=0.0;
  this->loca_res2=0.0;
//#pragma omp parallel for reduction(+:r)
  for(uint i=sumi0; i<n; i++){
    this->loca_res2 += sys_r[i] * sys_r[i] * sys_d[i]; };
  //this->loca_res2=r; //FIXME HALO SUM this->loca_res2 ***************************
  this->loca_rto2 = this->loca_rtol*loca_rtol *loca_res2;//FIXME Move this somewhere.
  //    printf("r2a: %9.2e, sys_r:\n",this->r2a);
  //    for(uint i=0; i<sys_r.size()/3; i++){
  //      printf("%u: %9.2e %9.2e %9.2e\n",i,
  //        sys_r[3*i],sys_r[3*i+1],sys_r[3*i+2]); };
  return(0);
};
int PCG::Iter(){// 2 FLOP + 12 FLOP/DOF, 14 float/DOF
  //NOTE Compute current sys_f=[k][p] before iterating with this.
  const uint n=sys_u.size();
  const uint sumi0=this->halo_loca_0;// *this->ndof_n;//FIXME Magic number
  const auto ra=this->loca_res2;// Make a local version of this member variable
  //
  //const FLOAT_SOLV alpha  = ra / inner_product( sys_p,sys_f );// 1 DIV +(2 FLOP/DOF)
  FLOAT_SOLV s=0.0;
//#pragma omp parallel for reduction(+:s)
  for(uint i=sumi0; i<n; i++){// 2 FLOP/DOF, 2 read =2/DOF
    s += sys_p[i] * sys_f[i];
  };//FIXME HALO SUM s **********************************************
  const FLOAT_SOLV alpha  = ra / s;// 1 DIV FLOP
  //
  //if(!isnan(alpha)){// moved to inside last loop
  //  sys_u += alpha * sys_p; };
  //if( ra < to2 ){ return(SOLV_CNVG_PTOL);};// moved to after last loop
  //sys_r -= alpha * sys_f;//Moved inside next loop
  //sys_z  = sys_d * sys_r;
  //r2b    = inner_product( sys_r,sys_z );
  //sys_p  = sys_z + (r2b/loca_res2)*sys_p;
  //FIXED Inlined sys_z into next two loops...
  FLOAT_SOLV r2b=0.0;
  for(uint i=0; i<n; i++){// 4 FLOP/DOF, 4 read + 2 write =6/DOF
    sys_u[i] += alpha * sys_p[i];// works fine here
    sys_r[i] -= alpha * sys_f[i];
  };
//#pragma omp parallel for reduction(+:r2b)
  for(uint i=sumi0; i<n; i++){// 3 FLOP/DOF, 2 read =2/DOF
    r2b += sys_r[i] * sys_r[i] * sys_d[i];
  };//FIXME HALO SUM r2b ********************************************
  if( ra < loca_rto2 ){ return(SOLV_CNVG_PTOL);};
  //sys_p  = sys_d * sys_r + (r2b/ra)*sys_p;
  const FLOAT_PHYS beta = r2b/ra;//  1 FLOP
  this->loca_res2 = r2b;// Update member residual (squared)
//#pragma omp parallel for
  for(uint i=0; i<n; i++){// 3 FLOP/DOF, 3 read + 1 write =4/DOF
    //sys_u[i] += alpha * sys_p[i];// Better data locality here, but not faster
    sys_p[i] = sys_d[i] * sys_r[i] + beta * sys_p[i];
  };
  //loca_res2 = r2b;
  //if( r2b < to2 ){ return(SOLV_CNVG_PTOL);};
  return(0);
};
int PCG::Solve( Elem* E, Phys* Y ){//FIXME Redo this
  //this->Setup( E,Y );
  this->Init( E,Y );
  if( !this->Init() ){//============ Solve ================
    printf("SER INIT r2a:%9.2e\n",this->loca_rto2);
    for(this->iter=0; this->iter < this->iter_max; this->iter++){
      this->sys_f=0.0;
      //Y->ScatterNode2Elem(E,this->sys_p,Y->elem_inout);
      //Y->ElemLinear( E );
      //Y->GatherElem2Node(E,Y->elem_inout,this->sys_f);
      //Y->ElemLinear( E,this->sys_f,this->sys_p );
      E->do_halo=true;
      Y->ElemLinear( E,this->sys_f,this->sys_p );
      E->do_halo=false;
      Y->ElemLinear( E,this->sys_f,this->sys_p );
      //FIXME HALO SYNC this->sys_f
      if( this->Iter() ){ break ;};// CG Iteration
#if VERB_MAX>2
      if(!((iter+1) % 100) ){
        printf("%6i ||R||%9.2e\n", iter+1, std::sqrt(loca_res2) ); };
          //std::sqrt(loca_res2), std::sqrt(this->loca_rto2) ); };
#endif
    };// End iteration loop.
  };
  return 0;//FIXME
};
int HaloPCG::Init(){// Preconditioned Conjugate Gradient
  int part_0=0;
  if(std::get<0>( this->mesh_part[0] )==NULL){ part_0=1; };
  const int part_n = int(this->mesh_part.size())-part_0;
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  //
  INT_MESH halo_n=0;
  // Local copies for atomic ops and reduction
  FLOAT_SOLV glob_r2a = this->glob_res2, glob_to2 = this->glob_rto2;
#pragma omp parallel num_threads(comp_n)
{// parallel init region
  long int my_scat_count=0, my_prec_count=0,
    my_gat0_count=0,my_gat1_count=0, my_gmap_count=0, my_solv_count=0;
  auto start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    S->Precond( E,Y );
  };
  // Sync sys_d [this inits halo_map]
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_prec_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
  Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
  const INT_MESH d=uint(Y->ndof_n);
  for(INT_MESH i=0; i<E->halo_node_n; i++){
#pragma omp critical(halomap)
{//FIXME critical section here?
      INT_MESH g=E->node_glid[i];
      if(this->halo_map.count(g)==0){// Initialize halo_map
        this->halo_map[g]=halo_n; halo_n++;
        for( uint j=0; j<d; j++){
          this->halo_val[d*this->halo_map[g]+j]  = S->sys_d[d*i +j]; };
      }else{// Add in the rest.
        for( uint j=0; j<d; j++){
          this->halo_val[d*this->halo_map[g]+j] += S->sys_d[d*i +j]; };
      };
}
    };
  };// End sys_d gather
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gmap_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_d[d*i +j] = this->halo_val[f+j]; };
    };
  };// end sys_d scatter
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_scat_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    S->sys_d=FLOAT_SOLV(1.0)/S->sys_d;
    S->Init( E,Y );// Zeros boundary conditions
    /*
    //S->Setup(E,Y);// Applies BCs & sets S->udof_flop and udof_band
    //S->BC0( E,Y );//FIXME Pasted below until Solv* class structure fixed
    const INT_MESH d=uint(Y->ndof_n);
    INT_MESH n; INT_DOF f; FLOAT_PHYS v;
    for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
      S->sys_d[d* n+uint(f)]=0.0; };
      //S->sys_f[d* n+uint(f)]=0.0; };//FIXME redundant?
    for(auto t : E->bc0_nf   ){ std::tie(n,f)=t;
      S->sys_d[d* n+uint(f)]=0.0; };*/
    //for(INT_MESH i=0; i<E->halo_node_n; i++){
    //  for( uint j=0; j<ndof_n; j++){// parallel halo_vals zero
    //    halo_vals[E->node_glid[i]][j] =0.0; }; };
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_solv_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp single
{   this->halo_val = 0.0; }// serial halo_vals zero
  my_gat0_count +=dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j] += S->sys_f[d*i +j]; };
    };
  };// End halo_vals 
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_gat1_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp for schedule(static) reduction(+:glob_r2a)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    for(INT_MESH i=0; i<E->halo_node_n; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for( uint j=0; j<d; j++){
#pragma omp atomic read
        S->sys_f[d*i +j] = this->halo_val[f+j]; };
    };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_scat_count += dur.count();
  start = std::chrono::high_resolution_clock::now();
#pragma omp critical(init)
{ S->Init(); }
  glob_r2a += S->loca_res2;
  };
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=this->mesh_part[part_i];
    S->loca_rto2 = S->loca_rtol*S->loca_rtol *glob_r2a;
#pragma omp atomic write
    glob_to2 = S->loca_rto2;// Pass the relative tolerance out.
  };
  dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  my_solv_count += dur.count();
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_prec_count)*1e-9;
  this->time_secs[1]+=float(my_gmap_count)*1e-9;
  this->time_secs[2]+=float(my_gat0_count)*1e-9;
  this->time_secs[3]+=float(my_gat1_count)*1e-9;
  this->time_secs[4]+=float(my_scat_count)*1e-9;
  this->time_secs[5]+=float(my_solv_count)*1e-9;
}
}// end init parallel region
  this->glob_res2 = glob_r2a;
  this->glob_chk2 = glob_r2a;
  this->glob_rto2 = glob_to2;// / ((FLOAT_SOLV)this->udof_n);
  return 0;
};
int HaloPCG::Iter(){
  int part_0=0;
  if(std::get<0>( this->mesh_part[0])==NULL ){ part_0=1; };
  const int part_n = int(this->mesh_part.size())-part_0;
#ifdef _OPENMP
  const int comp_n = this->comp_n;
#endif
  //
  //bool halo_update=true;
  //if( (iter % halo_mod)==0 ){ halo_update=true; }else{ halo_update=false; };
  //
  FLOAT_SOLV glob_sum1=0.0, glob_sum2=0.0;
  FLOAT_SOLV glob_r2a = this->glob_res2;
  const auto P=this->mesh_part;//FIXME Undo this?
#pragma omp parallel num_threads(comp_n)
{// iter parallel region
  long int my_phys_count=0, my_scat_count=0, my_solv_count=0,
    my_gat0_count=0,my_gat1_count=0;
  std::chrono::high_resolution_clock::time_point iter_start,
    solv_start, inte_start, iter_done;
  std::chrono::high_resolution_clock::time_point
    gath_start, scat_start, phys_start;
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (phys_start-phys_start);
  iter_start = std::chrono::high_resolution_clock::now();
  //nowait
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    phys_start = std::chrono::high_resolution_clock::now();
    S->sys_f=0.0;
    E->do_halo=true;
    Y->ElemLinear( E, S->sys_f, S->sys_p );
    dur = std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-phys_start);
    my_phys_count += dur.count();
    gath_start = std::chrono::high_resolution_clock::now();
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for(uint j=0; j<d; j++){
#pragma omp atomic write
        this->halo_val[f+j] = S->sys_f[d* i+j]; };
    };
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-gath_start);
    my_gat0_count += dur.count();
  };
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    gath_start = std::chrono::high_resolution_clock::now();
    const INT_MESH d=uint(Y->ndof_n);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        this->halo_val[f+j]+= S->sys_f[d* i+j]; };
    };
  };// End halo_vals sum; now scatter back to elems
  dur= std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-gath_start);
  my_gat1_count += dur.count();
#pragma omp for schedule(static) reduction(+:glob_sum1)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    scat_start = std::chrono::high_resolution_clock::now();
    const INT_MESH hnn=E->halo_node_n,hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hnn; i++){
      auto f = d* this->halo_map[E->node_glid[i]];
      for( uint j=0; j<d; j++){//NOTE appears not to be critical
//#pragma omp atomic read
        S->sys_f[d* i+j] = this->halo_val[f+j]; };
    };
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-scat_start);
    my_scat_count += dur.count();
    phys_start = std::chrono::high_resolution_clock::now();
    E->do_halo=false;
    Y->ElemLinear( E, S->sys_f, S->sys_p );
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-phys_start);
    my_phys_count += dur.count();
    //
    solv_start = std::chrono::high_resolution_clock::now();
    //FLOAT_SOLV loca_sum=0.0;
    for(INT_MESH i=hl0; i<sysn; i++){
      //loca_sum += S->sys_p[i] * S->sys_f[i];
      glob_sum1 += S->sys_p[i] * S->sys_f[i];
    };
    dur= std::chrono::duration_cast<std::chrono::nanoseconds>
      (std::chrono::high_resolution_clock::now()-solv_start);
    my_solv_count += dur.count();
//#pragma omp atomic
//        glob_sum1 += loca_sum;
  };
  solv_start = std::chrono::high_resolution_clock::now();
  const FLOAT_SOLV alpha = glob_r2a / glob_sum1;// 1 FLOP
#pragma omp for schedule(static) reduction(+:glob_sum2)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){// ? FLOP/DOF
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH hl0=S->halo_loca_0,sysn=S->udof_n;
    for(INT_MESH i=0; i<hl0; i++){
      S->sys_r[i] -= alpha * S->sys_f[i]; };//printf("DOF: %u\n",Y->ndof_n);
    for(INT_MESH i=hl0; i<sysn; i++){
      //r2b += S->sys_r[i] * S->sys_r[i] * S->sys_d[i];
      S->sys_r[i] -= S->sys_f[i] * alpha;
      glob_sum2   += S->sys_r[i] * S->sys_r[i] * S->sys_d[i]; };
  };
  const FLOAT_PHYS beta = glob_sum2 / glob_r2a;// 1 FLOP
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH sysn=S->udof_n;
    //sys_p  = sys_d * sys_r + (r2b/ra)*sys_p;
    //S->r2a = glob_sum2;// Update member residual (squared)
    for(INT_MESH i=0; i<sysn; i++){// ? FLOP/DOF
      S->sys_u[i] += S->sys_p[i] * alpha;// better data locality here
      S->sys_p[i]  = S->sys_d[i] * S->sys_r[i] + beta*S->sys_p[i]; };
  };
#pragma omp single nowait
{ glob_r2a = glob_sum2; }// Update residual (squared)
  iter_done  = std::chrono::high_resolution_clock::now();
  auto solv_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (iter_done - solv_start);
  auto iter_time  = std::chrono::duration_cast<std::chrono::nanoseconds>
    (iter_done - iter_start);// printf("%i ",iter);
#pragma omp critical(time)
{
  this->time_secs[0]+=float(my_phys_count)*1e-9;
  this->time_secs[1]+=float(my_gat0_count)*1e-9;
  this->time_secs[2]+=float(my_gat1_count)*1e-9;
  this->time_secs[3]+=float(my_scat_count)*1e-9;
  this->time_secs[4]+=float(my_solv_count+solv_time.count())*1e-9;
 //this->time_secs[4]+=float(solv_time.count())*1e-9;
  this->time_secs[5]+=float(iter_time.count())*1e-9;
}
}// end iter parallel region
  this->glob_res2 = glob_r2a;
  this->glob_chk2 = glob_r2a;
  return 0;
};
