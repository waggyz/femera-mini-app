#ifndef ZYC_HAS_ZOMPLEX_HPP
#define ZYC_HAS_ZOMPLEX_HPP

#include "zyc.hpp"

namespace zyc {

  class Zomplex {
  public:
    bool        is_cr_matrix ();
    bool        is_cr_upper  ();
    bool        is_cr_lower  ();
    bool        is_cr_full   ();
    bool        is_vector    ();
    bool        is_conjugate ();
    bool        is_sparse    ();
    Algebra     get_family   ();
    Zorder_int  get_order    ();
    zyc::Layout get_layout   ();
  public:
    Zomplex ()=default;
    Zomplex (Algebra, Zorder_int);
    Zomplex (Algebra, Zorder_int, zyc::Layout);
  private:
    Algebra           family = Algebra::Real;
    Zorder_int         order = 0;
    zyc::Layout layout = zyc::Layout::Native;
    bool        is_transpose = false;
    bool           has_upper = false;// triangular or full matrix storage
    bool           has_lower = false;// triangular or full matrix storage
    bool           has_zeros = true; // false: is sparse (zero terms removed)
  };
  const static Zomplex Real
    = Zomplex (Algebra::Real, 0, zyc::Layout::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, zyc::Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Quat, 2, zyc::Layout::Vector);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, zyc::Layout::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, zyc::Layout::Native);

}// end zyc:: namespace

#include "Zomplex.ipp"

//end ZYC_HAS_ZOMPLEX_HPP
#endif
