#ifndef ZYC_HAS_ZOMPLEX_IPP
#define ZYC_HAS_ZOMPLEX_IPP

namespace zyc {
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
  inline
  Algebra Zomplex::get_family   (){
    return this->family;
  }
  inline
  Zorder_int Zomplex::get_order    (){
    return this->order;
  }
  inline
  zyc::Layout Zomplex::get_layout   (){
    return this->layout;
  }
  inline
  Zomplex::Zomplex (Algebra z, Zorder_int p) : family (z), order(p) {
    if (p!=0) {layout = zyc::Layout::Block;}
  }
  inline
  Zomplex::Zomplex (Algebra z, Zorder_int p, zyc::Layout l) :
    family (z), order(p), layout(l) {
  }

}// end zyc:: namespace

//end ZYC_HAS_ZOMPLEX_IPP
#endif
