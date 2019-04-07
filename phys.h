#ifndef INCLUDED_PHYS_H
#define INCLUDED_PHYS_H

class Phys{
public:
  typedef std::valarray<FLOAT_PHYS> vals;
  //typedef std::vector<FLOAT_PHYS> valign;
  INT_DOF ndof_n;// Degrees of freedom per node:
  // 2 for elastic 2D, 3 for elastic 3D, 1 for thermal
  //INT_DOF ndop_n;// Padded degrees of freedom
  FLOAT_PHYS part_sum1=0.0;
  //
  virtual int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&)=0;
  virtual int ElemLinear( Elem*,FLOAT_SOLV*,const FLOAT_SOLV*)=0;
  virtual int ElemJacobi( Elem*,FLOAT_SOLV* )=0;// Jacobi Preconditioner
  virtual int ElemRowSumAbs(Elem*, FLOAT_SOLV* )=0;// Row Norm Preconditioner
  virtual int ElemStrain(Elem*, FLOAT_SOLV* )=0;// Applied Element Strain Preconditioner
  virtual int ElemLinear( Elem* )=0;//FIXME OLD
  virtual int ElemJacobi( Elem* )=0;//FIXME old
  //
  virtual inline int MtrlProp2MatC( )=0;//why does this inline?
  virtual RESTRICT Solv::vals MtrlLinear(//FIXME Not used for 3D yet
    const RESTRICT Solv::vals &strain )=0;
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
  virtual int ScatStiff( Elem* )=0;//FIXME should be local to subclass?
  int ElemStiff( RESTRICT Solv::vals& );//for comparison to LMA EBE
  int ElemStiff();
  //
  int SavePartFMR( const char* bname, bool is_bin );//FIXME ASCII/Binary file format
  int ReadPartFMR( const char* bname, bool is_bin );
  //
  int tens_flop=0, tens_band=0;
  int stif_flop=0, stif_band=0;
  //
  RESTRICT Solv::vals mtrl_prop;// Conventional Material Properties
  // (Young's, Poisson's, etc.)
  RESTRICT Solv::vals mtrl_dirs;// Orientation [x,z,x]
  //
  RESTRICT Solv::vals mtrl_matc;// Unique D-matrix values
  RESTRICT Solv::vals mtrl_rotc;
  //
  RESTRICT Solv::vals elem_inout;// Elemental nodal value workspace (serial)
  // Fill w/ Phys::ScatterNode2Elem(...),ElemLinear(Elem*),ElemJacobi(),...
  RESTRICT Solv::vals elem_in, elem_out;// Double-buffer for parallel
  // The next is for comparison to traditional EBE
  RESTRICT Solv::vals elem_stiff;// Fill w/ Phys::ScatterStiff(...)
  //size_t elem_linear_flop=0;
protected:
  Phys( Solv::vals p ) : mtrl_prop(p){};
  Phys( Solv::vals p, Solv::vals d ) : mtrl_prop(p),mtrl_dirs(d){};
  //constructor computes material vals
  inline RESTRICT Solv::vals Tens2VoigtEng(const RESTRICT Solv::vals&);
  inline RESTRICT Solv::vals Tens3VoigtEng(const RESTRICT Solv::vals&);
  inline RESTRICT Solv::vals Tens2VoigtEng(const FLOAT_PHYS H[4]);
  inline RESTRICT Solv::vals Tens3VoigtEng(const FLOAT_PHYS H[9]);
private:
};
// Inline Functions =======================================
//
inline RESTRICT Solv::vals Phys::Tens3VoigtEng(const RESTRICT Solv::vals &H){
  return(Solv::vals { H[0],H[4],H[8], H[1]+H[3],H[5]+H[7],H[2]+H[6] });
  // exx,eyy,ezz, exy,eyz,exz
};
inline RESTRICT Solv::vals Phys::Tens3VoigtEng(const FLOAT_PHYS H[9]){
  return(Solv::vals { H[0],H[4],H[8], H[1]+H[3],H[5]+H[7],H[2]+H[6] });};
  // exx,eyy,ezz, exy,eyz,exz
inline RESTRICT Solv::vals Phys::Tens2VoigtEng(const RESTRICT Solv::vals &H){
  return(Solv::vals { H[0],H[3], H[1]+H[2] });};
inline RESTRICT Solv::vals Phys::Tens2VoigtEng(const FLOAT_PHYS H[4]){
  return(Solv::vals { H[0],H[3], H[1]+H[2] });
};
// Physics Kernels: ---------------------------------------
class ElastIso2D final: public Phys{
public: ElastIso2D(FLOAT_PHYS young, FLOAT_PHYS poiss, FLOAT_PHYS thick) :
  Phys((Solv::vals){young,poiss,thick}){// Constructor
    this->ndof_n      = 2;
    //this->elem_flop = FIXME;
    ElastIso2D::MtrlProp2MatC();
  };
  int Setup( Elem* )final;
  int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&) final;
  int ElemLinear( Elem*,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ScatStiff ( Elem* ) final;// Used for testing traditional EBE
  inline int MtrlProp2MatC()final{//why does this inline?
    const FLOAT_PHYS E=mtrl_prop[0];
    const FLOAT_PHYS n=mtrl_prop[1];
    const FLOAT_PHYS t=mtrl_prop[2];
    const FLOAT_PHYS d=E/(1.0-n*n)*t;
    mtrl_matc.resize(3); mtrl_matc={ d, n*d, (1.0-n)*d*0.5 };//*0.5 eng. strain
    return 0;
  };
  RESTRICT Solv::vals MtrlLinear(//FIXME Doesn't inline
    const RESTRICT Solv::vals &strain_tensor)final{//FIXME Plane Stress
    const Solv::vals e=Tens2VoigtEng(strain_tensor);
    return( Solv::vals {
      mtrl_matc[0]*e[0] +mtrl_matc[1]*e[1] ,
      mtrl_matc[1]*e[0] +mtrl_matc[0]*e[1] ,
      mtrl_matc[2]*e[2] });
    };
protected:
private:
};
class ElastIso3D final: public Phys{
public: ElastIso3D(FLOAT_PHYS young, FLOAT_PHYS poiss ) :
  Phys((Solv::vals){young,poiss}){
    this->ndof_n = 3;
    //this->elem_flop = 225;//FIXME Tensor eval for linear tet
    // calc stiff_flop from (ndof_n*E->elem_node_n)*(ndof_n*E->elem_node_n-1.0)
    ElastIso3D::MtrlProp2MatC();
  };
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Solv::vals&,const RESTRICT Solv::vals&) final;
  int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&) final;
  int ElemLinear( Elem*,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) final;
  int ElemJacobi( Elem* ) final;
  int ScatStiff ( Elem* ) final;
  inline int MtrlProp2MatC()final{//why does this inline?
    const FLOAT_PHYS E =mtrl_prop[0];
    const FLOAT_PHYS nu=mtrl_prop[1];
    const FLOAT_PHYS d =E/((1.0+nu)*(1.0-2.0*nu));
    //const FLOAT_PHYS C11=(1.0-n)*d;
    //const FLOAT_PHYS C12=n*d ;
    //const FLOAT_PHYS C44=(1.0-2.0*n)*d*0.5;
    ////const Solv::vals C={C11,C12,C44};
    //return( Solv::vals {C11,C12,C44} );
    mtrl_matc.resize(3); mtrl_matc={ (1.0-nu)*d,nu*d,(1.0-2.0*nu)*d*0.5};
    return 0;
  };
  RESTRICT Solv::vals MtrlLinear( const RESTRICT Solv::vals &e)final{
    //FIXME Doesn't inline
    //const Solv::vals e=Tens3VoigtEng(strain_tensor);
    RESTRICT Solv::vals s(0.0,6);
    s[0]= mtrl_matc[0]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[1]*e[8];
    s[1]= mtrl_matc[1]*e[0] +mtrl_matc[0]*e[4] +mtrl_matc[1]*e[8];
    s[2]= mtrl_matc[1]*e[0] +mtrl_matc[1]*e[4] +mtrl_matc[0]*e[8];
    // Fused multiply-add probably better
    s[3]= mtrl_matc[2]*e[1] +mtrl_matc[2]*e[3];
    s[4]= mtrl_matc[2]*e[5] +mtrl_matc[2]*e[7];
    s[5]= mtrl_matc[2]*e[2] +mtrl_matc[2]*e[6];
    //FIXME Tensor form: http://solidmechanics.org/text/Chapter3_2/Chapter3_2.htm
    return s;
  };
protected:
private:
};
class ElastOrtho3D final: public Phys{
public:
  ElastOrtho3D(// Isotropic Material Constructor
    FLOAT_PHYS young, FLOAT_PHYS poiss ) :
    Phys((Solv::vals){young,poiss},
         (Solv::vals){0.0,0.0,0.0} )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Isotropic Material Constructor (Rotated)
    FLOAT_PHYS r1z  , FLOAT_PHYS r2x  , FLOAT_PHYS r3z,
    FLOAT_PHYS young, FLOAT_PHYS poiss ) :
    Phys((Solv::vals){ young, poiss },
         (Solv::vals){r1z,r2x,r3z} )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Cubic Material Constructor
    FLOAT_PHYS r1z  , FLOAT_PHYS r2x  , FLOAT_PHYS r3z,
    FLOAT_PHYS young, FLOAT_PHYS poiss, FLOAT_PHYS shear ) :
    Phys((Solv::vals){ young, poiss, shear },
         (Solv::vals){r1z,r2x,r3z} )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Transversely Isotropic Material Constructor
    FLOAT_PHYS r1z, FLOAT_PHYS r2x, FLOAT_PHYS r3z,
    FLOAT_PHYS C11, FLOAT_PHYS C33,
    FLOAT_PHYS C12, FLOAT_PHYS C13, FLOAT_PHYS C44 ) :
    Phys((Solv::vals){C11,C33, C12,C13, C44 },
         (Solv::vals){r1z,r2x,r3z} )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Orthotropic Material Constructor
    FLOAT_PHYS r1z, FLOAT_PHYS r2x, FLOAT_PHYS r3z,
    FLOAT_PHYS C11, FLOAT_PHYS C22, FLOAT_PHYS C33,
    FLOAT_PHYS C12, FLOAT_PHYS C23, FLOAT_PHYS C13,
    FLOAT_PHYS C44, FLOAT_PHYS C55, FLOAT_PHYS C66 ) :
    Phys((Solv::vals){C11,C22,C33, C12,C23,C13, C44,C55,C66 },
         (Solv::vals){r1z,r2x,r3z} )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  ElastOrtho3D(// Orthotropic Material Constructor
    Solv::vals prop, Solv::vals dirs ) :
    Phys( prop,dirs )
     { ndof_n = 3; ElastOrtho3D::MtrlProp2MatC(); };
  int Setup( Elem* )final;
  //int ElemLinear( std::vector<Elem*>,RESTRICT Solv::vals&,const RESTRICT Solv::vals&)
  int BlocLinear( Elem*,RESTRICT Solv::vals&,const RESTRICT Solv::vals&) final;
  int ElemLinear( Elem*,FLOAT_SOLV*,const FLOAT_SOLV*) final;
  int ElemJacobi( Elem*,FLOAT_SOLV* ) final;
  int ElemRowSumAbs(Elem*, FLOAT_SOLV* ) final;
  int ElemStrain(Elem*, FLOAT_SOLV* ) final;
  int ElemLinear( Elem* ) ;
  int ElemJacobi( Elem* ) ;
  int ScatStiff ( Elem* ) ;
  inline int MtrlProp2MatC()final{
    const FLOAT_PHYS z1=mtrl_dirs[0];// Rotation about z (radians)
    const FLOAT_PHYS x2=mtrl_dirs[1];// Rotation about x (radians)
    const FLOAT_PHYS z3=mtrl_dirs[2];// Rotation about z (radians)
    //mtrl_rotc.resize(6);// Rotation Matrix Components
    //mtrl_rotc={cos(z1),sin(z1),cos(x2),sin(x2),cos(z3),sin(z3)};
    mtrl_rotc.resize(9);// Rotation Matrix
    Solv::vals Z1={cos(z1),sin(z1),0.0, -sin(z1),cos(z1),0.0, 0.0,0.0,1.0};
    Solv::vals X2={1.0,0.0,0.0, 0.0,cos(x2),sin(x2), 0.0,-sin(x2),cos(x2)};
    Solv::vals Z3={cos(z3),sin(z3),0, -sin(z3),cos(z3),0.0, 0.0,0.0,1.0};
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
  RESTRICT Solv::vals MtrlLinear(const RESTRICT Solv::vals &e)final{
    RESTRICT Solv::vals S(9);//FIXME
    //RESTRICT Solv::vals s(6);
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
    //const RESTRICT Solv::vals E = MatMul3x3xN( mtrl_rotc,e );
    RESTRICT Solv::vals E(9);
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
    //const RESTRICT Solv::vals s = MatMul3x3xNT( mtrl_rotc,S );//FIXME
    RESTRICT Solv::vals s(9);
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
/*
// 2D Elastic Continuum ---------------------------------
inline RESTRICT Solv::vals Mtr2StrsIsoEng(//FIXME Plane Stress
  const FLOAT_PHYS E, const FLOAT_PHYS n, const FLOAT_PHYS t){
  const FLOAT_PHYS d=E/(1.0-n*n)*t;
  //const FLOAT_PHYS C11=d;
  //const FLOAT_PHYS C12=n*d ;
  //const FLOAT_PHYS C33=(1.0-n)*d*0.5;
  ////const Solv::vals C={C11,C12,C44};
  //return( Solv::vals {C11,C12,C33} );
  return(Solv::vals { d, n*d, (1.0-n)*d*0.5 } );
};
inline RESTRICT Solv::vals Mtr2ValsIsoEng2Cau(
  RESTRICT const Solv::vals &C, RESTRICT const Solv::vals &e){
  return( Solv::vals {
    C[0]*e[0] +C[1]*e[1] ,
    C[1]*e[0] +C[0]*e[1] ,
    C[2]*e[2] });
  //RESTRICT Solv::vals s(3,0.0);
  //s[0]= C[0]*e[0] +C[1]*e[1] ;
  //s[1]= C[1]*e[0] +C[0]*e[1] ;
  //s[2]= C[2]*e[2] ;
  //return s;
};
//FIXED 2D homogeneous isotropic already done.
//FIXME Redo the following derived from Phys::...
// 3D Elastic Continuum ---------------------------------
inline RESTRICT Solv::vals MtrlValsIsoEng(
  const FLOAT_PHYS E, const FLOAT_PHYS n){
  const FLOAT_PHYS d=E/((1.0+n)*(1.0-2.0*n));
  //const FLOAT_PHYS C11=(1.0-n)*d;
  //const FLOAT_PHYS C12=n*d ;
  //const FLOAT_PHYS C44=(1.0-2.0*n)*d*0.5;
  ////const Solv::vals C={C11,C12,C44};
  //return( Solv::vals {C11,C12,C44} );
  return( Solv::vals { (1.0-n)*d,n*d,(1.0-2.0*n)*d*0.5 } );
};
inline RESTRICT Solv::vals MtrlValsIsoEng2Cau(
  RESTRICT const Solv::vals &C, RESTRICT const Solv::vals &e){
  RESTRICT Solv::vals s(0.0,6);//0.0,0.0,0.0, 0.0,0.0,0.0};
  //RESTRICT Solv::vals C1={
  //  C[1],C[2],C[2],
  //  C[2],C[1],C[2],
  //  C[2],C[2],C[1]};
  //s1=Mesh::MatMul3xNx3t(C1,e);
  //s[1]=s1[1]; s[2]=s1[2]; s[3]=s1[3];
  //s[1]=Solv::inner_product(Solv::vals {C[1],C[4],C[6]},e1);
  s[0]= C[0]*e[0] +C[1]*e[1] +C[1]*e[2];
  s[1]= C[1]*e[0] +C[0]*e[1] +C[1]*e[2];
  s[2]= C[1]*e[0] +C[1]*e[1] +C[0]*e[2];
  s[3]= C[2]*e[3] ;
  s[4]= C[2]*e[4] ;
  s[5]= C[2]*e[5] ;
  return s;
};
inline RESTRICT Solv::vals Phys::MtrlValsTransEng(
  const FLOAT_PHYS Ep , const FLOAT_PHYS Epz,//FIXME What convention?
  const FLOAT_PHYS np , const FLOAT_PHYS npz,
  const FLOAT_PHYS Gzp ){
  //const FLOAT_PHYS d=Ep*Ep*Ep/((1+np)*(1-np-2*npz*nzp));
  const FLOAT_PHYS d=1.0;//1.0/(1-np*np-2*npz*nzp);//FIXME
  const FLOAT_PHYS C11=(0)*d;//FIXME
  const FLOAT_PHYS C33=(0)*d;//FIXME
  const FLOAT_PHYS C12=(0)*d;//FIXME
  //const FLOAT_PHYS C23=()*d;
  const FLOAT_PHYS C13=(0)*d;//FIXME
  const FLOAT_PHYS C44=(0)*d;//FIXME
  //const FLOAT_PHYS C66=()*d;
  const Solv::vals C={C11,C33,C12,C13,C44};
  return(C);
};
inline RESTRICT Solv::vals Phys::MtrlValsTransEng2Cau(
  RESTRICT const Solv::vals &C, RESTRICT const Solv::vals &e){
  RESTRICT Solv::vals s(6);
  //RESTRICT Solv::vals C1={
  //  C[1],C[2],C[2],
  //  C[2],C[1],C[2],
  //  C[2],C[2],C[1]};
  s[0]= C[0]*e[0] +C[2]*e[1] +C[3]*e[2];
  s[1]= C[2]*e[0] +C[0]*e[1] +C[3]*e[2];
  s[2]= C[3]*e[0] +C[3]*e[1] +C[1]*e[2];
  s[3]= C[4]*e[3];
  s[4]= C[4]*e[4];
  s[5]=(C[0]-C[2])*0.5*e[5];
  return s;
};
//inline int MtrlValsOrthoTru( RESTRICT const Solv::vals& e,
inline RESTRICT Solv::vals MtrlValsOrthoEng(
  const FLOAT_PHYS Ex , const FLOAT_PHYS Ey , const FLOAT_PHYS Ez ,
  const FLOAT_PHYS Gxy, const FLOAT_PHYS Gyz, const FLOAT_PHYS Gxz,
  const FLOAT_PHYS nxy, const FLOAT_PHYS nyz, const FLOAT_PHYS nxz){
  //
  const FLOAT_PHYS nyx=nxy*Ey/Ex;
  const FLOAT_PHYS nzy=nyz*Ez/Ey;
  const FLOAT_PHYS nzx=nxz*Ez/Ex;
  const FLOAT_PHYS d=Ex*Ey*Ez/(1.0-nxy*nyx-nyz*nzy-nzx*nxz-2.0*nxy*nyz*nzx);
  //
  const FLOAT_PHYS C11=(1.0-nyz*nzy)/(Ey*Ez)*d;
  const FLOAT_PHYS C22=(1.0-nzx*nxz)/(Ez*Ez)*d;
  const FLOAT_PHYS C33=(1.0-nxy*nyz)/(Ex*Ey)*d;
  //
  const FLOAT_PHYS C12=-(nxy+nxz*nzy)/(Ez*Ex)*d;
  const FLOAT_PHYS C13=-(nxz+nxy*nyz)/(Ez*Ex)*d;
  const FLOAT_PHYS C23=-(nyz+nxz*nyz)/(Ez*Ey)*d;
  //
  const FLOAT_PHYS C44=Gyz;// * 2.0 for true strain
  const FLOAT_PHYS C55=Gxz;
  const FLOAT_PHYS C66=Gxy;//FIXED make function to return these
  //
  //const Solv::vals C={C11,C22,C33, C12,C23,C13, C44,C55,C66};
  return( Solv::vals {C11,C22,C33, C12,C23,C13, C44,C55,C66} );
  //
  //const RESTRICT Solv::vals C={//FIXED Don't do it this way.
  //  C11,C12,C13,0.0,0.0,0.0,
  //  C12,C22,C23,0.0,0.0,0.0,
  //  C13,C23,C33,0.0,0.0,0.0,
  //  0.0,0.0,0.0,C44,0.0,0.0,
  //  0.0,0.0,0.0,0.0,C55,0.0,
  //  0.0,0.0,0.0,0.0,0.0,C66};
  //return(0);
  };
inline RESTRICT Solv::vals MtrlValsOrthoEng2Cau(
  RESTRICT const Solv::vals &C, RESTRICT const Solv::vals &e){
  //RESTRICT Solv::vals e1={e[1],e[2],e[3]};
  RESTRICT Solv::vals s(6);
  //RESTRICT Solv::vals C1={
  //  C[0],C[3],C[5],
  //  C[3],C[1],C[4],
  //  C[5],C45],C[2]};
  //s1=Mesh::MatMul3xNx3t(C1,e);
  //s[1]=s1[1]; s[2]=s1[2]; s[3]=s1[3];
  //s[1]=Solv::inner_product(Solv::vals {C[1],C[4],C[6]},e1);
  s[0]= C[0]*e[0] +C[3]*e[1] +C[5]*e[2];
  s[1]= C[3]*e[0] +C[1]*e[1] +C[4]*e[2];
  s[2]= C[5]*e[0] +C[4]*e[1] +C[2]*e[2];
  s[3]= C[6]*e[3];
  s[4]= C[7]*e[4];
  s[5]= C[8]*e[5];
  return s;
};
*/







#endif
