#ifndef ZYC_HAS_ZMAT_HPP
#define ZYC_HAS_ZMAT_HPP

#include "Zomplex.hpp"

namespace zyclops {

  class Zmat {// hypercomplex matrix or vector
  public:
    Algebra     get_family () noexcept;
    Zorder_int  get_order  () noexcept;
    Layout      get_layout () noexcept;
    Zindex_int  get_row_n  () noexcept;
    Zindex_int  get_col_n  () noexcept;
    std::size_t all_size   () noexcept;// rows * cols * 2^order
    std::size_t mat_size   () noexcept;// rows * cols
    Zindex_int elem_size   () noexcept;// 2^order
  public:
    Zmat ()=default;
    Zmat (Algebra, Zorder_int, std::size_t rows=1, std::size_t cols=1);
    Zmat (Algebra, Zorder_int, Layout, std::size_t rows=1, std::size_t cols=1);
  private:
    std::size_t rows = 0;
    std::size_t cols = 0;
    Zorder_int order = 0;
    Algebra   family = Algebra::Real;
    Layout    layout = Layout::Native;
  };

}// end zyclops:: namespace

#include "Zmat.ipp"

//end ZYC_HAS_ZMAT_HPP
#endif
