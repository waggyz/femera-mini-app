#include "../core.h"
#include "FEms.hpp"

namespace femera {


#if 0
//TODO Move quallity tolerances and checks elsewhere?
  private:
  // Element quality criteria: warn if bad.
  F dets_qual_min = F(0.2);// min (det) / abs (max (det)) at vert_coor
  // bars,tris,tets: const det > std::numeric_limits<F>::min() [* part_size??]
  //                          OR std::numeric_limits<F>::epsilon()
  //NOTE these are all vert_n == elem_d + 1 (when side nodes at natural positions)
  F edge_qual_min = F(0.2);// shortest / longest edge
  F skew_qual_min = F(0.6);// = 1.0-skewness [0-1] (quad, cube)
  F delt_qual_min// ratio of min edge length / max coord absolute value
  = F(std::numeric_limits<F>::epsilon ());
  //
  public:
  // Simple get / set / compare ----------------------------------------------
  //
  F get_dets_tol () const noexcept {return dets_qual_min;}
  F get_edge_tol () const noexcept {return edge_qual_min;}
  F get_skew_tol () const noexcept {return skew_qual_min;}
 //
  F set_dets_tol (const F val) noexcept {return dets_qual_min = F(val);}
  F set_edge_tol (const F val) noexcept {return edge_qual_min = F(val);}
  F set_skew_tol (const F val) noexcept {return skew_qual_min = F(val);}
  //
  bool is_dets_qual_ok (const F val) const noexcept {return val >= dets_qual_min;}
  bool is_edge_qual_ok (const F val) const noexcept {return val >= edge_qual_min;}
  bool is_skew_qual_ok (const F val) const noexcept {return val >= skew_qual_min;}
#endif

}//end femera namespace
