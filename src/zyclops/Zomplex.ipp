#ifndef ZYC_HAS_ZOMPLEX_IPP
#define ZYC_HAS_ZOMPLEX_IPP

namespace zyclops {
  inline
  Zomplex::Zomplex (Algebra z, Zorder_int p) noexcept : order(p), family (z) {
  }
  inline
  Algebra Zomplex::get_family () noexcept {
    return this->family;
  }
  inline
  Zorder_int Zomplex::get_order () noexcept {
    return this->order;
  }
  inline
  Zindex_int Zomplex::zval_size () noexcept {// 2^m == 1 << m
    return Zindex_int(1) << this->order;
  }
  inline
  Zindex_int Zomplex::cr_size () noexcept {// (2^m)^2 == 4^m == 2^(2*m)
#if 0
    const auto n=size_t(1) << this->order;
    return n*n;
#endif
    return Zindex_int(1) << (this->order << 1);
  }
  inline
  Zindex_int Zomplex::cr_nnz () noexcept {
    switch (this->family) {
      case Algebra::Int     ://fall through
      case Algebra::Nat     ://fall through
      case Algebra::Real    :{ return 1; }
      case Algebra::Dual    :{ return upow (3, this->order); }
      case Algebra::Split   ://fall through
      case Algebra::Qcda    ://fall through
      case Algebra::Complex :{ return this->cr_size (); }
      case Algebra::Oti     ://fall through
      case Algebra::User    ://fall through
      case Algebra::Unknown :{}
    }
    return 0;
  }
//
}// end zyclops:: namespace

//end ZYC_HAS_ZOMPLEX_IPP
#endif
