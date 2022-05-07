#ifndef ZYC_HAS_ZMAT_HPP
#define ZYC_HAS_ZMAT_HPP

#include "Zomplex.hpp"

namespace zyc {

  class Zmat {// hypercomplex matrix or vector
  public:
    Algebra     get_family ();
    Zorder_int  get_order  ();
    Layout      get_layout ();
    Zsize_t     get_row_n  ();
    Zsize_t     get_col_n  ();
    std::size_t all_size   ();// rows * cols * 2^order
    std::size_t mat_size   ();// rows * cols
    std::size_t elem_size  ();// 2^order
  public:
    Zmat ()=default;
    Zmat (Algebra, Zorder_int, Zsize_t rows=1, Zsize_t cols=1);
    Zmat (Algebra, Zorder_int, Layout, Zsize_t rows=1, Zsize_t cols=1);
  private:
    Zsize_t     rows = 0;
    Zsize_t     cols = 0;
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

}// end zyc:: namespace

#include "Zmat.ipp"

//end ZYC_HAS_ZMAT_HPP
#endif
