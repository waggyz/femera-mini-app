#ifndef ZYC_HAS_ZOMPLEX_HPP
#define ZYC_HAS_ZOMPLEX_HPP

#include "zyclops.hpp"
#include "data.hpp"

namespace zyc {

  class Zomplex {
  private:
    Algebra           family = Algebra::Real;
    Zorder_int         order = 0;
    zyc::data::Layout layout = zyc::data::Layout::Native;
    bool        is_transpose = false;
    bool           has_upper = false;// triangular or full matrix storage
    bool           has_lower = false;// triangular or full matrix storage
    bool           has_zeros = true; // false: sparse (zero terms removed)
  public:
    bool       is_cr_matrix (){ return this->has_upper || this->has_lower; }
    bool       is_cr_upper  (){ return this->has_upper && ! this->has_lower; }
    bool       is_cr_lower  (){ return ! this->has_upper && this->has_lower; }
    bool       is_cr_full   (){ return this->has_upper && this->has_lower; }
    bool       is_vector    (){ return ! this->is_cr_matrix (); }
    bool       is_conjugate (){ return this->is_transpose; }
    bool       is_sparse    (){ return ! has_zeros; }
    Algebra    get_family        (){ return this->family; }
    Zorder_int get_order         (){ return this->order; }
    zyc::data::Layout get_layout (){ return this->layout; }
  public:
    Zomplex () {}
    Zomplex (Algebra z, Zorder_int p) : family (z), order(p) {
      if (p!=0) {layout = zyc::data::Layout::Block;}
    }
    Zomplex (Algebra z, Zorder_int p, zyc::data::Layout l) :
      family (z), order(p), layout(l) {}
  };
  const static Zomplex Real
    = Zomplex (Algebra::Real, 0, zyc::data::Layout::Native);
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, zyc::data::Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Quat, 2, zyc::data::Layout::Vector);
  //
  const static Zomplex Integer
    = Zomplex (Algebra::Int, 0, zyc::data::Layout::Native);
  const static Zomplex Natural
    = Zomplex (Algebra::Nat, 0, zyc::data::Layout::Native);

}// end zyc:: namespace

//end ZYC_HAS_ZOMPLEX_HPP
#endif