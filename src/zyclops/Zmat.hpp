#ifndef ZYC_HAS_ZMAT_HPP
#define ZYC_HAS_ZMAT_HPP

#include "Zomplex.hpp"

namespace zyclops {

  class Zmat {// hypercomplex matrix or vector
  public:
    Algebra     get_family () noexcept;
    Zorder_int  get_order  () noexcept;
    Layout      get_layout () noexcept;
    Zarray_int  get_row_n  () noexcept;
    Zarray_int  get_col_n  () noexcept;
    std::size_t all_size   () noexcept;// rows * cols * 2^order
    std::size_t mat_size   () noexcept;// rows * cols
    Zarray_int elem_size   () noexcept;// 2^order
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
#if 0
  const static Zomplex Real
    = Zomplex (Algebra::Real, 0, Layout::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Quat, 2, Layout::Vector);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, Layout::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, zyc::Layout::Native);
#endif

}// end zyclops:: namespace

#include "Zmat.ipp"

//end ZYC_HAS_ZMAT_HPP
#endif
