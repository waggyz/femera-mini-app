#ifndef ZYC_HAS_ZMAT_HPP
#define ZYC_HAS_ZMAT_HPP

#include "Zomplex.hpp"

namespace zyc {

  class Zmat {// hypercomplex matrix or vector
  public:
    Algebra     get_family ();
    Zorder_int  get_order  ();
    Layout      get_layout ();
    Zix_int     get_row_n  ();
    Zix_int     get_col_n  ();
    std::size_t all_size   ();// rows * cols * 2^order
    std::size_t mat_size   ();// rows * cols
    Zix_int    elem_size   ();// 2^order
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

}// end zyc:: namespace

#include "Zmat.ipp"

//end ZYC_HAS_ZMAT_HPP
#endif
