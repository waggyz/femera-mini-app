#ifndef ZYC_HAS_ZMAT_IPP
#define ZYC_HAS_ZMAT_IPP

namespace zyclops {
  inline
  Algebra Zmat::get_family ()
  noexcept {
    return this->family;
  }
  inline
  Zorder_int Zmat::get_order ()
  noexcept {
    return this->order;
  }
  inline
  zyclops::Layout Zmat::get_layout ()
  noexcept {
    return this->layout;
  }
  inline
  std::size_t Zmat::all_size ()
  noexcept {
    return this->rows * this->cols * (std::size_t(1) << this->order);
  }
  inline
  std::size_t Zmat::mat_size ()
  noexcept {
    return this->rows * this->cols;
  }
  inline
  zyclops::Zarray_int Zmat::elem_size ()
  noexcept {
    return (Zarray_int(1) << this->order);
  }
  inline
  Zmat::Zmat (Algebra z, Zorder_int p, std::size_t r, std::size_t c)
   : rows (r), cols (c), order (p), family (z) {
    if (p!=0) { layout = Layout::Block; }
  }
  inline
  Zmat::Zmat (Algebra z, Zorder_int p, Layout l, std::size_t r, std::size_t c)
   : rows (r), cols (c), order (p), family (z), layout (l) {
  }

}// end zyclops:: namespace

//end ZYC_HAS_ZMAT_IPP
#endif
