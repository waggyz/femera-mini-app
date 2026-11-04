#ifndef FEMERA_HAS_GRID_ELEM_TEST_HPP
#define FEMERA_HAS_GRID_ELEM_TEST_HPP

#include "../../fmr/fmr.hpp"

namespace femera { namespace grid { namespace elem { //TODO move to test namespace?

struct Elem_args {
public:
  fmr::Local_int node_n;// N
  fmr::Local_int intp_n;// G
  fmr::Local_int sims_d;// D
};
template <class E, Elem_args const &A, typename F=fmr::Geom_float>
class Elem_test {
private:
  static constexpr fmr::Local_int node_n = A.node_n;
  static constexpr fmr::Local_int intp_n = A.intp_n;
  static constexpr fmr::Local_int sims_d = A.sims_d;
public:
  template <typename fmr::Local_int D=sims_d>
  constexpr typename std::enable_if <D==3, fmr::Local_int>::type
  test_vert_n () const noexcept {return E::vert_n;}
};

//} } }//end fmr::grid::elem:: namespace

//namespace femera { namespace grid { namespace elem { //TODO move to test namespace?

//TODO These are used for testing.
std::string tri2_norm_str // in 3D
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3);
std::string tri3_norm_str // in 3D
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3);

} } }//end femera::grid::elem:: namespace

//end FEMERA_HAS_GRID_ELEM_TEST_HPP
#endif
