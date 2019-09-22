#ifndef INCLUDED_PHYS_H
#define INCLUDED_PHYS_H
#include <iostream>

class Phys{
public:
  typedef std::valarray<FLOAT_PHYS> vals;
  INT_DIM node_d;// Degrees of freedom per node://WAS ndof_n
  // 1 for thermal, 2 for elastic 2D, 3 for elastic 3D, 4 for thermoelastic 3D
#if 0
  // The followig are stored interleaved in the system vectors
  INT_DIM ninp_d=3;// Inputs/node (defines size of sys_u,p?)
  //                  usually ndof_d + user-defined nodal field and state vars
  INT_DIM ndof_d=3;// Unknowns/node (defines size of sys_f?)
#endif
  //FIXME The followig will be stored in blocks, some in other arrays
  INT_DIM nvar_d=0;// Inputs/node: user-defined nodal state vars
  INT_DIM evar_d=0;// Inputs/element: user-defined elemental state vars
  INT_DIM gvar_d=0;// Inputs/gauss point: user-defined state vars
  //
  FLOAT_PHYS part_sum1=0.0;
  Phys::vals udof_magn={0.0,0.0,0.0,0.0};//1e-3,1e-3,1e-3,100.0};
  //FIXED Set from BCS
  //FIXME should be in Mesh* or Solv*
  //
  virtual int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&)=0;
  virtual int ElemLinear( Elem*,const INT_MESH,const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*)=0;
  virtual int ElemNonlinear( Elem*,const INT_MESH,const INT_MESH,
    FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*)=0;
  virtual int ElemJacobi( Elem*,FLOAT_SOLV* )=0;// Jacobi Preconditioner
  virtual int ElemRowSumAbs(Elem*, FLOAT_SOLV* )=0;// Row Norm Preconditioner
  virtual int ElemStrain(Elem*, FLOAT_SOLV* )=0;// Applied Element Strain Preconditioner
  virtual int ElemLinear( Elem* )=0;//FIXME OLD
  virtual int ElemJacobi( Elem* )=0;//FIXME old
  virtual int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*)=0;
  //
  virtual inline int MtrlProp2MatC( )=0;//why does this inline?
  virtual Phys::vals MtrlLinear(//FIXME Not used for 3D yet
    const RESTRICT Phys::vals &strain )=0;
  //
  int ScatterNode2Elem( Elem*,
    const RESTRICT Solv::vals & node_v,
          RESTRICT Solv::vals & elem_v );
  int GatherElem2Node( Elem*,
    const RESTRICT Solv::vals & elem_v,
          RESTRICT Solv::vals & node_v );
  //
  virtual int Setup( Elem* E )=0;
  int JacRot( Elem* E );
  int JacT  ( Elem* E );
  //
  virtual int ElemStiff( Elem* )=0;
  //
#if 0
  virtual int SavePartFMR( const char* bname, bool is_bin )=0;//FIXME ASCII/Binary file format
  virtual int ReadPartFMR( const char* bname, bool is_bin )=0;
#else
  int SavePartFMR( const char* bname, bool is_bin );//FIXME ASCII/Binary file format
  int ReadPartFMR( const char* bname, bool is_bin );
#endif
  //
  int tens_flop=0, tens_band=0;
  int stif_flop=0, stif_band=0;
  //
  Phys::vals mtrl_prop;// Conventional Material Properties
  // (Young's, Poisson's, etc.)
  Phys::vals mtrl_dirs;// Orientation [x,z,x]
  //
  Phys::vals mtrl_matc;// Unique D-matrix values? FIXME Change to mtrl_coef?
  Phys::vals mtrl_rotc;
  //
  //FIXME Replace these with enumerator keys for mtrl_prop or mtrl_matc?
  Phys::vals elas_prop;
  Phys::vals ther_expa;//FIXME Hacked thermal constants into these
  Phys::vals ther_cond;
  Phys::vals plas_prop;//FIXME Hacked plastiicity properties into this
  //
  Phys::vals elem_vars;// Element state variables
  //
  Phys::vals elem_inout;// Elemental nodal value workspace (serial)
  // Fill w/ Phys::ScatterNode2Elem(...),ElemLinear(Elem*),ElemJacobi(),...
  Phys::vals elem_in, elem_out;// Double-buffer for parallel
  // The next is for comparison to traditional EBE
  Phys::vals elem_stiff;// Fill w/ Phys::ScatterStiff(...)
  //size_t elem_linear_flop=0;
protected:
  Phys( Phys::vals p ) : mtrl_prop(p){};
  Phys( Phys::vals p, Phys::vals d ) : mtrl_prop(p),mtrl_dirs(d){};
  //constructor computes material vals
  inline Phys::vals Tens2VoigtEng(const RESTRICT Phys::vals&);
  inline Phys::vals Tens3VoigtEng(const RESTRICT Phys::vals&);
  inline Phys::vals Tens2VoigtEng(const FLOAT_PHYS H[4]);
  inline Phys::vals Tens3VoigtEng(const FLOAT_PHYS H[9]);
private:
};
// Inline Functions =======================================
//
inline Phys::vals Phys::Tens3VoigtEng(const RESTRICT Phys::vals &H){
  return(Phys::vals { H[0],H[4],H[8], H[1]+H[3],H[5]+H[7],H[2]+H[6] });
  // exx,eyy,ezz, exy,eyz,exz
};
inline Phys::vals Phys::Tens3VoigtEng(const FLOAT_PHYS H[9]){
  return(Phys::vals { H[0],H[4],H[8], H[1]+H[3],H[5]+H[7],H[2]+H[6] });};
  // exx,eyy,ezz, exy,eyz,exz
inline Phys::vals Phys::Tens2VoigtEng(const RESTRICT Phys::vals &H){
  return(Phys::vals { H[0],H[3], H[1]+H[2] });};
inline Phys::vals Phys::Tens2VoigtEng(const FLOAT_PHYS H[4]){
  return(Phys::vals { H[0],H[3], H[1]+H[2] });
};
// Physics Kernels: ---------------------------------------
class ElastIso2D final: public Phys{
public: ElastIso2D(FLOAT_PHYS young, FLOAT_PHYS poiss, FLOAT_PHYS thick) :
  Phys((Phys::vals){young,poiss,thick}){// Constructor
    this->node_d      = 2;
    //this->elem_flop = FIXME;
    ElastIso2D::MtrlProp2MatC();
  };
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*,const INT_MESH,const INT_MESH,
    FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;// Used for testing traditional EBE
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{//why does this inline?
    const FLOAT_PHYS E=mtrl_prop[0];
    const FLOAT_PHYS n=mtrl_prop[1];
    const FLOAT_PHYS t=mtrl_prop[2];
    const FLOAT_PHYS d=E/(1.0-n*n)*t;
    mtrl_matc.resize(3); mtrl_matc={ d, n*d, (1.0-n)*d*0.5 };//*0.5 eng. strain
    return 0;
  };
  Phys::vals MtrlLinear(//FIXME Doesn't inline
    const RESTRICT Phys::vals &strain_tensor)final{//FIXME Plane Stress
    const Phys::vals e=Tens2VoigtEng(strain_tensor);
    return( Phys::vals {
      mtrl_matc[0]*e[0] +mtrl_matc[1]*e[1] ,
      mtrl_matc[1]*e[0] +mtrl_matc[0]*e[1] ,
      mtrl_matc[2]*e[2] });
    };
protected:
private:
};
class ElastIso3D final: public Phys{
public: ElastIso3D(FLOAT_PHYS young, FLOAT_PHYS poiss ) :
  Phys((Phys::vals){young,poiss}){
    this->node_d = 3;
    //this->elem_flop = 225;//FIXME Tensor eval for linear tet
    // calc stiff_flop from (node_d*E->elem_node_n)*(node_d*E->elem_node_n-1.0)
    ElastIso3D::MtrlProp2MatC();
  };
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{//why does this inline?
    const FLOAT_PHYS E =mtrl_prop[0];
    const FLOAT_PHYS nu=mtrl_prop[1];
    const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
    //const FLOAT_PHYS C11=(1.0-n)*d;
    //const FLOAT_PHYS C12=n*d ;
    //const FLOAT_PHYS C44=(1.0-2.0*n)*d*0.5;
    ////const Phys::vals C={C11,C12,C44};
    //return( Phys::vals {C11,C12,C44} );
    mtrl_matc.resize(3); mtrl_matc={ (1.0-nu)*d,nu*d,(1.0-2.0*nu)*d*0.5};
    return 0;
  }
  Phys::vals MtrlLinear( const RESTRICT Phys::vals &e)final{
    //FIXME Doesn't inline
    //const Phys::vals e=Tens3VoigtEng(strain_tensor);
    RESTRICT Phys::vals s(0.0,6);
    s[0]= mtrl_matc[0]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[1]*e[8];
    s[1]= mtrl_matc[1]*e[0] +mtrl_matc[0]*e[4] +mtrl_matc[1]*e[8];
    s[2]= mtrl_matc[1]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[0]*e[8];
    // Fused multiply-add probably better
    s[3]= mtrl_matc[2]*e[1] +mtrl_matc[2]*e[3];
    s[4]= mtrl_matc[2]*e[5] +mtrl_matc[2]*e[7];
    s[5]= mtrl_matc[2]*e[2] +mtrl_matc[2]*e[6];
    //FIXME Tensor form: http://solidmechanics.org/text/Chapter3_2/Chapter3_2.htm
    return s;
  }
protected:
private:
};
class ElastOrtho3D final: public Phys{
public:
  ElastOrtho3D(// Isotropic Material Constructor
    FLOAT_PHYS young, FLOAT_PHYS poiss ) :
    Phys((Phys::vals){young,poiss},
         (Phys::vals){0.0,0.0,0.0} )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Isotropic Material Constructor (Rotated)
    FLOAT_PHYS r1z  , FLOAT_PHYS r2x  , FLOAT_PHYS r3z,
    FLOAT_PHYS young, FLOAT_PHYS poiss ) :
    Phys((Phys::vals){ young, poiss },
         (Phys::vals){r1z,r2x,r3z} )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Cubic Material Constructor
    FLOAT_PHYS r1z  , FLOAT_PHYS r2x  , FLOAT_PHYS r3z,
    FLOAT_PHYS young, FLOAT_PHYS poiss, FLOAT_PHYS shear ) :
    Phys((Phys::vals){ young, poiss, shear },
         (Phys::vals){r1z,r2x,r3z} )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Transversely Isotropic Material Constructor
    FLOAT_PHYS r1z, FLOAT_PHYS r2x, FLOAT_PHYS r3z,
    FLOAT_PHYS C11, FLOAT_PHYS C33,
    FLOAT_PHYS C12, FLOAT_PHYS C13, FLOAT_PHYS C44 ) :
    Phys((Phys::vals){C11,C33, C12,C13, C44 },
         (Phys::vals){r1z,r2x,r3z} )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Orthotropic Material Constructor
    FLOAT_PHYS r1z, FLOAT_PHYS r2x, FLOAT_PHYS r3z,
    FLOAT_PHYS C11, FLOAT_PHYS C22, FLOAT_PHYS C33,
    FLOAT_PHYS C12, FLOAT_PHYS C23, FLOAT_PHYS C13,
    FLOAT_PHYS C44, FLOAT_PHYS C55, FLOAT_PHYS C66 ) :
    Phys((Phys::vals){C11,C22,C33, C12,C23,C13, C44,C55,C66 },
         (Phys::vals){r1z,r2x,r3z} )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Orthotropic Material Constructor
    Phys::vals prop, Phys::vals dirs ) :
    Phys( prop,dirs )
     { node_d = 3; ElastOrtho3D::MtrlProp2MatC(); };
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Phys::vals&,const RESTRICT Phys::vals&)
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{
    const FLOAT_PHYS z1=mtrl_dirs[0];// Rotation about z (radians)
    const FLOAT_PHYS x2=mtrl_dirs[1];// Rotation about x (radians)
    const FLOAT_PHYS z3=mtrl_dirs[2];// Rotation about z (radians)
    //mtrl_rotc.resize(6);// Rotation Matrix Components
    //mtrl_rotc={cos(z1),sin(z1),cos(x2),sin(x2),cos(z3),sin(z3)};
    mtrl_rotc.resize(9);// Rotation Matrix
    Phys::vals Z1={cos(z1),sin(z1),0.0, -sin(z1),cos(z1),0.0, 0.0,0.0,1.0};
    Phys::vals X2={1.0,0.0,0.0, 0.0,cos(x2),sin(x2), 0.0,-sin(x2),cos(x2)};
    Phys::vals Z3={cos(z3),sin(z3),0, -sin(z3),cos(z3),0.0, 0.0,0.0,1.0};
    //mtrl_rotc = MatMul3x3xN(MatMul3x3xN(Z1,X2),Z3);
    for(int i=0;i<3;i++){
      for(int l=0;l<3;l++){ mtrl_rotc[3* i+l ]=0.0;
        for(int j=0;j<3;j++){
          for(int k=0;k<3;k++){
            mtrl_rotc[3* i+l ] += Z1[3* i+j ] * X2[3* j+k ] * Z3[3* k+l ];
    };};};};
    #if VERBOSITY_MAX>10
    printf("Material Tensor Rotation:");
    for(size_t i=0; i<mtrl_rotc.size(); i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",mtrl_rotc[i]);
    }; printf("\n");
   #endif
    mtrl_matc.resize(9);// Material Matrix Entries {C11,,,C12,,,C44,,} ;
    switch(mtrl_prop.size()){
      case(2):{// Isotropic
        const FLOAT_PHYS E =mtrl_prop[0];
        const FLOAT_PHYS nu=mtrl_prop[1];
        const FLOAT_PHYS G =E/(1.0+1.0*nu);
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
        mtrl_matc={(1.0-nu)*d,(1.0-nu)*d,(1.0-nu)*d, nu*d,nu*d,nu*d,
          0.5*G,0.5*G,0.5*G};//*0.5};
      break;};
      case(3):{// Cubic
        const FLOAT_PHYS E =mtrl_prop[0];
        const FLOAT_PHYS nu=mtrl_prop[1];
        const FLOAT_PHYS G =mtrl_prop[2];
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
        mtrl_matc={(1.0-nu)*d,(1.0-nu)*d,(1.0-nu)*d, nu*d,nu*d,nu*d,
          0.5*G,0.5*G,0.5*G};//*0.5};
      break;};
      case(5):{//Transversely Isotropic
        const FLOAT_PHYS Ex  = mtrl_prop[0];// Ey=Ex=Ep
        const FLOAT_PHYS Ez  = mtrl_prop[1];
        const FLOAT_PHYS nxy = mtrl_prop[2];// nu_p
        const FLOAT_PHYS nxz = mtrl_prop[3];// nyz=nxz
        const FLOAT_PHYS Gxz = mtrl_prop[4];// Gyz=Gxz
        const FLOAT_PHYS nzx = nxy*Ez/Ex;
        const FLOAT_PHYS d   = Ex*Ex*Ez/( (1.0+nxy)*(1.0-nxy-2.0*nxz*nxz) );
        const FLOAT_PHYS C11 = d* (1.0-nxz*nzx)/(Ex*Ez);//mtrl_prop[0];
        const FLOAT_PHYS C33 = d* (1.0-nxy*nxy)/(Ex*Ex);//mtrl_prop[1];
        const FLOAT_PHYS C12 = d* (nxy+nxz*nzx)/(Ex*Ex);//mtrl_prop[2];
        const FLOAT_PHYS C13 = d* (nxz+nxy*nxz)/(Ex*Ex);//mtrl_prop[3];
        const FLOAT_PHYS C44 = Gxz;//mtrl_prop[4];
        mtrl_matc={C11,C11,C33, C12,C13,C13, C44,C44,(C11-C12)*0.5};
      break;};
      case(9):{// Orthotropic
        //FIXME need conventional material properties
        //const FLOAT_PHYS C11 =mtrl_prop[0];
        //const FLOAT_PHYS C22 =mtrl_prop[1];
        //const FLOAT_PHYS C33 =mtrl_prop[2];
        //const FLOAT_PHYS C12 =mtrl_prop[3];
        //const FLOAT_PHYS C23 =mtrl_prop[4];
        //const FLOAT_PHYS C13 =mtrl_prop[5];
        //const FLOAT_PHYS C44 =mtrl_prop[6];
        //const FLOAT_PHYS C55 =mtrl_prop[7];
        //const FLOAT_PHYS C66 =mtrl_prop[8];
        mtrl_matc=mtrl_prop;
      break;};
    };// mtrl_prop cubic,transverse,ortho
    return 0;
  };
  //FLOAT_PHYS* MtrlLinear(FLOAT_PHYS e[9])final{
  Phys::vals MtrlLinear(const RESTRICT Phys::vals &e)final{
    RESTRICT Phys::vals S(9);//FIXME
    //RESTRICT Phys::vals s(6);
    //
    //FLOAT_PHYS Z1[4]={mtrl_rotc[0],mtrl_rotc[1],-mtrl_rotc[1],mtrl_rotc[0]};
    //FLOAT_PHYS X2[4]={mtrl_rotc[2],mtrl_rotc[3],-mtrl_rotc[3],mtrl_rotc[2]};
    //FLOAT_PHYS Z3[4]={mtrl_rotc[4],mtrl_rotc[5],-mtrl_rotc[5],mtrl_rotc[4]};
    //FLOAT_PHYS t1[4],t2[4];
    //FLOAT_PHYS E[9],S[9];//,V[6];
    //std::copy( &e[0],
    //           &e[8], E );
    #if VERBOSITY_MAX>10
    printf("Tensor Rotation:");
    for(size_t i=0; i<mtrl_rotc.size(); i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",mtrl_rotc[i]);
    }; printf("\n");
    //
    printf("Strain (Global):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",e[i]);
    }; printf("\n");
    #endif
    //
    //const RESTRICT Phys::vals E = MatMul3x3xN( mtrl_rotc,e );
    RESTRICT Phys::vals E(9);
    for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        for(int k=0;k<3;k++){
          E[3* i+k ] += mtrl_rotc[3* i+j ] * e[3* j+k ];
    };};};
    //
    #if VERBOSITY_MAX>10
    printf("Strain (Material):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",E[i]);
    }; printf("\n");
    #endif
    S[0]= mtrl_matc[0]*E[0] +mtrl_matc[1]*E[4] +mtrl_matc[1]*E[8];
    S[4]= mtrl_matc[1]*E[0] +mtrl_matc[0]*E[4] +mtrl_matc[1]*E[8];
    S[8]= mtrl_matc[1]*E[0] +mtrl_matc[1]*E[4] +mtrl_matc[0]*E[8];
    // Stress tensor
    S[1]= mtrl_matc[2]*(E[1]+E[3]); S[3]=S[1];
    S[5]= mtrl_matc[2]*(E[5]+E[7]); S[7]=S[5];
    S[2]= mtrl_matc[2]*(E[2]+E[6]); S[6]=S[2];
    //
    #if VERBOSITY_MAX>10
    printf("Stress (Material):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",S[i]);
    }; printf("\n");
    #endif
    //const RESTRICT Phys::vals s = MatMul3x3xNT( mtrl_rotc,S );//FIXME
    RESTRICT Phys::vals s(9);
    for(int i=0;i<3;i++){
      for(int k=0;k<3;k++){
        for(int j=0;j<3;j++){
          s[3* i+k ] += mtrl_rotc[3* i+j ] * S[3* k+j ];
    };};};
    //
    #if VERBOSITY_MAX>10
    printf("Stress (Global):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",S[i]);
    }; printf("\n");
    #endif
    //
    return {s[0],s[4],s[8], s[1],s[5],s[2]};
    //return s;
  };
protected:
private:
};
class ThermElastOrtho3D final: public Phys{
public:
  ThermElastOrtho3D(// Orthotropic Material Constructor
    Phys::vals prop, Phys::vals dirs, Phys::vals expa, Phys::vals cond ) :
    Phys( prop,dirs ){
      node_d = 4;
      ther_expa.resize(expa.size()); ther_expa=expa;
      ther_cond.resize(cond.size()); ther_cond=cond;
      ThermElastOrtho3D::MtrlProp2MatC(); 
    }
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Phys::vals&,const RESTRICT Phys::vals&)
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{
    // First, set the elastic-only part
    auto Y = new ElastOrtho3D(this->mtrl_prop,this->mtrl_dirs);
    Y->MtrlProp2MatC();
    auto n = Y->mtrl_matc.size();
    this->mtrl_matc.resize(n+9);
    for(uint i=0; i<n; i++){ this->mtrl_matc[i]=Y->mtrl_matc[i]; }
    //delete Y;
    // now set the thermal part
    if(this->ther_expa.size()>0){
      uint N=9;
      for(uint i=N;i<(N+3);i++){ mtrl_matc[i]=ther_expa[0]; }
      for(uint i=0;i<ther_expa.size();i++){ mtrl_matc[N+i]=ther_expa[i]; }
    }
    if(this->ther_cond.size()>0){
      uint N=12;
      for(uint i=N;i<(N+3);i++){ mtrl_matc[i]=ther_cond[0]; }
      for(uint i=0;i<ther_cond.size();i++){ mtrl_matc[N+i]=ther_cond[i]; }
#if 0
      //FIXME Scaling applied here for conditioning the system
      auto s = mtrl_matc[0] / mtrl_matc[N];
      for(uint i=0;i<3;i++){ mtrl_matc[N+i] *= s; }
      //FIXME Need to store this scaling factor in Phys* to adjust reactions
#endif
    }
    for(uint i=0;i<3;i++){
      // gamma = alpha * E/(1-2*nu), thermoelastic effect
      //FIXME may be 1.0/this
      mtrl_matc[15+i] = 1.0/(mtrl_matc[i] * mtrl_matc[9+i]); }
      //FIXME should read from .fmr file first
    //FIXME
    return 0;
  }
  Phys::vals MtrlLinear(const RESTRICT Phys::vals &e)final{ return e; }// dummy
protected:
private:
};
class ElastPlastJ2Iso3D final: public Phys{
public: ElastPlastJ2Iso3D(FLOAT_PHYS young, FLOAT_PHYS poiss ) :
  Phys((Phys::vals){young,poiss}){
    this->node_d = 3;
    //this->elem_flop = 225;//FIXME Tensor eval for linear tet
    // calc stiff_flop from (node_d*E->elem_node_n)*(node_d*E->elem_node_n-1.0)
    ElastPlastJ2Iso3D::MtrlProp2MatC();
  };
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*,const INT_MESH,
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{//why does this inline?
    const FLOAT_PHYS E =mtrl_prop[0];
    const FLOAT_PHYS nu=mtrl_prop[1];
    const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
    //const FLOAT_PHYS C11=(1.0-n)*d;
    //const FLOAT_PHYS C12=n*d ;
    //const FLOAT_PHYS C44=(1.0-2.0*n)*d*0.5;
    ////const Phys::vals C={C11,C12,C44};
    //return( Phys::vals {C11,C12,C44} );
    mtrl_matc.resize(3); mtrl_matc={ (1.0-nu)*d,nu*d,(1.0-2.0*nu)*d*0.5};
    return 0;
  }
  Phys::vals MtrlLinear( const RESTRICT Phys::vals &e)final{
    //FIXME Doesn't inline
    //const Phys::vals e=Tens3VoigtEng(strain_tensor);
    RESTRICT Phys::vals s(0.0,6);
    s[0]= mtrl_matc[0]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[1]*e[8];
    s[1]= mtrl_matc[1]*e[0] +mtrl_matc[0]*e[4] +mtrl_matc[1]*e[8];
    s[2]= mtrl_matc[1]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[0]*e[8];
    // Fused multiply-add probably better
    s[3]= mtrl_matc[2]*e[1] +mtrl_matc[2]*e[3];
    s[4]= mtrl_matc[2]*e[5] +mtrl_matc[2]*e[7];
    s[5]= mtrl_matc[2]*e[2] +mtrl_matc[2]*e[6];
    //FIXME Tensor form: http://solidmechanics.org/text/Chapter3_2/Chapter3_2.htm
    return s;
  }
protected:
private:
};



#endif
