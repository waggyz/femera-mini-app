#ifndef ZYC_HAS_ZOMPLEX_HPP
#define ZYC_HAS_ZOMPLEX_HPP

#include "zyc.hpp"

namespace zyclops {

  struct Zomplex {// hypercomplex scalar
  public:
    Algebra     get_family ();
    Zorder_int  get_order  ();
    Zarray_int zval_size   ();
    Zarray_int   cr_size   ();
    Zarray_int   cr_nnz    ();
#if 0
    bool       is_vector    ();
    bool       is_conjugate ();
    bool       is_cr_matrix ();
    bool       is_cr_upper  ();
    bool       is_cr_lower  ();
    bool       is_cr_full   ();
    bool       is_sparse    ();
#endif
  public:
    Zomplex ()=default;
    Zomplex (Algebra, Zorder_int);
  private:
    Zorder_int order = 0;
    Algebra   family = Algebra::Real;
#if 0
    bool is_transpose = false;
    bool    has_upper = false;// triangular or full matrix storage
    bool    has_lower = false;// triangular or full matrix storage
    bool    has_zeros = true; // false: is sparse (zeros removed from matrix)
#endif
  };
#if 0
  const static Zomplex Real
    = Zomplex (Algebra::Real, 0, Layout::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Fcda, 2, Layout::Vector);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, Layout::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, zyc::Layout::Native);
#endif

}// end zyclops:: namespace

#include "Zomplex.ipp"

//end ZYC_HAS_ZOMPLEX_HPP
#endif
