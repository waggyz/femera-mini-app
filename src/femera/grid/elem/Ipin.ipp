#ifndef FEMERA_HAS_GRID_ELEM_IPIN_IPP
#define FEMERA_HAS_GRID_ELEM_IPIN_IPP

namespace femera { namespace grid { namespace elem {

namespace detail {
template <> struct poly_dispatch<Ipin,1> {
	template <typename F> static constexpr void values(F* out, const F* x) {
		fmr::fems::elem::calc_poly_func_C1D2N(out,x);
	}
	template <typename F> static constexpr void node_grad(F* out, const F* x) {
		fmr::fems::elem::calc_poly_node_grad_C1D2N(out,x);
	}
	template <typename F> static constexpr void grad_node(F* out, const F* x) {
		fmr::fems::elem::calc_poly_grad_node_C1D2N(out,x);
	}
};
}

template <typename F>
constexpr F Ipin::vert_coor[];

template <typename F>
constexpr F Ipin::coor_vert[];


} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_IPIN_IPP
#endif
