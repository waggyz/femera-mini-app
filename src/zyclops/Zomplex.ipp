#ifndef ZYC_HAS_ZOMPLEX_IPP
#define ZYC_HAS_ZOMPLEX_IPP

namespace zyclops {
  inline
  Algebra Zomplex::get_family   (){
    return this->family;
  }
  inline
  Zorder_int Zomplex::get_order    (){
    return this->order;
  }
  inline
  Zomplex::Zomplex (Algebra z, Zorder_int p)
    :  order(p), family (z) {
  }
  inline
  Zarray_int Zomplex::zval_size () {
    return size_t(1) << this->order;
  }
  inline
  Zarray_int Zomplex::cr_size () {
    const auto n=size_t(1) << this->order;
    return n*n;
  }
  inline
  Zarray_int Zomplex::cr_nnz () {
    switch (this->family) {
      case Algebra::Int     ://fall through
      case Algebra::Nat     ://fall through
      case Algebra::Real    :{ return 1; }
      case Algebra::Dual    :{ return upow (3, this->order); }
      case Algebra::Split   ://fall through
      case Algebra::Fcda    ://fall through
      case Algebra::Complex :{ return this->cr_size (); }
      case Algebra::Oti     ://fall through
      case Algebra::User    ://fall through
      case Algebra::Unknown :{}
    }
    return 0;
  }
#if 0
  inline
  bool Zomplex::is_cr_matrix (){
    return this->has_upper || this->has_lower;
  }
  inline
  bool Zomplex::is_cr_upper  (){
    return this->has_upper && ! this->has_lower;
  }
  inline
  bool Zomplex::is_cr_lower  (){
    return ! this->has_upper && this->has_lower;
  }
  inline
  bool Zomplex::is_cr_full   (){
    return this->has_upper && this->has_lower;
  }
  inline
  bool Zomplex::is_vector    (){
    return ! this->is_cr_matrix ();
  }
  inline
  bool Zomplex::is_conjugate (){
    return this->is_transpose;
  }
  inline
  bool Zomplex::is_sparse    (){
    return ! has_zeros;
  }
#endif

}// end zyclops:: namespace

//end ZYC_HAS_ZOMPLEX_IPP
#endif
