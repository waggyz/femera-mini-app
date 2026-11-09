#ifndef FMR_HAS_FEMS_ELEM_IPP
#define FMR_HAS_FEMS_ELEM_IPP

  // Element Jacobian and determinant calculations
  template <typename F>
  inline void fmr::fems::calc_jacs_csoa_ref (F* jacs,
    const F* coor_node,// x1,x2,..., y1,y2,..., z1,z2,...
    const F* poly_grat,// transposed gradients at integration points
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d) noexcept {
      /* In general, Jacobians are D x D matrices at each integration point.
         The jacs array should be filled with the Jacobian matrices
         for each integration point.
         Each Jacobians is a matrix-matrix product of the first-order
         shape function gradients and the element node coordinates.
      */
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<sims_d; ++i){// can permute i & j loops
         for(fmr::Local_int j=0; j<sims_d; ++j){// this increments innermost
           jacs[ip*sims_d*sims_d + i*sims_d + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){// Longest dim innermost!
             jacs[ip*sims_d*sims_d + i*sims_d + j] += coor_node[i*node_n + k]
               * poly_grat[ip*node_n*sims_d + k*sims_d + j];
    } } } }
  }
  template <typename F>
  inline void fmr::fems::calc_jacs_caos_ref (F* jacs,
    const F* vert_node,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grat,// transposed gradients at integration points
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d) noexcept {
      //NOTE uses array of structures (AoS) and poly_grad (not transposed).
      /* In general, Jacobians are D x D matrices at each integration point.
         The jacs array should be filled with the Jacobian matrices
         for each integration point.
         Each Jacobians is a matrix-matrix product of the shape function gradients
         and the node coordinates.
      */
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<sims_d; ++i){// this increments innermost
         for(fmr::Local_int j=0; j<sims_d; ++j){// this increments innermost
           jacs[ip*sims_d*sims_d + i*sims_d + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){
             jacs[ip*sims_d*sims_d + i*sims_d + j] += vert_node[k*sims_d + i]
               * poly_grat[ip*node_n*sims_d + k*sims_d + j];
    } } } }
  }
  template <typename F>
  inline void fmr::fems::calc_jacs_caos_a (F* jacs,
    const F* vert_node,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grad,// per integration point: dimension-major, then vertex
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n,
    const fmr::Local_int sims_d) noexcept {
    //NOTE likely to ouperform calc_jacs_caos_ref when tested,
    //     especially for higher order 3D elements (large node count).
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<sims_d; ++i){// this increments innermost
         for(fmr::Local_int j=0; j<sims_d; ++j){// can permute i & j loops
           jacs[ip*sims_d*sims_d + i*sims_d + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){// Longest dim innermost!
             jacs[ip*sims_d*sims_d + i*sims_d + j] += vert_node[k*sims_d + i]
               * poly_grad[ip*node_n*sims_d + j*node_n + k];
    } } } }
  }


#endif
//end FMR_HAS_FEMS_ELEM_IPP

