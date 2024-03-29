#ifndef INCLUDED_SOLV_H
#define INCLUDED_SOLV_H
#if VERB_MAX > 3
#include <iostream>
#endif
class Solv{
public:
  typedef std::valarray<FLOAT_SOLV> vals;
  typedef FLOAT_SOLV* valign;// memory-aligned vals
#ifdef ALIGN_SYS
  const uint valign_byte = SYSALIGN_BYTE;
#endif
  enum Meth {
    SOLV_GD=0, SOLV_CG=1, SOLV_NG=2, SOLV_CR=3
  };
  enum Cond {
    COND_NONE= 0, COND_JACO= 1, COND_TANG= 2, COND_JAC3= 3,
    COND_ROW1=11, COND_STRA=12
  };
  INT_MESH iter=0, iter_max=0;
  INT_MESH uinp_n,udof_n,cond_bloc_n=1;
  //
  FLOAT_SOLV load_scal=1.0;
  FLOAT_SOLV loca_rtol=0.0;
  FLOAT_SOLV loca_delu=0.0, rtol_delu=0.0;// Cauchy convergence check
  //
  // Stop when ALL resi_maxi < resi_rtol * resi_init;
  FLOAT_SOLV resi_init;// Initial force residual avg (for relative check)
  FLOAT_SOLV resi_maxi;// Current maximum residual force
  FLOAT_SOLV resi_rtol;// Max residual relative convergence tolerance
  //
  FLOAT_SOLV cube_init=0.0;// Start u0 at scaled iso cube exact solution
  FLOAT_SOLV loca_bmax[4]={0.0,0.0,0.0,0.0};
  FLOAT_SOLV glob_bmax[4]={0.0,0.0,0.0,0.0};
  //
  // Pointers to memory-aligned vanilla C arrays
  valign part_f;// f=[A]{u}//FIXME Move to Phys*?
  valign part_u;// solution
  valign part_r;// Residuals
  valign part_b;// RHS
  //NOTE Additional working vectors needed are defined in each solver subclass
  //FIXME Moved them back here for now
  valign part_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  valign part_p, part_q, part_g, prev_r, prev_u;//FIXME working vectors specific to method
  // The data is actually stored in corresponding C++ data_ objects.
  Solv::vals data_f;
  Solv::vals data_u;
  Solv::vals data_r;// Force residuals (PCG)
  Solv::vals data_b;// RHS
  Solv::vals data_d;
  Solv::vals data_p, data_q, data_g, data_o, data_l;
  //
  std::string meth_name="";
  //
  virtual int Solve( Elem*, Phys* )=0;//returns # iterations FIXME
  virtual int Setup( Elem*, Phys* )=0;
  //virtual int Setup()=0;
  //virtual int Precond( Elem*, Phys* )=0;
  //virtual int RHS( Mesh* )=0;
  //virtual int BC( Mesh* )=0;
  virtual int Init( Elem*, Phys* )=0;
  virtual int BC0( Elem*, Phys* )=0;
  virtual int Init()=0;
  virtual int Iter()=0;
  //virtual int Iter (Elem*, Phys*)=0;
  int Precond( Elem*, Phys*);
  //
  //FIXME Make the rest private or protected later?
  FLOAT_SOLV loca_rto2=0.0;// Solution Tolerance Squared
  FLOAT_SOLV loca_res2=0.0;// [r2b, alpha kept local to each iteration]
  INT_MESH halo_loca_0=0;// Associated Elem->halo_remo_n * Phys->node_d
  uint udof_flop=0, udof_band=0;
  int solv_cond;
protected:
  inline valign align_resize(RESTRICT Solv::vals&, INT_MESH, const uint);
  Solv( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) :
    iter_max(i), udof_n(n), loca_rtol(r){
    loca_rto2=loca_rtol*loca_rtol;
    //FIXME part_X vectors padded to allow simd access when node_d=simd_n-1.
#ifdef ALIGN_SYS
    part_f = align_resize( data_f, udof_n+1, valign_byte );// f=Au
    part_u = align_resize( data_u, udof_n+1, valign_byte );// solution
    part_r = align_resize( data_r, udof_n+1, valign_byte );// residuals
    //part_d = align_resize( data_d, 3*udof_n+1, valign_byte );// Preconditioner
    //FIXME Make this the right size
    //for(INT_MESH i=0; i<udof_n; i++){ part_0[i]=1.0; }
#else
    part_f.resize(udof_n+1,0.0);
    part_u.resize(udof_n+1,0.0);// Initial Solution Guess
    part_r.resize(udof_n+1,0.0);// Residuals
#endif
#if VERB_MAX > 13
    std::cout << valign_byte "-byte aligned pointer to part_f[0]: "
      << &part_f[0] <<'\n';
#endif
  };
private:
};
class PCG final: public Solv{
// Preconditioned Conjugate Gradient Kernel ----------------------------
public:
  //PCG() : Solv(){
  //  meth_name="preconditioned conjugate gradient";
  //};
  PCG( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="preconditioned conjugate gradient";
#ifdef ALIGN_SYS
    part_p = align_resize( data_p, udof_n+1, valign_byte );
#else
    part_p.resize(udof_n+1,0.0);// CG working vector
#endif
    udof_flop = 12;//*elem_n
    udof_band = 14*sizeof(FLOAT_SOLV);//*udof_n + 2
  };
  //PCG( ):Solv(12,11){};
  int Solve( Elem*, Phys* ) final;
  int Setup( Elem*, Phys* ) final;
  int Init ( Elem*, Phys* ) final;
  int Init () final;
  int Iter () final;
  //int Iter (Elem*, Phys*) final;
  //FIXME Make the rest private or protected later?
  //RESTRICT Solv::vals part_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals part_p;// [part_z no longer needed]
  int BC0( Elem*, Phys* Y ) final;
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
};
class PCR final: public Solv{
// Preconditioned Conjugate Residual Kernel ----------------------------
public:
  //PCR() : Solv(){
  //  meth_name="preconditioned conjugate residual";
  //};
  PCR( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="preconditioned conjugate residual";
#ifdef ALIGN_SYS
    part_p = align_resize( data_p, udof_n+1, valign_byte );
    part_g = align_resize( data_g, udof_n+1, valign_byte );
#else
    part_p.resize(udof_n+1,0.0);// CR working vector
    part_g.resize(udof_n+1,0.0);// CR working vector
#endif
    udof_flop = 14;//*elem_n
    udof_band = 17*sizeof(FLOAT_SOLV);//*udof_n +?
  };
  //PCR( ):Solv(14,13){};
  int Solve( Elem*, Phys* ) final;
  int Setup( Elem*, Phys* ) final;
  int Init ( Elem*, Phys* ) final;
  int Init () final;
  int Iter () final;
  //int Iter (Elem*, Phys*) final;
  //FIXME Make the rest private or protected later?
  //RESTRICT Solv::vals part_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals part_g;// [f]=[A][r], [g]=[A][p]
  int BC0( Elem*, Phys* Y ) final;
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
};
class NCG final: public Solv{// Nonlinear Conjugate Gradient Kernel -----------
public:
  NCG( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="nonlinear conjugate gradient";
#ifdef ALIGN_SYS
    part_b = align_resize( data_b, udof_n+1, valign_byte );
    part_p = align_resize( data_p, udof_n+1, valign_byte );
    part_g = align_resize( data_g, udof_n+1, valign_byte );
    part_q = align_resize( data_q, udof_n+1, valign_byte );
    prev_r = align_resize( data_o, udof_n+1, valign_byte );
    prev_u = align_resize( data_l, udof_n+1, valign_byte );
#else
    // part_d : diagonal preconditioner
    // part_b : RHS
    // part_u : solution
    // part_f : solution forces f = A u
    // part_r : force (gradient) residuals  r = A u - b
    part_b.resize(udof_n+1,0.0);// RHS
    part_p.resize(udof_n+1,0.0);// Search direction
    part_q.resize(udof_n+1,0.0);// perturbed solution gradient: q = u + alpha*p
    part_g.resize(udof_n+1,0.0);// residual grad. of perturbed sol. g = A q -b
    prev_r.resize(udof_n+1,0.0);
    prev_u.resize(udof_n+1,0.0);
#endif
    udof_flop = 23;//*elem_n
    udof_band = 10*sizeof(FLOAT_SOLV);//*udof_n + 2
  };
  //NCG( ):Solv(12,11){};
  int Solve( Elem*, Phys* ) final;
  int Setup( Elem*, Phys* ) final;
  int Init ( Elem*, Phys* ) final;
  int Init () final;
  int Iter () final;
  //int Iter (Elem*, Phys*) final;
  //FIXME Make the rest private or protected later?
  //RESTRICT Solv::vals part_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals part_p;// [part_z no longer needed]
  int BC0( Elem*, Phys* Y ) final;
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
};
//============= Inline Function Definitions ===============
#ifdef ALIGN_SYS
inline Solv::valign Solv::align_resize(RESTRICT Solv::vals &v,
  INT_MESH n, const uint bytes){
  //FIXME Hack to align part_f
  v.resize( n + bytes/sizeof(FLOAT_SOLV),0.0);
  intptr_t ptr = reinterpret_cast<intptr_t>(&v[0]);
  const auto offset = bytes - ( ptr % bytes );
  return(&v[( offset % bytes )/sizeof(FLOAT_SOLV)]);
};
#endif
//============= Inline Template Definitions ===============
#endif
