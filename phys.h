#ifndef INCLUDED_PHYS_H
#define INCLUDED_PHYS_H
#include "phys-inline.cc"
#include <iostream>

class Phys{
public:
  typedef std::valarray<FLOAT_PHYS> vals;
  enum Eval {
    EBE_TTE=0, EBE_LMS=1
  };
  INT_DIM node_d;// Degrees of freedom per node://WAS ndof_n
  // 1 for thermal, 2 for elastic 2D, 3 for elastic 3D, 4 for thermoelastic 3D
#if 0
  // The following are stored interleaved in the system vectors
  INT_DIM ninp_d=3;// Inputs/node (defines size of part_u,p?)
  //                  usually ndof_d + user-defined nodal field and state vars
  INT_DIM ndof_d=3;// Unknowns/node (defines size of part_f?)
#endif
  //FIXME The following will be stored in blocks.
  INT_DIM nvar_d=0;// Inputs/node: user-defined nodal state vars
  INT_DIM evar_d=0;// Inputs/element: user-defined elemental state vars
  INT_DIM gvar_d=0;// Inputs/gauss point: user-defined state vars
  //
  int solv_cond=Solv::COND_JACO;
  int phys_eval=Phys::EBE_TTE;
  //
  FLOAT_PHYS part_sum1=0.0;
  //
  Phys::vals udof_magn={0.0,0.0,0.0,0.0};//1e-3,1e-3,1e-3,100.0};
  //FIXED Set from BCS
  //FIXME should be in Mesh* or Solv*
  //
  virtual int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&)=0;
  virtual int ElemLinear( Elem*,const INT_MESH,const INT_MESH,
    FLOAT_SOLV*,const FLOAT_SOLV* )=0;
  virtual int ElemNonlinear( Elem*,const INT_MESH,const INT_MESH,
    FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool )=0;
  virtual int ElemJacobi( Elem*, FLOAT_SOLV* )=0;// Jacobi Preconditioner
  virtual int ElemJacobi( Elem*, FLOAT_SOLV*, const FLOAT_SOLV* )=0;// Nonlinear Jacobi
  virtual int ElemJacNode( Elem*, FLOAT_SOLV* )=0;// Node block Jacobi Preconditioner
  virtual int ElemRowSumAbs( Elem*, FLOAT_SOLV* )=0;// Row Norm Preconditioner
  virtual int ElemStrain( Elem*, FLOAT_SOLV* )=0;// Applied Strain Preconditioner
  virtual int ElemLinear( Elem* )=0;//FIXME OLD
  virtual int ElemJacobi( Elem* )=0;//FIXME OLD
  virtual int ElemStrainStress( std::ostream&, Elem*, FLOAT_SOLV* )=0;
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
  int IniRot();
  int JacRot( Elem* E );
  int JacT  ( Elem* E );
#if 0
  int MtrRot();// Not yet needed
#endif
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
  Phys::vals mtrl_dmat;// Conventional 6x6 DMAT, full storage, padded to 8 cols
  //
  //FIXME Replace these with enumerator keys for mtrl_prop or mtrl_matc?
  Phys::vals elas_prop;
  Phys::vals ther_expa;//FIXME Hacked thermal constants into these
  Phys::vals ther_cond;
  Phys::vals plas_prop;//FIXME Hacked plasticity properties into this
  //
  Phys::vals elem_vars;// Element state variables
  Phys::vals elgp_vars;// Element integration point state variables
  //
  Phys::vals elem_inout;// Elemental nodal value workspace (serial)
  // Fill w/ Phys::ScatterNode2Elem(...),ElemLinear(Elem*),ElemJacobi(),...
  Phys::vals elem_in, elem_out;// Double-buffer for parallel
  // The next is for comparison to traditional EBE
  Phys::vals elem_stiff;// Fill w/ Phys::ScatterStiff(...)
  //size_t elem_linear_flop=0;
protected:
  Phys( Phys::vals p ) : mtrl_prop(p){};
  Phys( Phys::vals p, Phys::vals d ) : mtrl_prop(p),mtrl_dirs(d){
    elas_prop.resize(mtrl_prop.size());
    elas_prop=mtrl_prop;
  };
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
    FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*,bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
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
    } } } }
    #if VERBOSITY_MAX>10
    printf("Material Tensor Rotation:");
    for(size_t i=0; i<mtrl_rotc.size(); i++){
      if(!(i%3)){printf("\n");};
      printf("%10.3e ",mtrl_rotc[i]);
    } printf("\n");
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
      break;}
      case(3):{// Cubic
        const FLOAT_PHYS E =mtrl_prop[0];
        const FLOAT_PHYS nu=mtrl_prop[1];
        const FLOAT_PHYS G =mtrl_prop[2];
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
        mtrl_matc={(1.0-nu)*d,(1.0-nu)*d,(1.0-nu)*d, nu*d,nu*d,nu*d,
          0.5*G,0.5*G,0.5*G};//*0.5};
      break;}
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
      break;}
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
      break;}
    }// mtrl_prop cubic,transverse,ortho
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
      if(!(i%3)){printf("\n");}
      printf("%10.3e ",mtrl_rotc[i]);
    } printf("\n");
    //
    printf("Strain (Global):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");}
      printf("%10.3e ",e[i]);
    } printf("\n");
    #endif
    //
    //const RESTRICT Phys::vals E = MatMul3x3xN( mtrl_rotc,e );
    RESTRICT Phys::vals E(9);
    for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
        for(int k=0;k<3;k++){
          E[3* i+k ] += mtrl_rotc[3* i+j ] * e[3* j+k ];
    } } }
    //
    #if VERBOSITY_MAX>10
    printf("Strain (Material):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");}
      printf("%10.3e ",E[i]);
    } printf("\n");
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
    } printf("\n");
    #endif
    //const RESTRICT Phys::vals s = MatMul3x3xNT( mtrl_rotc,S );//FIXME
    RESTRICT Phys::vals s(9);
    for(int i=0;i<3;i++){
      for(int k=0;k<3;k++){
        for(int j=0;j<3;j++){
          s[3* i+k ] += mtrl_rotc[3* i+j ] * S[3* k+j ];
    } } }
    //
    #if VERBOSITY_MAX>10
    printf("Stress (Global):");
    for(size_t i=0; i<9; i++){
      if(!(i%3)){printf("\n");}
      printf("%10.3e ",S[i]);
    } printf("\n");
    #endif
    //
    return { s[0],s[4],s[8], s[1],s[5],s[2] };
    //return s;
  };
protected:
private:
};
class ElastDmv3D final: public Phys{
public: ElastDmv3D(Phys::vals prop, Phys::vals dirs ) :
  Phys(prop,dirs){
    this->node_d = 3;
    if(prop.size()<21){
      mtrl_prop.resize(prop.size()); mtrl_prop=prop;
    }else{
      mtrl_matc.resize(prop.size()); mtrl_matc=prop;
    }
    mtrl_dmat.resize(48);
    ElastDmv3D::MtrlProp2MatC();
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{//why does this inline?
    switch(mtrl_prop.size()){
      case( 2):{// Build isotropic DMAT.
        const FLOAT_PHYS E =mtrl_prop[0];
        const FLOAT_PHYS nu=mtrl_prop[1];
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
        const FLOAT_PHYS C0 =(1.0-nu)*d;
        const FLOAT_PHYS C1 =nu*d;
        const FLOAT_PHYS C2 =(1.0-2.0*nu)*d*0.5;
#ifdef HAS_AVX
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,0.0,C2, 0.0,0.0 };
#else
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0,
          0.0,0.0,0.0,0.0,0.0,C2 };
#endif
        mtrl_dmat=D;
        mtrl_matc.resize(3);
        mtrl_matc[0]=C0; mtrl_matc[1]=C1; mtrl_matc[2]=C2;
        break;}
      case( 3):{// Build cubic DMAT.
        const FLOAT_PHYS E =mtrl_prop[0];
        const FLOAT_PHYS nu=mtrl_prop[1];
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));//FIXME Check these
        const FLOAT_PHYS C0 =(1.0-nu)*d;
        const FLOAT_PHYS C1 =nu*d;
        const FLOAT_PHYS C2 =mtrl_prop[2];
#ifdef HAS_AVX
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,0.0,C2, 0.0,0.0 };
#else
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0,
          0.0,0.0,0.0,0.0,0.0,C2 };
#endif
        mtrl_dmat=D;
        mtrl_matc.resize(3);
        mtrl_matc[0]=C0; mtrl_matc[1]=C1; mtrl_matc[2]=C2;
        break;}
    }
    switch(mtrl_matc.size()){//FIXME Should Lame constants?
      case( 2):{// Build isotropic DMAT.
        mtrl_prop.resize(2);
        mtrl_prop=mtrl_matc;
        const FLOAT_PHYS E =mtrl_matc[0];
        const FLOAT_PHYS nu=mtrl_matc[1];
        const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
        const FLOAT_PHYS C0 =(1.0-nu)*d;
        const FLOAT_PHYS C1 =nu*d;
        const FLOAT_PHYS C2 =(1.0-2.0*nu)*d*0.5;
#ifdef HAS_AVX
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0, 0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,0.0,C2, 0.0,0.0 };
#else
        const Phys::vals D={
          C0,C1,C1,0.0,0.0,0.0,
          C1,C0,C1,0.0,0.0,0.0,
          C1,C1,C0,0.0,0.0,0.0,
          0.0,0.0,0.0,C2,0.0,0.0,
          0.0,0.0,0.0,0.0,C2,0.0,
          0.0,0.0,0.0,0.0,0.0,C2 };
#endif
        mtrl_dmat=D;
        mtrl_matc.resize(3);
        mtrl_matc[0]=C0; mtrl_matc[1]=C1; mtrl_matc[2]=C2;
        break;}
      case( 3):{// Build cubic DMAT.
        mtrl_prop.resize(3);
        mtrl_prop=mtrl_matc;
        const FLOAT_PHYS* RESTRICT C =& mtrl_matc[0];
#ifdef HAS_AVX
        const Phys::vals D={
          C[0],C[1],C[1],0.0,0.0,0.0, 0.0,0.0,
          C[1],C[0],C[1],0.0,0.0,0.0, 0.0,0.0,
          C[1],C[1],C[0],0.0,0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,C[2],0.0,0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,C[2],0.0, 0.0,0.0,
          0.0,0.0,0.0,0.0,0.0,C[2], 0.0,0.0 };
#else
        const Phys::vals D={
          C[0],C[1],C[1],0.0,0.0,0.0,
          C[1],C[0],C[1],0.0,0.0,0.0,
          C[1],C[1],C[0],0.0,0.0,0.0,
          0.0,0.0,0.0,C[2],0.0,0.0,
          0.0,0.0,0.0,0.0,C[2],0.0,
          0.0,0.0,0.0,0.0,0.0,C[2] };
#endif
        mtrl_dmat=D;
        break;}
      case(21):{// 21 values for symmetric 6x6.
        const FLOAT_PHYS* RESTRICT C =& mtrl_matc[0];
        mtrl_prop.resize(3);// Estimate isotropic properties
        const FLOAT_PHYS C0=(C[ 0]+C[ 6]+C[11])/3.0;
        const FLOAT_PHYS C1=(C[ 1]+C[ 2]+C[ 7])/3.0;
        const FLOAT_PHYS C2=(C[15]+C[18]+C[20])/3.0;
        const FLOAT_PHYS E =(C0-C1)*(2.0*C1+C0)/(C0+C1);
        const FLOAT_PHYS nu= C1/(C0+C1);
        mtrl_prop[0]=E; mtrl_prop[1]=nu; mtrl_prop[2]=C2;
#ifdef HAS_AVX
        const Phys::vals D={
          C[ 0],C[ 1],C[ 2],C[ 3],C[ 4],C[ 5], 0.0,0.0,
          C[ 1],C[ 6],C[ 7],C[ 8],C[ 9],C[10], 0.0,0.0,
          C[ 2],C[ 7],C[11],C[12],C[13],C[14], 0.0,0.0,
          C[ 3],C[ 8],C[12],C[15],C[16],C[17], 0.0,0.0,
          C[ 4],C[ 9],C[13],C[16],C[18],C[19], 0.0,0.0,
          C[ 5],C[10],C[14],C[17],C[19],C[20], 0.0,0.0 };
#else
        const Phys::vals D={
          C[ 0],C[ 1],C[ 2],C[ 3],C[ 4],C[ 5],
          C[ 1],C[ 6],C[ 7],C[ 8],C[ 9],C[10],
          C[ 2],C[ 7],C[11],C[12],C[13],C[14],
          C[ 3],C[ 8],C[12],C[15],C[16],C[17],
          C[ 4],C[ 9],C[13],C[16],C[18],C[19],
          C[ 5],C[10],C[14],C[17],C[19],C[20] };
#endif
        mtrl_dmat=D;
        mtrl_matc.resize(3);
        mtrl_matc[0]=C0; mtrl_matc[1]=C1; mtrl_matc[2]=C2;
        break;}
      case(36):{
        const FLOAT_PHYS* RESTRICT C =& mtrl_matc[0];
        mtrl_prop.resize(3);// Estimate isotropic properties
        const FLOAT_PHYS C0=(C[ 0]+C[ 7]+C[14])/3.0;
        const FLOAT_PHYS C1=(C[ 1]+C[ 2]+C[ 8])/3.0;
        const FLOAT_PHYS C2=(C[21]+C[28]+C[35])/3.0;
        const FLOAT_PHYS E =(C0-C1)*(2.0*C1+C0)/(C0+C1);
        const FLOAT_PHYS nu= C1/(C0+C1);
        mtrl_prop[0]=E; mtrl_prop[1]=nu; mtrl_prop[2]=C2;
#ifdef HAS_AVX
        const Phys::vals D={
          C[ 0],C[ 1],C[ 2],C[ 3],C[ 4],C[ 5], 0.0,0.0,
          C[ 6],C[ 7],C[ 8],C[ 9],C[10],C[11], 0.0,0.0,
          C[12],C[13],C[14],C[15],C[16],C[17], 0.0,0.0,
          C[18],C[19],C[20],C[21],C[22],C[23], 0.0,0.0,
          C[24],C[25],C[26],C[27],C[28],C[29], 0.0,0.0,
          C[30],C[31],C[32],C[33],C[34],C[35], 0.0,0.0 };
#else
        const Phys::vals D={
          C[ 0],C[ 1],C[ 2],C[ 3],C[ 4],C[ 5],
          C[ 6],C[ 7],C[ 8],C[ 9],C[10],C[11],
          C[12],C[13],C[14],C[15],C[16],C[17],
          C[18],C[19],C[20],C[21],C[22],C[23],
          C[24],C[25],C[26],C[27],C[28],C[29],
          C[30],C[31],C[32],C[33],C[34],C[35] };
#endif
        mtrl_dmat=D;
        mtrl_matc.resize(3);
        mtrl_matc[0]=C0; mtrl_matc[1]=C1; mtrl_matc[2]=C2;
        break;}
    }
    return 0;
  }
  Phys::vals MtrlLinear( const RESTRICT Phys::vals &e)final{
    return e;
  }
protected:
private:
};
class ThermIso3D final: public Phys{
public:
  ThermIso3D( Phys::vals cond ) :// Constructor
    Phys( cond ){
      node_d = 1;
      ther_cond.resize(cond.size()); ther_cond=cond;
      ThermIso3D::MtrlProp2MatC(); 
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{
    mtrl_matc.resize(1);
    if(this->ther_cond.size()>0){
      mtrl_matc[0]=ther_cond[0];
    }else{ mtrl_matc[0]=1.0; }
    return 0;
  }
  Phys::vals MtrlLinear(const RESTRICT Phys::vals &e)final{ return e; }// dummy
protected:
private:
};
#if 0
class ThermOrtho3D final: public Phys{
public:
  ThermIso3D( Phys::vals cond, Phys::vals dirs ) :// Constructor
    Phys( cond,dirs ){
      node_d = 1;
      ther_cond.resize(cond.size()); ther_cond=cond;
      ThermIso3D::MtrlProp2MatC(); 
    }
#if 0
  int SavePartFMR( const char* bname, bool is_bin ) final;
  int ReadPartFMR( const char* bname, bool is_bin ) final;
#endif
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Phys::vals&,const RESTRICT Phys::vals&)
  int BlocLinear( Elem*,RESTRICT Phys::vals&,const RESTRICT Phys::vals&) final;
  int ElemLinear( Elem*, const INT_MESH, const INT_MESH,
    FLOAT_SOLV*, const FLOAT_SOLV*) final;
  int ElemNonlinear( Elem*, const INT_MESH, const INT_MESH,
    FLOAT_SOLV*, const FLOAT_SOLV*, const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{
    mtrl_matc.resize(3);
    if(this->ther_cond.size()>0){ mtrl_matc[0]=ther_cond[0];
      }else{ mtrl_matc[0]=1.0 }
    if(this->ther_cond.size()>1){ mtrl_matc[1]=ther_cond[1];
      }else{ mtrl_matc[1]=mtrl_matc[0] }
    if(this->ther_cond.size()>2){ mtrl_matc[2]=ther_cond[2];
      }else{ mtrl_matc[2]=mtrl_matc[1] }
    this->mtrl_dmat.resize(9,0.0);
    if(this->mtrl_rotc.size()>8){// Apply rotation matrix.
      const FLOAT_PHYS* RESTRICT R =& this->mtrl_rotc[0];
      for(int i=0;i<3;i++){
      for(int j=0;j<3;j++){
      for(int k=0;k<3;k++){
      for(int l=0;l<3;l++){
      for(int m=0;m<3;m++){
        mtrl_dmat[3*i+m]+=R[3*j+i]*R[3*k+j]*mtrl_matc[k]*R[3*k+l]*R[3*l+m];
      } } } } }
    }else{// Set the diagonal (unrotated)
      this->mtrl_dmat[0]=mtrl_matc[0];
      this->mtrl_dmat[4]=mtrl_matc[1];
      this->mtrl_dmat[8]=mtrl_matc[2];
    }
    return 0;
  }
  Phys::vals MtrlLinear(const RESTRICT Phys::vals &e)final{ return e; }// dummy
protected:
private:
};
#endif
class ThermElastIso3D final: public Phys{
public:
  ThermElastIso3D(//FIXME Orthotropic Material Constructor
    Phys::vals prop, Phys::vals dirs, Phys::vals expa, Phys::vals cond ) :
    Phys( prop,dirs ){
      node_d = 4;
      ther_expa.resize(expa.size()); ther_expa=expa;
      ther_cond.resize(cond.size()); ther_cond=cond;
      ThermElastIso3D::MtrlProp2MatC(); 
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ElemStiff ( Elem* ) final;
  int ElemStrainStress(std::ostream&, Elem*, FLOAT_SOLV*) final;
  inline int MtrlProp2MatC()final{
    // First, set the elastic-only part
    auto Y = new ElastIso3D(this->mtrl_prop[0],this->mtrl_prop[1]);
    Y->MtrlProp2MatC();
    auto n = Y->mtrl_matc.size();
    this->mtrl_matc.resize(n+3);
    for(uint i=0; i<n; i++){ this->mtrl_matc[i]=Y->mtrl_matc[i]; }
    //delete Y;
    // now set the thermal part
    if(this->ther_expa.size()>0){ mtrl_matc[3]=ther_expa[0]; }
    if(this->ther_cond.size()>0){ mtrl_matc[4]=ther_cond[0]; }
#if 0
      //FIXME Scaling applied here for conditioning the system
      auto s = mtrl_matc[0] / mtrl_matc[N];
      for(uint i=0;i<1;i++){ mtrl_matc[N+i] *= s; }
      //FIXME Need to store this scaling factor in Phys* to adjust reactions
#endif
    // gamma = alpha * E/(1-2*nu), thermoelastic effect
    //FIXME may be 1.0/this
    mtrl_matc[5] = 1.0/(mtrl_matc[0] * mtrl_matc[3]);
    //FIXME should read from .fmr file first
    //FIXME
    return 0;
  }
  Phys::vals MtrlLinear(const RESTRICT Phys::vals &e)final{ return e; }// dummy
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
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
class ElastPlastKHIso3D final: public Phys{
public: ElastPlastKHIso3D(FLOAT_PHYS young, FLOAT_PHYS poiss ) :
  Phys((Phys::vals){young,poiss}){
    this->node_d = 3;
    //this->elem_flop = 225;//FIXME Tensor eval for linear tet
    // calc stiff_flop from (node_d*E->elem_node_n)*(node_d*E->elem_node_n-1.0)
    ElastPlastKHIso3D::MtrlProp2MatC();
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
    const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemJacobi( Elem*,FLOAT_SOLV*,const FLOAT_SOLV* ) final;
  int ElemJacNode( Elem*,FLOAT_SOLV* ) final;
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
