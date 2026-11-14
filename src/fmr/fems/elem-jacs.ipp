#ifndef FMR_HAS_FEMS_JACS_IPP
#define FMR_HAS_FEMS_JACS_IPP

namespace fmr { namespace fems {

// Element Jacobian calculations.
template <typename F>
inline void elem::elem_jacs_coor_node_ref (F* jacs,
const F* coor_node,// x1,x2,..., y1,y2,..., z1,z2,...
const F* poly_grad,// [ip][node][reference-coordinate]
const fmr::Local_int intp_n,
const fmr::Local_int node_n,
const fmr::Dim_int sims_d,
const fmr::Dim_int elem_d) noexcept {
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
    for(fmr::Dim_int i=0; i<sims_d; ++i){
        for(fmr::Dim_int j=0; j<elem_d; ++j){
        jacs[ip*sims_d*elem_d + i*elem_d + j] = 0;
        for(fmr::Local_int k=0; k<node_n; ++k){// Longest dim innermost!
            jacs[ip*sims_d*elem_d + i*elem_d + j] += coor_node[i*node_n + k]
            * poly_grad[ip*node_n*elem_d + k*elem_d + j];
    } } } }
}
template <typename F>
inline void elem::elem_jacs_node_coor_ref (F* jacs,
const F* vert_node,// x1,y1,z1, x2,y2,z2,...
const F* poly_grad,// [ip][node][reference-coordinate]
const fmr::Local_int intp_n,
const fmr::Local_int node_n,
const fmr::Dim_int sims_d,
const fmr::Dim_int elem_d) noexcept {
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
    for(fmr::Dim_int i=0; i<sims_d; ++i){
        for(fmr::Dim_int j=0; j<elem_d; ++j){
        jacs[ip*sims_d*elem_d + i*elem_d + j] = 0;
        for(fmr::Local_int k=0; k<node_n; ++k){
            jacs[ip*sims_d*elem_d + i*elem_d + j] += vert_node[k*sims_d + i]
            * poly_grad[ip*node_n*elem_d + k*elem_d + j];
    } } } }
}
template <typename F>
inline void elem::elem_jacs_node_coor_a (F* jacs,
const F* vert_node,// x1,y1,z1, x2,y2,z2,...
const F* poly_grad,// [ip][reference-coordinate][node]
const fmr::Local_int intp_n,
const fmr::Local_int node_n,
const fmr::Dim_int sims_d,
const fmr::Dim_int elem_d) noexcept {
//NOTE likely to ouperform elem_jacs_node_coor_ref when tested,
//     especially for higher order 3D elements (large node count).
    for(fmr::Local_int ip=0; ip<intp_n; ++ip){
    for(fmr::Dim_int i=0; i<sims_d; ++i){
        for(fmr::Dim_int j=0; j<elem_d; ++j){
        jacs[ip*sims_d*elem_d + i*elem_d + j] = 0;
        for(fmr::Local_int k=0; k<node_n; ++k){// Longest dim innermost!
            jacs[ip*sims_d*elem_d + i*elem_d + j] += vert_node[k*sims_d + i]
            * poly_grad[ip*elem_d*node_n + j*node_n + k];
    } } } }
}

} }// end fmr::fems namespace

#endif
//end FMR_HAS_FEMS_ELEM_IPP
