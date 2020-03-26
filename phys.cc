#ifdef _OPENMP
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include <cstring>// std::memcpy
#include "femera.h"
int Phys::ScatterNode2Elem(Elem* E,//FIXME
  RESTRICT const Phys::vals & node_v,
  RESTRICT       Phys::vals & elem_v ){
  const uint elem_n = E->elem_n;
  const uint conn_n = E->elem_conn_n;
  const uint N      = elem_n*conn_n;
  const uint Nv     = elem_n*conn_n*this->node_d;
  if(elem_v.size()!=Nv){ elem_v.resize(Nv); }//FIXME
  for(uint i=0; i<N; i++){//FIXME replace elem_d with dofs_n
    elem_v[std::slice( i*this->node_d,this->node_d,1 )]
      =node_v[std::slice(E->elem_conn[i]*this->node_d,this->node_d,1)];
  }
  return 0;
}
int Phys::GatherElem2Node(Elem* E,
  RESTRICT const Phys::vals & elem_v,
  RESTRICT       Phys::vals & node_v ){
  const uint elem_n = E->elem_n;
  const uint conn_n = E->elem_conn_n;
  const uint N      = elem_n*conn_n;
  for (uint i=0; i<N; i++){
    node_v[std::slice(E->elem_conn[i]*this->node_d,this->node_d,1)]
      +=elem_v[std::slice( i*this->node_d,this->node_d,1 )];
  }
  return 0;
}
int Phys::IniRot(){
  static const int mesh_d = 3;
  #if VERB_MAX>10
  printf("Material Orientation (zxz) (rad):");
  for(uint i=0; i<mtrl_dirs.size(); i++){
    printf("%10.3e ",mtrl_dirs[i]);
  }// printf("\n");
  #endif
  if((mtrl_rotc.size()!=9) ){
    mtrl_rotc.resize(9);
    mtrl_rotc[std::slice(0,mesh_d,mesh_d+1)]=1.0;
  }
  if(mtrl_dirs.size()==3){
    mtrl_rotc.resize(9);
    const FLOAT_PHYS z1=mtrl_dirs[0];// Rotation about z (radians)
    const FLOAT_PHYS x2=mtrl_dirs[1];// Rotation about x (radians)
    const FLOAT_PHYS z3=mtrl_dirs[2];// Rotation about z (radians)
    Phys::vals Z1={cos(z1),sin(z1),0.0, -sin(z1),cos(z1),0.0, 0.0,0.0,1.0};
    Phys::vals X2={1.0,0.0,0.0, 0.0,cos(x2),sin(x2), 0.0,-sin(x2),cos(x2)};
    Phys::vals Z3={cos(z3),sin(z3),0.0, -sin(z3),cos(z3),0.0, 0.0,0.0,1.0};
    //mtrl_rotc = MatMul3x3xN(Z1,MatMul3x3xN(X2,Z3));
    for(int i=0;i<3;i++){
      for(int l=0;l<3;l++){ mtrl_rotc[3* i+l ]=0.0;
        for(int j=0;j<3;j++){
          for(int k=0;k<3;k++){
            mtrl_rotc[3* i+l ] += Z1[3* i+j ] * X2[3* j+k ] * Z3[3* k+l ];
    } } } }
    //mtrl_rotc=MatMul3x3xN(R,R);
    #if VERB_MAX>10
    printf("Material Tensor Rotation:");
    for(uint i=0; i<mtrl_rotc.size(); i++){
      if(!(i%3)){printf("\n");}
      printf("%10.3e ",mtrl_rotc[i]);
    } printf("\n");
    #endif
  return 0;
  }
  return 1;
}
#if 0
int Phys::MtrRot(){
  if(mtrl_rotc.size()==9){ if( mtrl_matc.size()>35 ){
    //
    // inline function Voigt3Index( i,j );
    // Vi = (i==j) ? i : 2+i+j;// Voigt vector index {xx,yy,zz,xy,xz,yz}
  return 0;
  } }
  return 1;
}
#endif
int Phys::JacRot( Elem* E ){
  if(mtrl_rotc.size()==9){
    static const uint  Nj = 10,d2=9;
    const uint elem_n = E->elem_n;
    const int intp_n = E->elip_jacs.size()/elem_n/Nj;
    Phys::vals jac(d2),J(d2);
    for(uint ie=0;ie<elem_n;ie++){
      for(int ip=0;ip<intp_n;ip++){
        std::copy(&E->elip_jacs[ie*intp_n*Nj+ip*Nj],
                  &E->elip_jacs[ie*intp_n*Nj+ip*Nj+d2], &jac[0] );
        #if VERB_MAX>10
        printf("Jac:");
        for(uint i=0; i<9; i++){
          if(!(i%3)){printf("\n");}
          printf("%10.3e ",jac[i]);
        } printf("\n");
        #endif
        //J=MatMul3x3xN(mtrl_rotc,jac);
        for(int i=0;i<3;i++){
          for(int k=0;k<3;k++){ J[3* i+k ]=0.0;
            for(int j=0;j<3;j++){
              J[3* i+k ] += mtrl_rotc[3* i+j ] * jac[3* j+k ];
        } } }
        #if VERB_MAX>10
        printf("Jac Rotated:");
        for(uint i=0; i<9; i++){
          if(!(i%3)){printf("\n");}
          printf("%10.3e ",J[i]);
        } printf("\n");
        #endif
        E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,d2,1)]=J;
      }//end intp loop
    }//end elem loop
    return 0;
  }else{ return 1;}
}
int Phys::JacT  ( Elem* E ){
  static const uint  Nj = 10,d2=9;//mesh_d*mesh_d;
  const uint elem_n = E->elem_n;
  const int intp_n = E->elip_jacs.size()/elem_n/Nj;
  Phys::vals J(d2),JT(d2);
  for(uint ie=0;ie<elem_n;ie++){
    for(int ip=0;ip<intp_n;ip++){
      std::copy(&E->elip_jacs[ie*intp_n*Nj+ip*Nj],
                &E->elip_jacs[ie*intp_n*Nj+ip*Nj+d2], &J[0] );
      for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
          JT[3* i+j ] = J[3* j+i ]; } }
      E->elip_jacs[std::slice(ie*intp_n*Nj+ip*Nj,d2,1)]=JT;
    }
  }
  return 0;
}
//-------------------------------------------------------------------
int Phys::ReadPartFMR( const char* fname, bool is_bin ){
  //FIXME This is not used. It's done in Mesh::ReadPartFMR...
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";}
  if(is_bin){
    std::cout << "ERROR Could not open "<< fname << " for reading." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  }
  std::string fmrstring;
  std::ifstream fmrfile(fname);
  while( fmrfile >> fmrstring ){
    if(fmrstring=="$ElasticProperties"){//FIXME Deprecated
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i]; }
      //this->MtrlProp2MatC();
      s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; mtrl_dirs[i]*=(PI/180.0) ;}
      }
    }
    if(fmrstring=="$Orientation"){// Material orientation (radians)
      int s=0; fmrfile >> s;
      if(s>0){
        mtrl_dirs.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> mtrl_dirs[i]; mtrl_dirs[i]*=(PI/180.0) ;}
      }
    }
    //FIXED This parsing no longer requires properties in a specific order
    if(fmrstring=="$Elastic"){// Elastic Constants
      int s=0; fmrfile >> s;
      if(s<21){
        elas_prop.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> elas_prop[i]; }
      }else{
        mtrl_matc.resize(s);
        for(int i=0; i<s; i++){ fmrfile >> mtrl_matc[i]; }
      }
    }
    if(fmrstring=="$ThermalExpansion"){// Thermal expansion
      int s=0; fmrfile >> s;
      ther_expa.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> ther_expa[i]; }
    }
    if(fmrstring=="$ThermalConductivity"){// Thermal conductivity
      int s=0; fmrfile >> s;
      ther_cond.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> ther_cond[i]; }
    }
    if(fmrstring=="$Plastic"){// Plasticity Constants
      int s=0; fmrfile >> s;
      plas_prop.resize(s);
      for(int i=0; i<s; i++){ fmrfile >> plas_prop[i]; }
    }
  }
  return 0;
}
int Phys::SavePartFMR( const char* fname, bool is_bin ){
  std::string s; if(is_bin){ s="binary";}else{s="ASCII";}
  if(is_bin){
    std::cout << "ERROR Could not append "<< fname << "." <<'\n'
      << "ERROR Femera (fmr) "<< s <<" format not yet supported." <<'\n';
    return 1;
  }
  std::ofstream fmrfile;
  fmrfile.open(fname, std::ios_base::app);
  //
  elas_prop.resize(mtrl_prop.size());
  for(uint i=0;i<mtrl_prop.size();i++){elas_prop[i] = mtrl_prop[i];}
#if 0
  fmrfile << "$ElasticProperties" <<'\n';//FIXME Deprecated
  fmrfile << mtrl_prop.size();
  for(uint i=0;i<mtrl_prop.size();i++){ fmrfile <<" "<< mtrl_prop[i]; }
  fmrfile << '\n';
  if(mtrl_dirs.size()>0){
    fmrfile << mtrl_dirs.size();
    for(uint i=0;i<mtrl_dirs.size();i++){ fmrfile <<" "<< mtrl_dirs[i]; }
    fmrfile <<'\n';
  }
#endif
  // Replaced $ElasticProperties with these
  if(mtrl_dirs.size()>0){
    fmrfile << "$Orientation" <<'\n';
    fmrfile << mtrl_dirs.size();
    for(uint i=0;i<mtrl_dirs.size();i++){ fmrfile <<" "<< mtrl_dirs[i]; }
    fmrfile <<'\n';
  }
  if(mtrl_dmat.size()>0){
    fmrfile << "$Elastic" <<'\n';
    if(mtrl_dmat.size()>36){
      fmrfile << 36;
      for(int i=0;i<6;i++){
        for(int j=0;j<6;j++){
          fmrfile <<" "<< mtrl_dmat[8*i+j]; } }
    }else{
      fmrfile << mtrl_dmat.size();
      for(uint i=0;i<mtrl_dmat.size();i++){ fmrfile <<" "<< mtrl_dmat[i]; }
    }
    fmrfile << '\n';
  }
  else if(elas_prop.size()>0){
    fmrfile << "$Elastic" <<'\n';
    fmrfile << elas_prop.size();
    for(uint i=0;i<elas_prop.size();i++){ fmrfile <<" "<< elas_prop[i]; }
    fmrfile << '\n';
  }
  if(ther_expa.size()>0){
    fmrfile << "$ThermalExpansion" <<'\n';
    fmrfile << ther_expa.size();
    for(uint i=0;i<ther_expa.size();i++){ fmrfile <<" "<< ther_expa[i]; }
    fmrfile << '\n';
  }
  if(ther_cond.size()>0){
    fmrfile << "$ThermalConductivity" <<'\n';
    fmrfile << ther_cond.size();
    for(uint i=0;i<ther_cond.size();i++){ fmrfile <<" "<< ther_cond[i]; }
    fmrfile << '\n';
  }
  if(plas_prop.size()>0){
    fmrfile << "$Plastic" <<'\n';
    fmrfile << plas_prop.size();
    for(uint i=0;i<plas_prop.size();i++){ fmrfile <<" "<< plas_prop[i]; }
    fmrfile << '\n';
  }
  return 0;
}