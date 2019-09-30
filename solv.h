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
  const uint valign_byte = 64;
#endif
  enum Meth {
    SOLV_GD=0, SOLV_CG=1, SOLV_NG=2, SOLV_CR=3
  };
  enum Cond {
    COND_NONE=0, COND_JACO=3, COND_ROW1=1, COND_STRA=4
  };
  INT_MESH iter=0, iter_max=0;
  INT_MESH uinp_n,udof_n;
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
  //FIXME Rename to part_*
  valign sys_f;// f=[A]{u}//FIXME Move to Phys*?
  valign sys_u;// solution
  valign sys_r;// Residuals
  valign sys_b;// RHS
  valign sys_0;// Dirichlet (fixed to zero) BCs
  valign sys_1;// Fixed nonzero BCs
  //NOTE Additional working vectors needed are defined in each solver subclass
  //FIXME Moved them back here for now
  valign sys_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  valign sys_p, sys_q, sys_g, old_r, last_u;//FIXME working vectors specific to method
  // The data is actually stored in corresponding C++ objects.
  //FIXME Rename to data_*
  Solv::vals dat_f;
  Solv::vals dat_u;
  Solv::vals dat_r;// Force residuals (PCG)
  Solv::vals dat_b;// RHS
  Solv::vals dat_d;
  Solv::vals dat_0;
  Solv::vals dat_1;
  Solv::vals dat_p, dat_q, dat_g, dat_o, data_l;
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
#ifdef ALIGN_SYS
    sys_f = align_resize( dat_f, udof_n, valign_byte );// f=Au
    sys_u = align_resize( dat_u, udof_n, valign_byte );// solution
    sys_r = align_resize( dat_r, udof_n, valign_byte );// residuals
    sys_d = align_resize( dat_d, udof_n, valign_byte );// Preconditioner
    sys_0 = align_resize( dat_0, udof_n, valign_byte );// Dirichlet (fix 0) BC
    sys_1 = align_resize( dat_1, udof_n, valign_byte );// Fixed nonzero BC
    for(INT_MESH i=0; i<udof_n; i++){ sys_0[i]=1.0; }
#else
    sys_u.resize(udof_n,0.0);// Initial Solution Guess
    sys_r.resize(udof_n,0.0);// Residuals
    sys_d.resize(udof_n,0.0);// Diagonal Preconditioner
    sys_0.resize(udof_n,1.0);
#endif
#if VERB_MAX > 13
    std::cout << valign_byte "-byte aligned pointer to sys_f[0]: "
      << &sys_f[0] <<'\n';
#endif
  };
private:
};
class PCG final: public Solv{
// Preconditioned Conjugate Gradient Kernel ----------------------------
public:
  //PCG() : Solv(){
  //  meth_name="preconditioned cojugate gradient";
  //};
  PCG( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="preconditioned cojugate gradient";
#ifdef ALIGN_SYS
    sys_p = align_resize( dat_p, udof_n, valign_byte );
#else
    sys_p.resize(udof_n,0.0);// CG working vector
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
  //RESTRICT Solv::vals sys_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals sys_p;// [sys_z no longer needed]
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
  int BC0( Elem*, Phys* Y );
};
class PCR final: public Solv{
// Preconditioned Conjugate Residual Kernel ----------------------------
public:
  //PCR() : Solv(){
  //  meth_name="preconditioned cojugate residual";
  //};
  PCR( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="preconditioned cojugate residual";
#ifdef ALIGN_SYS
    sys_p = align_resize( dat_p, udof_n, valign_byte );
    sys_g = align_resize( dat_g, udof_n, valign_byte );
#else
    sys_p.resize(udof_n,0.0);// CR working vector
    sys_g.resize(udof_n,0.0);// CR working vector
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
  //RESTRICT Solv::vals sys_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals sys_g;// [f]=[A][r], [g]=[A][p]
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
  int BC0( Elem*, Phys* Y );
};
class NCG final: public Solv{// Nonlinear Conjugate Gradient Kernel -----------
public:
  NCG( INT_MESH n, INT_MESH i, FLOAT_PHYS r ) : Solv(n,i,r){
    meth_name="nonlinear cojugate gradient";
#ifdef ALIGN_SYS
    sys_b = align_resize( dat_b, udof_n, valign_byte );
    sys_p = align_resize( dat_p, udof_n, valign_byte );
    sys_g = align_resize( dat_g, udof_n, valign_byte );
    sys_q = align_resize( dat_q, udof_n, valign_byte );
    old_r = align_resize( dat_o, udof_n, valign_byte );
    last_u = align_resize( data_l, udof_n, valign_byte );
#else
    // sys_d : diagonal preconditioner
    // sys_b : RHS
    // sys_u : solution
    // sys_f : solution forces f = A u
    // sys_r : force (gradient) residuals  r = A u - b
    sys_b.resize(udof_n,0.0);// RHS
    sys_p.resize(udof_n,0.0);// Search direction
    sys_q.resize(udof_n,0.0);// perturbed solution gradient: q = u + alpha*p
    sys_g.resize(udof_n,0.0);// residual grad. of perturbed sol. g = A q -b
    old_r.resize(udof_n,0.0);
    last_u.resize(udof_n,0.0);
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
  //RESTRICT Solv::vals sys_d;//FIXME diagonal preconditioner w/ fixed BC DOFs set to zero
  //RESTRICT Solv::vals sys_p;// [sys_z no longer needed]
protected:
private:
  //int Precond( Elem*, Phys*);
  int RHS( Mesh* );//FIXME Remove?
  int BC ( Mesh* );//FIXME Remove?
  int RHS( Elem*, Phys* Y );
  int BCS( Elem*, Phys* Y );
  int BC0( Elem*, Phys* Y );
};
//============= Inline Function Definitions ===============
#ifdef ALIGN_SYS
inline Solv::valign Solv::align_resize(RESTRICT Solv::vals &v,
  INT_MESH n, const uint bytes){
  //FIXME Hack to align sys_f
  v.resize( n + bytes/sizeof(FLOAT_SOLV),0.0);
  intptr_t ptr = reinterpret_cast<intptr_t>(&v[0]);
  const auto offset = bytes - ( ptr % bytes );
  return(&v[( offset % bytes )/sizeof(FLOAT_SOLV)]);
};
#endif

#endif
