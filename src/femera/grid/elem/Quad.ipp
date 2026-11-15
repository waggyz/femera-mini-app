#ifndef FEMERA_HAS_GRID_ELEM_QUAD_IPP
#define FEMERA_HAS_GRID_ELEM_QUAD_IPP

namespace femera { namespace grid { namespace elem {
namespace detail {
template <fmr::Dim_int P> struct poly_dispatch<Quad,P> {
  static_assert(P >= 1 && P <= 3, "polynomial dispatch supports orders 1 through 3");
  template <typename F> static constexpr void values(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_func_C2D4N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_func_C2D8N(out,x);
    else fmr::fems::elem::calc_poly_func_C2D12N(out,x);
  }
  template <typename F> static constexpr void node_grad(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_node_grad_C2D4N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_node_grad_C2D8N(out,x);
    else fmr::fems::elem::calc_poly_node_grad_C2D12N(out,x);
  }
  template <typename F> static constexpr void grad_node(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_grad_node_C2D4N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_grad_node_C2D8N(out,x);
    else fmr::fems::elem::calc_poly_grad_node_C2D12N(out,x);
  }
};
}
#if 0
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
//constexpr fmr::Local_int Quad::tris_vert_conn [];
constexpr fmr::Local_int Quad::quad_vert_conn [];
constexpr fmr::Local_int Quad::bars_vert_conn [];
constexpr fmr::Geom_float Quad::vert_coor [];
constexpr fmr::Geom_float Quad::coor_vert [];
#endif
} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_QUAD_IPP
#endif
