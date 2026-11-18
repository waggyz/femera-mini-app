#ifndef FEMERA_HAS_GRID_ELEM_BARS_IPP
#define FEMERA_HAS_GRID_ELEM_BARS_IPP

namespace femera { namespace grid { namespace elem {
//namespace femera { namespace grid {

namespace detail {
template <fmr::Dim_int P> struct poly_dispatch<Bars,P> {
  static_assert(P >= 1 && P <= 3, "polynomial dispatch supports orders 1 through 3");
  template <typename F> static constexpr void values(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_func_C1D2N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_func_C1D3N(out,x);
    else fmr::fems::elem::calc_poly_func_C1D4N(out,x);
  }
  template <typename F> static constexpr void node_grad(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_node_grad_C1D2N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_node_grad_C1D3N(out,x);
    else fmr::fems::elem::calc_poly_node_grad_C1D4N(out,x);
  }
  template <typename F> static constexpr void grad_node(F* out, const F* x) {
    if (P==1) fmr::fems::elem::calc_poly_grad_node_C1D2N(out,x);
    else if (P==2) fmr::fems::elem::calc_poly_grad_node_C1D3N(out,x);
    else fmr::fems::elem::calc_poly_grad_node_C1D4N(out,x);
  }
};
}//end namespace detail

template <typename F>
constexpr F Bars::vert_coor[];

template <typename F>
constexpr F Bars::coor_vert[];

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_BARS_IPP
#endif
