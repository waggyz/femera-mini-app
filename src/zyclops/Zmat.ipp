#ifndef ZYC_HAS_ZMAT_IPP
#define ZYC_HAS_ZMAT_IPP

namespace zyc {
  inline
  Algebra Zmat::get_family   (){
    return this->family;
  }
  inline
  Zorder_int Zmat::get_order    (){
    return this->order;
  }
  inline
  zyc::Layout Zmat::get_layout   (){
    return this->layout;
  }
  inline
  std::size_t Zmat::all_size   (){
    return this->rows * this->cols * (std::size_t(1) << this->order);
  }
  inline
  std::size_t Zmat::mat_size   (){
    return this->rows * this->cols;
  }
  inline
  zyc::Zsize_t Zmat::elem_size   (){
    return (zyc::Zsize_t(1) << this->order);
  }
  inline
  Zmat::Zmat (Algebra z, Zorder_int p, std::size_t r, std::size_t c)
    :  rows(r), cols(c), order(p), family (z) {
    if (p!=0) {layout = zyc::Layout::Block;}
  }
  inline
  Zmat::Zmat (Algebra z, Zorder_int p, zyc::Layout l,
    std::size_t r, std::size_t c) :
    rows(r), cols(c), order(p), family (z), layout(l) {
  }

}// end zyc:: namespace

//end ZYC_HAS_ZMAT_IPP
#endif
