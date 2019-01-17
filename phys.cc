#ifdef _OPENMP
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>// std::memcpy
//#include <omp.h>
#include "femera.h"
int Phys::ScatterNode2Elem(Elem* E,//FIXME
  RESTRICT const Phys::vals & node_v,
  RESTRICT       Phys::vals & elem_v ){
  //const uint mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint conn_n = E->elem_conn_n;
  const uint N      = elem_n*conn_n;
  const uint Nv     = elem_n*conn_n*this->ndof_n;
  if(elem_v.size()!=Nv){ elem_v.resize(Nv); };//FIXME
  //FIXED only need one thread for each memory controller?
//#ifdef _OPENMP
//  int numa_n = omp_get_max_threads() / 2;//FIXME just a guess
//  if( numa_n<2 ){ numa_n=2; };
//#endif
//#pragma omp parallel for schedule(static) num_threads(numa_n)
  for(uint i=0; i<N; i++){//FIXME replace elem_d with dofs_n
    //elem_v[std::slice((i/conn_n)*this->ndof_n*conn_n+(i%conn_n),this->ndof_n,conn_n)]
    elem_v[std::slice( i*this->ndof_n,this->ndof_n,1 )]
      =node_v[std::slice(E->elem_conn[i]*this->ndof_n,this->ndof_n,1)];
  };
  //elem_v=sys_v;
  return 0;
};
int Phys::GatherElem2Node(Elem* E,
  RESTRICT const Phys::vals & elem_v,
  RESTRICT       Phys::vals & node_v ){
  //const uint mesh_d = E->elem_d;
  const uint elem_n = E->elem_n;
  const uint conn_n = E->elem_conn_n;
  const uint N      = elem_n*conn_n;
//#ifdef _OPENMP
//  int numa_n = omp_get_max_threads() / 2;
//  if( numa_n<2 ){ numa_n=2; };
//#endif
  //FIXED It's still faster to do this serially
//#pragma omp parallel for schedule(static) num_threads(numa_n)
  for (uint i=0; i<N; i++){//FIXED replace elem_d with dofs_n
//#ifdef _OPENMP
//    auto c=E->elem_conn[i] * this->ndof_n ;
//    for(uint j=0;j<this->ndof_n;j++){
//#pragma omp atomic update
//      node_v[c+j]+=elem_v[i*this->ndof_n+j ];
//    };
//#else
    node_v[std::slice(E->elem_conn[i]*this->ndof_n,this->ndof_n,1)]
      +=elem_v[std::slice( i*this->ndof_n,this->ndof_n,1 )];
//#endif
  };
//#pragma omp parallel
//  {     const int thread_n = omp_get_max_threads();
//        const int thread_i = omp_get_thread_num();
//        const int s=node_v.size()/mesh_d/thread_n;
//  for (uint i=0; i<N; i++){
//    INT_MESH c=E->elem_conn[i]*mesh_d;
//    if( c>=thread_i*s ){ if(c<(thread_i+1)*s ){
//      node_v[std::slice(c*mesh_d,mesh_d,1)]
//        +=elem_v[std::slice( i*mesh_d,mesh_d,1 )]; }; };
//    };
  return 0;
};/*
int Phys::JacRot( Elem* E ){//
  static const int mesh_d = E->elem_d;;
  static const uint  Nj = mesh_d*mesh_d;
  const uint elem_n = E->elem_n;
  const int intp_n = E->elip_dets.size()/elem_n;
  //
  for(uint ie=0;ie<elem_n;ie++){
    for(int ip=0;ip<intp_n;ip++){
      E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,Nj,1)]=MatMul3x3xN( mtrl_rotc,
        E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,Nj,1)]);
      //for(uint k=0; k<3; k++){
      //for(uint i=0; i<3; i++){
      //for(uint j=0; j<3; j++){
    //    H[3*k+0]=rotc[3*0+0]*h[3*k+0]+rotc[3*1+0]*h[3*k+1]+rotc[3*2+0]*h[3*k+2];
    //    H[3*k+1]=rotc[3*0+1]*h[3*k+0]+rotc[3*1+1]*h[3*k+1]+rotc[3*2+1]*h[3*k+2];
    //    H[3*k+2]=rotc[3*0+2]*h[3*k+0]+rotc[3*1+2]*h[3*k+1]+rotc[3*2+2]*h[3*k+2];
      };//};//};
    };//end intp loop
  };//end elem loop
  return 0;
};*/
int Phys::JacRot( Elem* E ){
  static const int mesh_d = E->elem_d;;
  #if VERB_MAX>10
  printf("Material Orientation (zxz) (rad):");
  for(uint i=0; i<mtrl_dirs.size(); i++){
    printf("%10.3e ",mtrl_dirs[i]);
  };// printf("\n");
  #endif
  if((mtrl_rotc.size()!=9) ){
    mtrl_rotc.resize(9);
    mtrl_rotc[std::slice(0,mesh_d,mesh_d+1)]=1.0;
  };
  if(mtrl_dirs.size()==3){
    mtrl_rotc.resize(9);
    const FLOAT_PHYS z1=mtrl_dirs[0];// Rotation about z (radians)
    const FLOAT_PHYS x2=mtrl_dirs[1];// Rotation about x (radians)
    const FLOAT_PHYS z3=mtrl_dirs[2];// Rotation about z (radians)
    Phys::vals Z1={cos(z1),sin(z1),0.0, -sin(z1),cos(z1),0.0, 0.0,0.0,1.0};
    Phys::vals X2={1.0,0.0,0.0, 0.0,cos(x2),sin(x2), 0.0,-sin(x2),cos(x2)};
    Phys::vals Z3={cos(z3),sin(z3),0, -sin(z3),cos(z3),0.0, 0.0,0.0,1.0};
    mtrl_rotc = MatMul3x3xN(Z1,MatMul3x3xN(X2,Z3));
    //mtrl_rotc=MatMul3x3xN(R,R);
    #if VERB_MAX>10
    printf("Material Tensor Rotation:");
    for(uint i=0; i<mtrl_rotc.size(); i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",mtrl_rotc[i]);
    }; printf("\n");
    #endif
  };
  if(mtrl_rotc.size()==9){
    static const uint  Nj = 10,d2=9;//mesh_d*mesh_d;
    const uint elem_n = E->elem_n;
    const int intp_n = E->elip_jacs.size()/elem_n/Nj;//E->elip_dets.size()/elem_n;
    Phys::vals jac(d2),J(d2);
    for(uint ie=0;ie<elem_n;ie++){
      for(int ip=0;ip<intp_n;ip++){
        std::copy(&E->elip_jacs[ie*intp_n*Nj+ip*Nj],
                  &E->elip_jacs[ie*intp_n*Nj+ip*Nj+d2], &jac[0] );
        #if VERB_MAX>10
        printf("Jac:");
        for(uint i=0; i<9; i++){
          if(!(i%3)){printf("\n");};
          printf("%10.3e ",jac[i]);
        }; printf("\n");
        #endif
        J=MatMul3x3xN(mtrl_rotc,jac);
        //Phys::vals J=MatMul3x3xN( mtrl_rotc,MatMul3x3xNT(jac,mtrl_rotc));
        /*
        FLOAT_PHYS dx=1.0/sqrt(jac[0]*jac[0] + jac[3]*jac[3] + jac[6]*jac[6]);
        FLOAT_PHYS dy=1.0/sqrt(jac[1]*jac[1] + jac[4]*jac[4] + jac[7]*jac[7]);
        FLOAT_PHYS dz=1.0/sqrt(jac[2]*jac[2] + jac[5]*jac[5] + jac[8]*jac[8]);
        Phys::vals R ={
          jac[0]*dx,jac[1]*dy,jac[2]*dz,
          jac[3]*dx,jac[4]*dy,jac[5]*dz,
          jac[6]*dx,jac[7]*dy,jac[8]*dz};
        //
        Phys::vals RJ=MatMul3x3xN(R,J); J=RJ;
        */
        #if VERB_MAX>10
        printf("Jac Rotated:");
        for(uint i=0; i<9; i++){
          if(!(i%3)){printf("\n");};
          printf("%10.3e ",J[i]);
        }; printf("\n");
        #endif
        //Phys::vals J(9),R=mtrl_rotc,
        //  I=E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,Nj,1)];
        //for(uint k=0; k<3; k++){
        ////for(uint i=0; i<3; i++){
        ////for(uint j=0; j<3; j++){
        //  J[3*k+0]=R[3*0+0]*I[3*k+0]+R[3*1+0]*I[3*k+1]+R[3*2+0]*I[3*k+2];
        //  J[3*k+1]=R[3*0+1]*I[3*k+0]+R[3*1+1]*I[3*k+1]+R[3*2+1]*I[3*k+2];
        //  J[3*k+2]=R[3*0+2]*I[3*k+0]+R[3*1+2]*I[3*k+1]+R[3*2+2]*I[3*k+2];
        //};//};//};
        E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,d2,1)]=J;
      };//end intp loop
    };//end elem loop
    return 0;
  }else{ return 1;};
};
//-------------------------------------------------------------------
int Phys::ReadPartFMR( const char* fname, bool is_bin ){
  //FIXME This is not used. It's done in Elem::ReadPartFMR...
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not open "<< fname << " for reading." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  };
  std::string fmrstring;
  std::ifstream fmrfile(fname);
  while( fmrfile >> fmrstring ){
    if(fmrstring=="$ElasticProperties"){
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i]; };
      //this->MtrlProp2MatC();
      s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; mtrl_dirs[i]*=(PI/180.0) ;};
      };
    };
  };
  return 0;
};
int Phys::SavePartFMR( const char* fname, bool is_bin ){
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";};
  if(is_bin){
    std::cout << "ERROR Could not append "<< fname << "." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  };
  std::ofstream fmrfile;
  fmrfile.open(fname, std::ios_base::app);
  //
  fmrfile << "$ElasticProperties" <<'\n';
  fmrfile << mtrl_prop.size();
  for(uint i=0;i<mtrl_prop.size();i++){ fmrfile <<" "<< mtrl_prop[i]; };
  fmrfile << '\n';
  if(mtrl_dirs.size()>0){
    fmrfile << mtrl_dirs.size();
    for(uint i=0;i<mtrl_dirs.size();i++){ fmrfile <<" "<< mtrl_dirs[i]; };
  }; fmrfile <<'\n';
  return 0;
};
