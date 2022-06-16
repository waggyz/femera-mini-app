#ifndef ZYC_HAS_ZOMPLEX_HPP
#define ZYC_HAS_ZOMPLEX_HPP

#include "zyc.hpp"

namespace zyclops {

  struct Zomplex {// hypercomplex number info
  public:
    Algebra     get_family () noexcept;
    Zorder_int  get_order  () noexcept;
    Zindex_int zval_size   () noexcept;
    Zindex_int   cr_size   () noexcept;
    Zindex_int   cr_nnz    () noexcept;
  public:
    Zomplex () noexcept=default;
    Zomplex (Algebra, Zorder_int) noexcept;
  private:
    Zorder_int order = 0;// default is real number
    Algebra   family = Algebra::Real;
  };
  const static Zomplex Real    = Zomplex (Algebra::Real,0);
  const static Zomplex Integer = Zomplex (Algebra::Int, 0);
  const static Zomplex Natural = Zomplex (Algebra::Nat, 0);
#if 0
  const static Zomplex Real = Zomplex (Algebra::Real, 0, Stored::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, Stored::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Qcda, 2, Stored::Mixed);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, Stored::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, zyc::Stored::Native);
#endif

}// end zyclops:: namespace

#include "Zomplex.ipp"

//end ZYC_HAS_ZOMPLEX_HPP
#endif
