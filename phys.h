#ifndef INCLUDED_PHYS_H
#define INCLUDED_PHYS_H
#include <iostream>
#include <immintrin.h>

class Phys{
public:
  typedef std::valarray<FLOAT_PHYS> vals;
  INT_DIM node_d;// Degrees of freedom per node://WAS ndof_n
  // 1 for thermal, 2 for elastic 2D, 3 for elastic 3D, 4 for thermoelastic 3D
#if 0
  // The followig are stored interleaved in the system vectors
  INT_DIM ninp_d=3;// Inputs/node (defines size of part_u,p?)
  //                  usually ndof_d + user-defined nodal field and state vars
  INT_DIM ndof_d=3;// Unknowns/node (defines size of part_f?)
#endif
  //FIXME The followig will be stored in blocks, some in other arrays
  INT_DIM nvar_d=0;// Inputs/node: user-defined nodal state vars
  INT_DIM evar_d=0;// Inputs/element: user-defined elemental state vars
  INT_DIM gvar_d=0;// Inputs/gauss point: user-defined state vars
  //
  int solv_cond=Solv::COND_JACO;
  //
  FLOAT_PHYS part_sum1=0.0;
  //
  Phys::vals udof_magn={0.0,0.0,0.0,0.0};//1e-3,1e-3,1e-3,100.0};
  //FIXED Set from BCS
  //FIXME should be in Mesh* or Solv*
  //
  virtual int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&)=0;
  virtual int ElemLinear( Elem*,const INT_MESH,const INT_MESH,FLOAT_SOLV*,const FLOAT_SOLV*)=0;
  virtual int ElemNonlinear( Elem*,const INT_MESH,const INT_MESH,
    FLOAT_SOLV*,const FLOAT_SOLV*,const FLOAT_SOLV*, bool)=0;
  virtual int ElemJacobi( Elem*, FLOAT_SOLV* )=0;// Jacobi Preconditioner
  virtual int ElemJacobi( Elem*, FLOAT_SOLV*, const FLOAT_SOLV* )=0;// Nonlinear Jacobi
  virtual int ElemJacNode( Elem*, FLOAT_SOLV* )=0;// Jacobi Preconditioner
  //virtual int ElemJacobi( Elem*, FLOAT_SOLV*,const FLOAT_SOLV* )=0;// Nonlinear Jacobi
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
class ThermElastIso3D final: public Phys{
public:
  ThermElastIso3D(// Orthotropic Material Constructor
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
//FIXME These inline intrinsics functions should be in the class where used.
static inline void accumulate_f( __m256d* vf,
  const __m256d* a, const FLOAT_PHYS* G, const int Nc ){
  for(int i= 0; i< 4; i++){
    vf[i]= _mm256_add_pd(vf[i],
      _mm256_add_pd(_mm256_mul_pd(a[0],_mm256_set1_pd(G[4*i  ])),
        _mm256_add_pd(_mm256_mul_pd(a[1],_mm256_set1_pd(G[4*i+1])),
          _mm256_mul_pd(a[2],_mm256_set1_pd(G[4*i+2])))));
  }
  if(Nc>4){
    for(int i= 4; i<10; i++){
      vf[i]= _mm256_add_pd(vf[i],
        _mm256_add_pd(_mm256_mul_pd(a[0],_mm256_set1_pd(G[4*i  ])),
          _mm256_add_pd(_mm256_mul_pd(a[1],_mm256_set1_pd(G[4*i+1])),
            _mm256_mul_pd(a[2],_mm256_set1_pd(G[4*i+2])))));
    }
    if(Nc>10){
      for(int i=10; i<20; i++){
        vf[i]= _mm256_add_pd(vf[i],
          _mm256_add_pd(_mm256_mul_pd(a[0],_mm256_set1_pd(G[4*i  ])),
            _mm256_add_pd(_mm256_mul_pd(a[1],_mm256_set1_pd(G[4*i+1])),
              _mm256_mul_pd(a[2],_mm256_set1_pd(G[4*i+2])))));
      }
    }
  }
}
static inline void rotate_g_h(
  FLOAT_PHYS* G, __m256d* vH,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* R, const FLOAT_PHYS* u ){
  //FLOAT_PHYS* RESTRICT isp = &intp_shpg[ip*Ne];
  __m256d a036=_mm256_setzero_pd(), a147=_mm256_setzero_pd(),
    a258=_mm256_setzero_pd();
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    { const __m256d g0 =
      _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+0])),
      _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+1])),
                    _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+2]))));
      a036 = _mm256_add_pd(a036,_mm256_mul_pd(g0,_mm256_set1_pd( u[i+0])));
      a147 = _mm256_add_pd(a147,_mm256_mul_pd(g0,_mm256_set1_pd( u[i+1])));
      a258 = _mm256_add_pd(a258,_mm256_mul_pd(g0,_mm256_set1_pd( u[i+2])));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    }if((i+5)<Ne){
      const __m256d g1 =
        _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+3])),
        _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+4])),
                      _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+5]))));
      a036 = _mm256_add_pd(a036,_mm256_mul_pd(g1,_mm256_set1_pd( u[i+3])));
      a147 = _mm256_add_pd(a147,_mm256_mul_pd(g1,_mm256_set1_pd( u[i+4])));
      a258 = _mm256_add_pd(a258,_mm256_mul_pd(g1,_mm256_set1_pd( u[i+5])));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d g2 =
        _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+6])),
        _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+7])),
                      _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+8]))));
      a036 = _mm256_add_pd(a036,_mm256_mul_pd(g2,_mm256_set1_pd( u[i+6])));
      a147 = _mm256_add_pd(a147,_mm256_mul_pd(g2,_mm256_set1_pd( u[i+7])));
      a258 = _mm256_add_pd(a258,_mm256_mul_pd(g2,_mm256_set1_pd( u[i+8])));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  vH[0] = _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[0]),a036),
            _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[1]),a147),
              _mm256_mul_pd(_mm256_set1_pd(R[2]),a258)));
  vH[1] = _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[3]),a036),
            _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[4]),a147),
              _mm256_mul_pd(_mm256_set1_pd(R[5]),a258)));
  vH[2] = _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[6]),a036),
            _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(R[7]),a147),
              _mm256_mul_pd(_mm256_set1_pd(R[8]),a258)));
  // Take advantage of the fact that the pattern of usage is invariant
  // with respect to transpose _MM256_TRANSPOSE3_PD(h036,h147,h258);
}
static inline void compute_ort_s_voigt(FLOAT_PHYS* S, const __m256d* vH,
  const __m256d* vC, const FLOAT_PHYS dw){
  FLOAT_PHYS VECALIGNED H[12];
  _mm256_store_pd( &H[0], vH[0] );
  _mm256_store_pd( &H[4], vH[1] );
  _mm256_store_pd( &H[8], vH[2] );
  //__m256d s048 =//Vectorized calc for diagonal of S
  _mm256_store_pd( &S[0],
    _mm256_mul_pd(_mm256_set1_pd(dw),
      _mm256_add_pd(_mm256_mul_pd(vC[0],_mm256_set1_pd(H[0])),
        _mm256_add_pd(_mm256_mul_pd(vC[1],_mm256_set1_pd(H[5])),
          _mm256_mul_pd(vC[2],_mm256_set1_pd(H[10]))
          ))));
  {FLOAT_PHYS VECALIGNED c678[4];
  _mm256_store_pd( &c678[0], vC[3] );
  S[4]=(H[1] + H[4])*c678[0]*dw;// S[1]
  S[5]=(H[2] + H[8])*c678[2]*dw;// S[2]
  S[6]=(H[6] + H[9])*c678[1]*dw;// S[5]
  }
}
static inline void rotate_s_voigt( __m256d* vS,
    const FLOAT_PHYS* S, const __m256d* vR ){
  const __m256d s0 = _mm256_set1_pd(S[0]);
  const __m256d s1 = _mm256_set1_pd(S[4]);
  const __m256d s2 = _mm256_set1_pd(S[5]);
  vS[0]=_mm256_add_pd(_mm256_mul_pd(vR[0],s0),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s1),
      _mm256_mul_pd(vR[2],s2)));
  const __m256d s4 = _mm256_set1_pd(S[1]);
  const __m256d s5 = _mm256_set1_pd(S[6]);
  vS[1]=_mm256_add_pd(_mm256_mul_pd(vR[0],s1),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s4),
      _mm256_mul_pd(vR[2],s5)));
  const __m256d s8 = _mm256_set1_pd(S[2]);
  vS[2]=_mm256_add_pd(_mm256_mul_pd(vR[0],s2),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s5),
      _mm256_mul_pd(vR[2],s8)));
}
#if 0
static inline void compute_ort_s_voigt(__m256d* vS, const __m256d* vH,
  const __m256d* vC, const FLOAT_PHYS dw ){
  vS[0] =//Vectorized calc for diagonal of S
    _mm256_mul_pd(_mm256_set1_pd(dw),
      _mm256_add_pd(_mm256_mul_pd(vC[0],
        _mm256_permute4x64_pd( vH[0], _MM_SHUFFLE(0,0,0,0) )),
        _mm256_add_pd(_mm256_mul_pd(vC[1],
          _mm256_permute4x64_pd( vH[1], _MM_SHUFFLE(1,1,1,1) )),
          _mm256_mul_pd(vC[2],
            _mm256_permute4x64_pd( vH[2], _MM_SHUFFLE(2,2,2,2) ))
        )));
  {
  const __m256d d0 =_mm256_set_pd(0.0,dw,dw,dw);
  vS[1] =_mm256_setzero_pd();
  vS[1]+=_mm256_permute4x64_pd( vH[0]*d0, _MM_SHUFFLE(0,2,3,1) )*vC[3];
  vS[1]+=_mm256_permute4x64_pd( vH[1]*d0, _MM_SHUFFLE(1,3,2,0) )*vC[3];
  vS[1]+=_mm256_permute4x64_pd( vH[2]*d0, _MM_SHUFFLE(2,0,1,3) )*vC[3];
  vS[1] =_mm256_permute4x64_pd( vS[1]   , _MM_SHUFFLE(3,1,2,0) );
  }//NOTE The shear terms are arranged differently.
  // sxx syy szz ---   sxy sxz syz ---
}
#endif
#if 0
static inline void rotate_s_voigt( __m256d* vS, const __m256d* vR ){
  const __m256d s0 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(0,0,0,0) );// S[0]
  const __m256d s1 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(0,0,0,0) );// S[4]
  const __m256d s2 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(1,1,1,1) );// S[5]
  const __m256d s4 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(1,1,1,1) );// S[1]
  const __m256d s5 =_mm256_permute4x64_pd( vS[1], _MM_SHUFFLE(2,2,2,2) );// S[6]
  const __m256d s8 =_mm256_permute4x64_pd( vS[0], _MM_SHUFFLE(2,2,2,2) );// S[2]
  vS[0]=_mm256_add_pd(_mm256_mul_pd(vR[0],s0),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s1),
      _mm256_mul_pd(vR[2],s2)));
  vS[1]=_mm256_add_pd(_mm256_mul_pd(vR[0],s1),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s4),
      _mm256_mul_pd(vR[2],s5)));
  vS[2]=_mm256_add_pd(_mm256_mul_pd(vR[0],s2),
    _mm256_add_pd(_mm256_mul_pd(vR[1],s5),
      _mm256_mul_pd(vR[2],s8)));
}
#endif
// Isotropic intrinsics -------------------------------------------------------
static inline void compute_iso_s(__m256d* vS, const __m256d* vH,
  const FLOAT_PHYS lambda, const FLOAT_PHYS mu ){
#ifdef HAS_AVX2
  // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
  // Structure of S and H
  //  3   2   1     0
  // sxx sxy sxz | sxy
  // sxy syy syz | sxz
  // sxz syz szz | ---
  __m256d Ssum=_mm256_setzero_pd();
  {
  const __m256d z0 =_mm256_set_pd(0.0,1.0,1.0,1.0);
  const __m256d ml =_mm256_set_pd(lambda,mu,mu,mu);
  vS[0]=_mm256_permute4x64_pd( vH[0]*z0, _MM_SHUFFLE(0,2,3,1) );
  Ssum+= vS[0]*ml;
  vS[1]=_mm256_permute4x64_pd( vH[1]*z0, _MM_SHUFFLE(1,3,2,0) );
  Ssum+= vS[1]*ml;
  vS[2]=_mm256_permute4x64_pd( vH[2]*z0, _MM_SHUFFLE(2,0,1,3) );
  Ssum+= vS[2]*ml;
  }
  //      3   2   1   0
  //     sxy 0.0 sxz sxx
  //     sxy syz 0.0 syy
  //     0.0 syz sxz szz
  //
  // mu*(sxy syz sxz)trace(H)*lambda : Ssum
#if 0
  printf("H\n");
  print_m256(H[0]); print_m256(H[1]); print_m256(H[2]);
  printf("S step 1\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
  printf("Ssum\n");
  print_m256( Ssum );
#endif
  {
  const __m256d m2=_mm256_set_pd(2.0*mu,0.0,0.0,0.0);
#if 0
  vS[0]= Ssum + vS[0]*m2;
  vS[1]= Ssum + vS[1]*m2;
  vS[2]= Ssum + vS[2]*m2;
#endif
  //      3   2   1   0
  // mu*(sxy 0.0 sxz)sxx*2*mu+lambda*trace(H)
  // mu*(sxy syz 0.0)syy*2*mu+lambda*trace(H)
  // mu*(0.0 syz sxz)szz*2*mu+lambda*trace(H)
#if 0
  printf("S step 2\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
  vS[0]=_mm256_permute4x64_pd( Ssum + vS[0]*m2, _MM_SHUFFLE(3,2,0,3) );
  vS[1]=_mm256_permute4x64_pd( Ssum + vS[1]*m2, _MM_SHUFFLE(3,1,3,0) );
  vS[2]=_mm256_permute4x64_pd( Ssum + vS[2]*m2, _MM_SHUFFLE(3,3,1,2) );
  }
#if 0
  printf("S step 3\n");
  print_m256( S[0] ); print_m256( S[1] ); print_m256( S[2] );
#endif
#else
// Does not have avx2 support
  FLOAT_PHYS VECALIGNED fH[12], fS[12];
  _mm256_store_pd(&fH[0],vH[0]);
  _mm256_store_pd(&fH[4],vH[1]);
  _mm256_store_pd(&fH[8],vH[2]);
  {
  const __m256d mw= _mm256_set1_pd(mu);
  _mm256_store_pd( &fS[0], mw * _mm256_load_pd(&fH[0]) );// sxx sxy sxz | sxy
  _mm256_store_pd( &fS[4], mw * _mm256_load_pd(&fH[4]) );// sxy syy syz | sxz
  _mm256_store_pd( &fS[8], mw * _mm256_load_pd(&fH[8]) );// sxz syz szz | ---
  }
  const FLOAT_PHYS tr = (fH[0]+fH[5]+fH[10]) * lambda;
  fS[0]=2.0*fS[0]+tr; fS[5]=2.0*fS[5]+tr; fS[10]=2.0*fS[10]+tr;
  fS[1]+= fS[4];
  fS[2]+= fS[8];
  fS[6]+= fS[9];
  fS[4]=fS[1]; fS[9]=fS[6]; fS[8]=fS[2];
  vS[0] = _mm256_load_pd(&fS[0]); // [a3 a2 a1 a0]
  vS[1] = _mm256_load_pd(&fS[4]); // [a6 a5 a4 a3]
  vS[2] = _mm256_load_pd(&fS[8]); // [a9 a8 a7 a6]
#endif
}
static inline void compute_g_h(
  FLOAT_PHYS* G, __m256d* H,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* u ){
  H[0]=_mm256_setzero_pd(); H[1]=_mm256_setzero_pd(); H[2]=_mm256_setzero_pd();
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    { const __m256d g0 =
	_mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+0])),
	  _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+1])),
	    _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+2]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+0])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+1])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+2])));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    }if((i+5)<Ne){
      const __m256d g1 =
	_mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+3])),
	  _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+4])),
	    _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+5]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+3])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+4])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+5])));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d g2 =
	_mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+6])),
	  _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+7])),
	    _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+8]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+6])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+7])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+8])));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
}
static inline void compute_g_p_h(
  FLOAT_PHYS* G, __m256d* P, __m256d* H,
  const int Ne, const  __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* p, const FLOAT_PHYS* u ){
  H[0]=_mm256_setzero_pd(); H[1]=_mm256_setzero_pd(); H[2]=_mm256_setzero_pd(),
  P[0]=_mm256_setzero_pd(); P[1]=_mm256_setzero_pd(); P[2]=_mm256_setzero_pd();
  int ig=0;
  for(int i=0; i<Ne; i+=9){
    { const __m256d g0 =
        _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+0])),
          _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+1])),
            _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+2]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+0])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+1])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g0,_mm256_set1_pd( u[i+2])));
      P[0] = _mm256_add_pd(P[0],_mm256_mul_pd(g0,_mm256_set1_pd( p[i+0])));
      P[1] = _mm256_add_pd(P[1],_mm256_mul_pd(g0,_mm256_set1_pd( p[i+1])));
      P[2] = _mm256_add_pd(P[2],_mm256_mul_pd(g0,_mm256_set1_pd( p[i+2])));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    }if((i+5)<Ne){
      const __m256d g1 =
        _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+3])),
          _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+4])),
            _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+5]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+3])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+4])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g1,_mm256_set1_pd( u[i+5])));
      P[0] = _mm256_add_pd(P[0],_mm256_mul_pd(g1,_mm256_set1_pd( p[i+3])));
      P[1] = _mm256_add_pd(P[1],_mm256_mul_pd(g1,_mm256_set1_pd( p[i+4])));
      P[2] = _mm256_add_pd(P[2],_mm256_mul_pd(g1,_mm256_set1_pd( p[i+5])));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d g2 =
        _mm256_add_pd(_mm256_mul_pd(J[0],_mm256_set1_pd(isp[i+6])),
          _mm256_add_pd(_mm256_mul_pd(J[1],_mm256_set1_pd(isp[i+7])),
            _mm256_mul_pd(J[2],_mm256_set1_pd(isp[i+8]))));
      H[0] = _mm256_add_pd(H[0],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+6])));
      H[1] = _mm256_add_pd(H[1],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+7])));
      H[2] = _mm256_add_pd(H[2],_mm256_mul_pd(g2,_mm256_set1_pd( u[i+8])));
      P[0] = _mm256_add_pd(P[0],_mm256_mul_pd(g2,_mm256_set1_pd( p[i+6])));
      P[1] = _mm256_add_pd(P[1],_mm256_mul_pd(g2,_mm256_set1_pd( p[i+7])));
      P[2] = _mm256_add_pd(P[2],_mm256_mul_pd(g2,_mm256_set1_pd( p[i+8])));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
}
static inline void print_m256(const __m256d v){
  double V[4];
  _mm256_store_pd(&V[0],v);
  printf("%9.2e %9.2e %9.2e %9.2e\n",V[0],V[1],V[2],V[3]);
}
//FIXME Remove these later ====================================================
#if 0
//FIXME Refactor stress and strain tensors from double* to __m256d*
static inline void compute_g_p_h(
  FLOAT_PHYS* G, FLOAT_PHYS* P, FLOAT_PHYS* H,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* p, const FLOAT_PHYS* u ){
  __m256d
    a036=_mm256_setzero_pd(),a147=_mm256_setzero_pd(),a258=_mm256_setzero_pd(),
    b036=_mm256_setzero_pd(),b147=_mm256_setzero_pd(),b258=_mm256_setzero_pd();
  int ig=0;
  for(int i=0; i<Ne; i+=9){
    //__m256d p0,p1,p2,p3,p4,p5,p6,p7,p8;
    //__m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    //__m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    const __m256d is0= _mm256_set1_pd(isp[i+0]);
    const __m256d is1= _mm256_set1_pd(isp[i+1]);
    const __m256d is2= _mm256_set1_pd(isp[i+2]);
    const __m256d g0 = _mm256_add_pd(_mm256_mul_pd(J[0],is0),
      _mm256_add_pd(_mm256_mul_pd(J[1],is1),
        _mm256_mul_pd(J[2],is2)));
    const __m256d u0 = _mm256_set1_pd(  u[i+0]);
    const __m256d u1 = _mm256_set1_pd(  u[i+1]);
    const __m256d u2 = _mm256_set1_pd(  u[i+2]);
    a036 = _mm256_add_pd(a036, _mm256_mul_pd(g0,u0));
    a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1));
    a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
    const __m256d p0 = _mm256_set1_pd(  p[i+0]);
    const __m256d p1 = _mm256_set1_pd(  p[i+1]);
    const __m256d p2 = _mm256_set1_pd(  p[i+2]);
    b036 = _mm256_add_pd(b036, _mm256_mul_pd(g0,p0));
    b147 = _mm256_add_pd(b147, _mm256_mul_pd(g0,p1));
    b258 = _mm256_add_pd(b258, _mm256_mul_pd(g0,p2));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      const __m256d is3= _mm256_set1_pd(isp[i+3]);
      const __m256d is4= _mm256_set1_pd(isp[i+4]);
      const __m256d is5= _mm256_set1_pd(isp[i+5]);
      const __m256d g1 = _mm256_add_pd(_mm256_mul_pd(J[0],is3),
        _mm256_add_pd(_mm256_mul_pd(J[1],is4),
          _mm256_mul_pd(J[2],is5)));
      const __m256d u3 = _mm256_set1_pd(  u[i+3]);
      const __m256d u4 = _mm256_set1_pd(  u[i+4]);
      const __m256d u5 = _mm256_set1_pd(  u[i+5]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g1,u3));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
      const __m256d p3 = _mm256_set1_pd(  p[i+3]);
      const __m256d p4 = _mm256_set1_pd(  p[i+4]);
      const __m256d p5 = _mm256_set1_pd(  p[i+5]);
      b036 = _mm256_add_pd(b036, _mm256_mul_pd(g1,p3));
      b147 = _mm256_add_pd(b147, _mm256_mul_pd(g1,p4));
      b258 = _mm256_add_pd(b258, _mm256_mul_pd(g1,p5));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d is6= _mm256_set1_pd(isp[i+6]);
      const __m256d is7= _mm256_set1_pd(isp[i+7]);
      const __m256d is8= _mm256_set1_pd(isp[i+8]);
      const __m256d g2 = _mm256_add_pd(_mm256_mul_pd(J[0],is6),
        _mm256_add_pd(_mm256_mul_pd(J[1],is7),
          _mm256_mul_pd(J[2],is8)));
      const __m256d u6 = _mm256_set1_pd(  u[i+6]);
      const __m256d u7 = _mm256_set1_pd(  u[i+7]);
      const __m256d u8 = _mm256_set1_pd(  u[i+8]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g2,u6));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
      const __m256d p6 = _mm256_set1_pd(  p[i+6]);
      const __m256d p7 = _mm256_set1_pd(  p[i+7]);
      const __m256d p8 = _mm256_set1_pd(  p[i+8]);
      b036 = _mm256_add_pd(b036,_mm256_mul_pd(g2,p6));
      b147 = _mm256_add_pd(b147, _mm256_mul_pd(g2,p7));
      b258 = _mm256_add_pd(b258, _mm256_mul_pd(g2,p8));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  _mm256_store_pd(&H[0],a036);
  _mm256_store_pd(&H[4],a147);
  _mm256_store_pd(&H[8],a258);
  _mm256_store_pd(&P[0],b036);
  _mm256_store_pd(&P[4],b147);
  _mm256_store_pd(&P[8],b258);
}
static inline void compute_iso_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS C2, const __m256d c0,const __m256d c1,const __m256d c2,
  const FLOAT_PHYS dw){
  { // Scope vector registers
    __m256d s048;
    s048= _mm256_mul_pd(_mm256_set1_pd(dw),
      _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])),
        _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[5])),
          _mm256_mul_pd(c2,_mm256_set1_pd(H[10])))));
     _mm256_store_pd(&S[0], s048);
  } // end scoping unit
  S[5]=S[1]; S[10]=S[2];// Move the diagonal to their correct locations
  // Then, do the rest. Dunno if this is faster...
  S[1]=( H[1] + H[4] )*C2*dw;// S[3]= S[1];//Sxy Syx
  S[2]=( H[2] + H[8] )*C2*dw;// S[6]= S[2];//Sxz Szx
  S[6]=( H[6] + H[9] )*C2*dw;// S[7]= S[5];//Syz Szy
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
}
static inline void compute_iso_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS lambda, const FLOAT_PHYS mu ){
  // S = mu * (H+H^T) + lambda * I * ( H[0]+H[5]+H[10] )
#if 0
  for(int i=0; i<3; i++){
    for(int j=0; j<4; j++){ S[4* i+j] = H[4* i+j] * mu * dw; } }
  const FLOAT_PHYS tr = (H[0]+H[5]+H[10]) * lambda * dw;
  S[0]=2.0*S[0]+tr; S[5]=2.0*S[5]+tr; S[10]=2.0*S[10]+tr;
  S[1]+= S[4];
  S[2]+= S[8];
  S[6]+= S[9];
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
#else
  //FIXME These are all about the same speed as above, slower than ortho.
  const __m256d mw= _mm256_set1_pd(mu);
  _mm256_store_pd( &S[0], mw * _mm256_load_pd(&H[0]) );// sxx sxy sxz | sxy
  _mm256_store_pd( &S[4], mw * _mm256_load_pd(&H[4]) );// sxy syy syz | sxz
  _mm256_store_pd( &S[8], mw * _mm256_load_pd(&H[8]) );// sxz syz szz | ---
  const FLOAT_PHYS tr = (H[0]+H[5]+H[10]) * lambda;
  S[0]=2.0*S[0]+tr; S[5]=2.0*S[5]+tr; S[10]=2.0*S[10]+tr;
  S[1]+= S[4];
  S[2]+= S[8];
  S[6]+= S[9];
  S[4]=S[1]; S[9]=S[6]; S[8]=S[2];
#endif
}
static inline void compute_g_h(
  FLOAT_PHYS* G, FLOAT_PHYS* H,
  const int Ne, const __m256d* J,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* u ){
  __m256d a036=_mm256_setzero_pd(),
    a147=_mm256_setzero_pd(), a258=_mm256_setzero_pd();
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    //__m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    //__m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    const __m256d is0= _mm256_set1_pd(isp[i+0]);
    const __m256d is1= _mm256_set1_pd(isp[i+1]);
    const __m256d is2= _mm256_set1_pd(isp[i+2]);
    const __m256d g0 = _mm256_add_pd(_mm256_mul_pd(J[0],is0),
      _mm256_add_pd(_mm256_mul_pd(J[1],is1),
        _mm256_mul_pd(J[2],is2)));
    const __m256d u0 = _mm256_set1_pd(  u[i+0]);
    const __m256d u1 = _mm256_set1_pd(  u[i+1]);
    const __m256d u2 = _mm256_set1_pd(  u[i+2]);
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0));
    a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1));
    a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      const __m256d is3= _mm256_set1_pd(isp[i+3]);
      const __m256d is4= _mm256_set1_pd(isp[i+4]);
      const __m256d is5= _mm256_set1_pd(isp[i+5]);
      const __m256d g1 = _mm256_add_pd(_mm256_mul_pd(J[0],is3),
        _mm256_add_pd(_mm256_mul_pd(J[1],is4),
          _mm256_mul_pd(J[2],is5)));
      const __m256d u3 = _mm256_set1_pd(  u[i+3]);
      const __m256d u4 = _mm256_set1_pd(  u[i+4]);
      const __m256d u5 = _mm256_set1_pd(  u[i+5]);
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      const __m256d is6= _mm256_set1_pd(isp[i+6]);
      const __m256d is7= _mm256_set1_pd(isp[i+7]);
      const __m256d is8= _mm256_set1_pd(isp[i+8]);
      const __m256d g2 = _mm256_add_pd(_mm256_mul_pd(J[0],is6),
        _mm256_add_pd(_mm256_mul_pd(J[1],is7),
          _mm256_mul_pd(J[2],is8)));
      const __m256d u6 = _mm256_set1_pd(  u[i+6]);
      const __m256d u7 = _mm256_set1_pd(  u[i+7]);
      const __m256d u8 = _mm256_set1_pd(  u[i+8]);
      a036 = _mm256_add_pd(a036, _mm256_mul_pd(g2,u6));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  _mm256_store_pd(&H[0],a036);
  _mm256_store_pd(&H[4],a147);
  _mm256_store_pd(&H[8],a258);
}
#endif

#endif
