#ifndef FEMERA_HAS_GRID_ELEM_BARS_IPP
#define FEMERA_HAS_GRID_ELEM_BARS_IPP

namespace femera { namespace grid { namespace elem {
#if 0
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
//constexpr fmr::Local_int Bars::tris_conn [];
//constexpr fmr::Local_int Bars::quad_conn [];

inline constexpr fmr::Local_int Bars::bars_conn [];
inline constexpr fmr::Geom_float Bars::vert_coor [];
inline constexpr fmr::Geom_float Bars::coor_vert [];

#endif
  //TODO FIXME only works for elem_p==1; generalize for higher orders
  inline constexpr
  bool get_tile_conn (fmr::Local_int* conn,//FIXME generalize & move to Elem.ipp
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size)
    noexcept{
    conn[0] = (tile_i + 0);// % part_size[0];
    conn[1] = (tile_i + 1);// % part_size[0];// periodic BC
    return tile_i < part_size[0];// false if tile_i out of range
  }
  inline constexpr
  bool get_tile_conn_prdc (fmr::Local_int* conn,//FIXME generalize & move to Elem.ipp
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size)
    noexcept{
    conn[0] = (tile_i + 0) % part_size[0];// periodic BC
    conn[1] = (tile_i + 1) % part_size[0];
    return tile_i < part_size[0];// false if tile_i out of range
  }
  inline constexpr
  bool get_tile_node_coor(fmr::Geom_float* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)
    noexcept{
    coor[0] = part_orig[0] + node_i * elem_size[0];
    coor[1] = part_orig[1] + node_i * elem_size[1];
    coor[2] = part_orig[2] + node_i * elem_size[2];
    return node_i < node_n;
  }
  constexpr bool get_tile_elem_vert_coor(//x1,y1,z1, x2,y2,z2
    fmr::Geom_float* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i,
    const fmr::Local_int elem_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v * 3 + 0] = part_orig[0] + (elem_i + v) * elem_size[0];
        vert_coor[v * 3 + 1] = part_orig[1] + (elem_i + v) * elem_size[1];
        vert_coor[v * 3 + 2] = part_orig[2] + (elem_i + v) * elem_size[2];
      }
      return elem_i < elem_n;
    }
  constexpr bool get_tile_elem_coor_vert(//x1,x2, y1,y2, z1, z2
    fmr::Geom_float* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i,
    const fmr::Local_int elem_n,
    const fmr::Geom_float* elem_size,
    const fmr::Geom_float* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v + 0] = part_orig[0] + (elem_i + v) * elem_size[0];
        vert_coor[v + 2] = part_orig[1] + (elem_i + v) * elem_size[1];
        vert_coor[v + 4] = part_orig[2] + (elem_i + v) * elem_size[2];
      }
      return elem_i < elem_n;
    }
#if 0
  inline
  bool Elem::get_tile_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int grid_x = 1,// handles grid_x not multiple of part_x
    const fmr::Local_int part_x = 1) noexcept{
    conn[0] = tile_i % grid_x % part_x;
    conn[1] = (tile_i + 1) % grid_x % part_x;// periodic BC
    return tile_i < grid_x;// false if tile_i out of range
  }
    // free surface BCs
  constexpr fmr::Local_int* Bars::get_tile_conn (fmr::Local_int tile_i,
    // local element index
    const fmr::Local_int grid_x = 1,
    const fmr::Local_int part_x = 1) noexcept{
    return new fmr::Local_int[2]{
      tile_i % grid_x % part_x,
      tile_i % grid_x % part_x + 1};// periodic BC
    }
#endif

//} } }//end femera::grid::elem namespace


//namespace femera { namespace grid {
// Bars::poly_func_X and Bars::poly_grad_N evaluate the shape functions or
// gradients at 3D (reduced to 1D) natural coordinate point x.
// Bars nonzero shape functions are only in the x-direction x[0].
// When edge nodes are at their natural locations, the Jacobian is constant,
// and only poly_func2 and poly_grad_2 are needed for constant Jacobian evaluation.

template <typename F> inline// single or double precision
F* Bars::poly_func_2
(F f[2u], const F x[1u]) {// x[1], x[2] not used for 1D bar element
  f[ 0] = 1.0-x[0];
  f[ 1] = x[0];
  return f;
}
template <typename F> inline
F* Bars::poly_grad_2
(F g[2u *3u], const F[]) {// x[3u] not used (constant gradient)
  g[ 0]=-1.0; g[ 1]=1.0;// dN/dx (natural coords)
  g[ 2]= 0.0; g[ 3]=0.0;// dN/dy
  g[ 4]= 0.0; g[ 5]=0.0;// dN/dz
  return g;
}
template <typename F> inline
F* Bars::poly_grat_2
(F g[2u *3u], const F[]) {// x[3u] not used (constant gradient)
  g[ 0]=-1.0; g[ 3]=1.0;// dN/dx (natural coords)
  g[ 1]= 0.0; g[ 4]=0.0;// dN/dy
  g[ 2]= 0.0; g[ 5]=0.0;// dN/dz
  return g;
}
template <typename F> inline
F* Bars::poly_func_3
(F f[3u], const F x[1u]) {
  const F L2=x[0];
  const F L1=1.0-L2;
  f[ 0] = 2.0*L1*L1 - L1;// Vertex nodes
  f[ 1] = 2.0*L2*L2 - L2;
  f[ 2] = 4.0*L1*L2;// Edge node
  return f;
}
template <typename F> inline
F* Bars::poly_grad_3
(F g[3u *3u], const F x[1u]) {
  const F L2=x[0];
  const F L1=1.0-L2;
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0;
  const F L1s=-1.0, L2s=0.0;
  const F L1t=-1.0, L2t=0.0;
  // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
  // r-derivs by product rule
  g[ 0] = 2.0*L1*L1r + 2.0*L1r*L1 - L1r;// Vertex nodes
  g[ 1] = 2.0*L2*L2r + 2.0*L2r*L2 - L2r;
  g[ 2] = 4.0*L2*L1r + 4.0*L2r*L1;// Edge node
  // s-derivs
  g[ 3] = 2.0*L1*L1s + 2.0*L1s*L1 - L1s;// Vertex nodes
  g[ 4] = 2.0*L2*L2s + 2.0*L2s*L2 - L2s;
  g[ 5] = 4.0*L2*L1s + 4.0*L2s*L1;// Edge node
  // t-derivs
  g[ 6] = 2.0*L1*L1t + 2.0*L1t*L1 - L1t;// Vertex nodes
  g[ 7] = 2.0*L2*L2t + 2.0*L2t*L2 - L2t;
  g[ 8] = 4.0*L2*L1t + 4.0*L2t*L1;// Edge node
  return g;
}
template <typename F> inline
F* Bars::poly_grat_3// transposed
(F g[3u *3u], const F x[1u]) {
  const F L2=x[0];
  const F L1=1.0-L2;
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0;
  const F L1s=-1.0, L2s=0.0;
  const F L1t=-1.0, L2t=0.0;
  // edges: 0,1; 1,2; 0,2; 0,3; 2,3; 1,3;
  // r-derivs by product rule
  g[ 0] = 2.0*L1*L1r + 2.0*L1r*L1 - L1r;// Vertex nodes
  g[ 3] = 2.0*L2*L2r + 2.0*L2r*L2 - L2r;
  g[ 6] = 4.0*L2*L1r + 4.0*L2r*L1;// Edge node
  // s-derivs
  g[ 1] = 2.0*L1*L1s + 2.0*L1s*L1 - L1s;// Vertex nodes
  g[ 4] = 2.0*L2*L2s + 2.0*L2s*L2 - L2s;
  g[ 7] = 4.0*L2*L1s + 4.0*L2s*L1;// Edge node
  // t-derivs
  g[ 2] = 2.0*L1*L1t + 2.0*L1t*L1 - L1t;// Vertex nodes
  g[ 5] = 2.0*L2*L2t + 2.0*L2t*L2 - L2t;
  g[ 8] = 4.0*L2*L1t + 4.0*L2t*L1;// Edge node
  return g;
}
template <typename F> inline
F* elem::Bars::poly_func_4
(F f[4u], const F x[1u]) {
  const F L2=x[0];
  const F L1=(1.0-L2);
  f[ 0]= 0.5* L1 *(3.0* L1 -1.0)*(3.0* L1 -2.0);// Vertex nodes;
  f[ 1]= 0.5* L2 *(3.0* L2 -1.0)*(3.0* L2 -2.0);
  // Edge nodes
  f[ 2]=4.5*( L1*L2 *(3.0*L1 -1.0) );
  f[ 3]=4.5*( L1*L2 *(3.0*L2 -1.0) );
  return f;
}
template <typename F> inline
F* elem::Bars::poly_grad_4
(F g[4u *3u], const F x[1u]) {
  const F L2=x[0];
  const F L1=(1.0-L2);
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0;
  const F L1s=-1.0, L2s=0.0;
  const F L1t=-1.0, L2t=0.0;
  // edge: 0,1;
  g[ 0]= 0.5* L1r *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1r   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1r   );
  g[ 1]= 0.5* L2r *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2r   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2r   );
  // r-derivs, edge nodes
  g[ 2]=4.5*( L1r*L2 *(3.*L1 -1.) + L1 *L2r*(3.*L1 -1.) + L1 *L2 *(3.*L1r) );
  g[ 3]=4.5*( L1r*L2 *(3.*L2 -1.) + L1 *L2r*(3.*L2 -1.) + L1 *L2 *(3.*L2r) );
  // s-derivs
  g[ 4]= 0.5* L1s *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1s   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1s   );
  g[ 5]= 0.5* L2s *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2s   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2s   );
  // s-derivs, edge nodes
  g[ 6]=4.5*( L1s*L2 *(3.*L1 -1.) + L1 *L2s*(3.*L1 -1.) + L1 *L2 *(3.*L1s) );
  g[ 7]=4.5*( L1s*L2 *(3.*L2 -1.) + L1 *L2s*(3.*L2 -1.) + L1 *L2 *(3.*L2s) );
  // t-derivs
  g[ 8]= 0.5* L1t *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1t   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1t   );
  g[ 9]= 0.5* L2t *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2t   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2t   );
  // t-derivs, edge nodes
  g[10]=4.5*( L1t*L2 *(3.*L1 -1.) + L1 *L2t*(3.*L1 -1.) + L1 *L2 *(3.*L1t) );
  g[11]=4.5*( L1t*L2 *(3.*L2 -1.) + L1 *L2t*(3.*L2 -1.) + L1 *L2 *(3.*L2t) );
  return g;
}
template <typename F> inline
F* elem::Bars::poly_grat_4
(F g[4u *3u], const F x[1u]) {
  const F L2=x[0];
  const F L1=(1.0-L2);
  // Term-by-term derivs
  const F L1r=-1.0, L2r=1.0;
  const F L1s=-1.0, L2s=0.0;
  const F L1t=-1.0, L2t=0.0;
  // edge: 0,1;
  g[ 0]= 0.5* L1r *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1r   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1r   );
  g[ 3]= 0.5* L2r *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2r   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2r   );
  // r-derivs, edge nodes
  g[ 6]=4.5*( L1r*L2 *(3.*L1 -1.) + L1 *L2r*(3.*L1 -1.) + L1 *L2 *(3.*L1r) );
  g[ 9]=4.5*( L1r*L2 *(3.*L2 -1.) + L1 *L2r*(3.*L2 -1.) + L1 *L2 *(3.*L2r) );
  // s-derivs
  g[ 1]= 0.5* L1s *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1s   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1s   );
  g[ 4]= 0.5* L2s *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2s   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2s   );
  // s-derivs, edge nodes
  g[ 7]=4.5*( L1s*L2 *(3.*L1 -1.) + L1 *L2s*(3.*L1 -1.) + L1 *L2 *(3.*L1s) );
  g[10]=4.5*( L1s*L2 *(3.*L2 -1.) + L1 *L2s*(3.*L2 -1.) + L1 *L2 *(3.*L2s) );
  // t-derivs
  g[ 2]= 0.5* L1t *(3.* L1 -1.)*(3.* L1 -2.)// Vertex nodes
       + 0.5* L1  *(3.* L1t   )*(3.* L1 -2.)
       + 0.5* L1  *(3.* L1 -1.)*(3.* L1t   );
  g[ 5]= 0.5* L2t *(3.* L2 -1.)*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2t   )*(3.* L2 -2.)
       + 0.5* L2  *(3.* L2 -1.)*(3.* L2t   );
  // t-derivs, edge nodes
  g[ 8]=4.5*( L1t*L2 *(3.*L1 -1.) + L1 *L2t*(3.*L1 -1.) + L1 *L2 *(3.*L1t) );
  g[11]=4.5*( L1t*L2 *(3.*L2 -1.) + L1 *L2t*(3.*L2 -1.) + L1 *L2 *(3.*L2t) );
  return g;
}

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_BARS_IPP
#endif
