#ifndef ZYC_HAS_ZOMPLEX_HPP
#define ZYC_HAS_ZOMPLEX_HPP

#include "zyc.hpp"

namespace zyclops {

  struct Zomplex {// hypercomplex number/array info
  public:
    Algebra    get_family () noexcept;
    Zorder_int get_order  () noexcept;
    Layout     get_layout () noexcept;
    Zindex_int  hc_size   () noexcept;
    Zindex_int  cr_size   () noexcept;
    Zindex_int  cr_nnz    () noexcept;
  public:
    Zomplex () noexcept=default;
    Zomplex (Algebra, Zorder_int, Layout) noexcept;
  private:
    Zorder_int order = 0;// default is real number
    Algebra   family = Algebra::Real;
    Layout    layout = Layout::Native;/// only applies to hypercomplex arrays
  };
  const static Zomplex Real    = Zomplex (Algebra::Real,0, Layout::Native);
  const static Zomplex Integer = Zomplex (Algebra::Integer, 0, Layout::Native);
  const static Zomplex Natural = Zomplex (Algebra::Natural, 0, Layout::Native);
#if 0
  const static Zomplex Native_complex
    = Zomplex (Algebra::Complex, 1, Layout::Native);
  const static Zomplex Unit_quaternion
    = Zomplex (Algebra::Qcda, 2, Layout::Inset);
#endif

}// end zyclops:: namespace

#include "Zomplex.ipp"

//end ZYC_HAS_ZOMPLEX_HPP
#endif
