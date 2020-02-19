#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
//
int ThermIso3D::ElemLinear( Elem* ){ return 1; }//FIXME
int ThermIso3D::ElemJacobi( Elem* ){ return 1; }//FIXME
int ThermIso3D::BlocLinear( Elem* ,
  RESTRICT Phys::vals &, const RESTRICT Solv::vals & ){
  return 1;
  }
int ThermIso3D::ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*){
  return 1;
}
int ThermIso3D::ElemJacobi( Elem*, FLOAT_SOLV*, const FLOAT_SOLV* ){
  return 1; }
int ThermIso3D::ElemRowSumAbs(Elem*, FLOAT_SOLV* ){ return 1; };
int ThermIso3D::ElemStrain( Elem*, FLOAT_SOLV* ){ return 1; };
//
int ThermIso3D::ElemJacNode(Elem* E, FLOAT_SOLV* part_d ){
  return this->ElemJacobi( E, part_d );
}
//
int ThermIso3D::ElemNonlinear( Elem* E, const INT_MESH e0, const INT_MESH e1,
  FLOAT_SOLV* part_f, const FLOAT_SOLV* part_u, const FLOAT_SOLV*, bool ){
  return this->ElemLinear( E, e0,e1, part_f, part_u );
  }
int ThermIso3D::ElemStiff(Elem* E ){//FIXME should be ScatterStiff( E )
  const int Dm = 3;//E->mesh_d
  //const int Dn = 1;//this->node_d;
  const int Nj = 10;
  const int Nc = int(E->elem_conn_n);
  const int Ne = Dm*Nc;
  const int Nr = Nc;// One row/col of stiffness matrix
#ifdef __INTEL_COMPILER
  const int Nk =int(Nr*(Nr + 1))/2;// Packed symmetric stiffness
#else
  const int Nk = Nr * Nr;// Elements of stiffness matrix
#endif
  const INT_MESH elem_n = int(E->elem_n);
  const int intp_n = int(E->gaus_n);
  //
  const FLOAT_PHYS C = mtrl_matc[0];
  FLOAT_PHYS G[Ne];
  for(INT_MESH ie=0;ie<elem_n;ie++){
    for(int ip=0;ip<intp_n;ip++){
      int ig=ip*Ne;
      for(int i=0;i<Ne;i++){ G[i]=0.0; }
      for(int k=0;k<Nc;k++){
      for(int i=0;i<Dm;i++){
      for(int j=0;j<Dm;j++){
        G[Nc* i+k] += E->elip_jacs[Nj*ie+Dm* j+i ] * E->intp_shpg[ig+Dm* k+j ];
      } } }
      FLOAT_PHYS Cdw = C * E->elip_jacs[Nj*ie+9] * E->gaus_weig[ip];
#ifdef __INTEL_COMPILER
      int ik=-1;
#endif
      for(int i=0; i<Nr; i++){
      for(int l=0; l<Nr; l++){
#ifdef __INTEL_COMPILER
        ik=ik+int(i<=l);
        if(i<=l){
#endif
      for(int k=0; k<3 ; k++){
#ifdef __INTEL_COMPILER
        // Use packed symmmetric matrix storage.
          elem_stiff[Nk*ie + ik ]+=G[Nc* k+i ] * G[Nc* k+l ] * Cdw;
        }
#else
        elem_stiff[Nk*ie +Nr* i+l ]+=G[Nc* k+i ] * G[Nc* k+l ] * Cdw;
#endif
      } } }
    }// end intp loop
  }// End elem loop
  return 0;
}//============================================================== End ElemStiff
int ThermIso3D::ElemJacobi(Elem* E, FLOAT_SOLV* part_d ){
  //const uint ndof   = 3;//this->node_d
  const uint Dm =  3;
  //const uint Dn =  1;
  const uint Nj = 10;
  const uint Nc = E->elem_conn_n;
  const uint Ne = uint(Nc*Dm);
  const uint En = E->elem_n;
  const uint intp_n = E->gaus_n;
  //
  const FLOAT_PHYS C = mtrl_matc[0];
  RESTRICT Phys::vals elem_diag(Nc);
  FLOAT_PHYS G[Ne];
  for(uint ie=0;ie<En;ie++){
    for(uint ip=0;ip<intp_n;ip++){
      uint ig=ip*Ne;
      for(uint i=0;i<Ne;i++){ G[i]=0.0; }
      for(uint k=0;k<Nc;k++){
      for(uint i=0;i<Dm;i++){
      for(uint j=0;j<Dm;j++){
        G[Nc* i+k] += E->elip_jacs[Nj*ie+Dm* j+i] * E->intp_shpg[ig+Dm* k+j];
      } } }
      FLOAT_PHYS Cdw = C * E->elip_jacs[Nj*ie+9] * E->gaus_weig[ip];
      for(uint i=0; i<Nc; i++){
      for(uint k=0; k<3 ; k++){
        elem_diag[i]+= G[Nc*k + i] * G[Nc*k + i] * Cdw;
      } }
    }//end intp loop
    for (uint i=0; i<Nc; i++){
        part_d[E->elem_conn[Nc*ie+i]] += elem_diag[ i ];
      }
    elem_diag=0.0;
  }
  return 0;
}
#if 0
int ThermIso3D::ReadPartFMR( const char* fname, bool is_bin ){
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
    //FIXME This parsing requires properties in a specific order
    auto tprop = mtrl_prop; auto tsz=tprop.size();
    if(fmrstring=="$Elastic"){// Elastic Constants
      int s=0; fmrfile >> s;
      mtrl_prop.resize(tsz+s);
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
    if(fmrstring=="$ThermalExpansion"){// Thermal expansion
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s + tprop.size());
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
    if(fmrstring=="$ThermalConductivity"){// Thermal conductivity
      int s=0; fmrfile >> s;
      mtrl_prop.resize(s + tprop.size());
      mtrl_prop[std::slice(tsz,tsz+s,1)] = tprop;
      for(int i=0; i<s; i++){ fmrfile >> mtrl_prop[i+tprop.size()]; }
    }
  }
  return 0;
}
int ThermIso3D::SavePartFMR( const char* fname, bool is_bin ){
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
#endif