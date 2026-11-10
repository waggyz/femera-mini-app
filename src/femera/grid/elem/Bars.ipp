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
constexpr bool Bars::calc_tile_vert_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int p)
    noexcept {
    conn[0] = p * tile_i;
    conn[1] = p * (tile_i + 1);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bars::calc_tile_vert_conn_prdc (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int p)
    noexcept{
    if (part_size[0] == 0) return false;//TODO remove this check?
    // part_size counts elements; each element spans p node indices.
    conn[0] = p * (tile_i % part_size[0]);
    conn[1] = p * ((tile_i + 1) % part_size[0]);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bars::calc_tile_node_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int elem_p)
    noexcept {
    conn[0] = elem_p * tile_i;
    conn[1] = elem_p * (tile_i + 1);
    if (elem_p > 1) {
      for (fmr::Local_int j = 1; j < elem_p; ++j) {
        conn[j + 1] = elem_p * tile_i + j;
      }
    }
    return tile_i < part_size[0];// false if tile_i out of range
  }
#if 0
constexpr bool Bars::calc_tile_node_coor(fmr::Geom_float* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* tile_size,
    const fmr::Geom_float* part_orig)
    noexcept{
    coor[0] = part_orig[0] + node_i * tile_size[0];
    coor[1] = part_orig[1] + node_i * tile_size[1];
    coor[2] = part_orig[2] + node_i * tile_size[2];
    //TODO FIXME this only works for P=1
    return node_i < node_n;
  }
#endif
  template <typename F>
  constexpr bool Bars::calc_tile_vert_coor(F* coor,
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig)
    noexcept{
    coor[0] = part_orig[0] + vert_i * tile_size[0];
    coor[1] = part_orig[1] + vert_i * tile_size[1];
    coor[2] = part_orig[2] + vert_i * tile_size[2];
    return vert_i < vert_n;
  }
  template <typename F>
constexpr bool Bars::calc_tile_elem_vert_coor(//x1,y1,z1, x2,y2,z2
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v * 3 + 0] = part_orig[0] + (elem_i + v) * tile_size[0];
        vert_coor[v * 3 + 1] = part_orig[1] + (elem_i + v) * tile_size[1];
        vert_coor[v * 3 + 2] = part_orig[2] + (elem_i + v) * tile_size[2];
      }
      return elem_i < elem_n;
    }
  template <typename F>
  constexpr bool Bars::calc_tile_elem_coor_vert(//x1,x2, y1,y2, z1, z2
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v + 0] = part_orig[0] + (elem_i + v) * tile_size[0];
        vert_coor[v + 2] = part_orig[1] + (elem_i + v) * tile_size[1];
        vert_coor[v + 4] = part_orig[2] + (elem_i + v) * tile_size[2];
      }
      return elem_i < elem_n;
    }
#if 0
  inline
  bool Elem::get_tile_vert_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int grid_x = 1,// handles grid_x not multiple of part_x
    const fmr::Local_int part_x = 1) noexcept{
    conn[0] = tile_i % grid_x % part_x;
    conn[1] = (tile_i + 1) % grid_x % part_x;// periodic BC
    return tile_i < grid_x;// false if tile_i out of range
  }
    // free surface BCs
  constexpr fmr::Local_int* Bars::get_tile_vert_conn (fmr::Local_int tile_i,
    // local element index
    const fmr::Local_int grid_x = 1,
    const fmr::Local_int part_x = 1) noexcept{
    return new fmr::Local_int[2]{
      tile_i % grid_x % part_x,
      tile_i % grid_x % part_x + 1};// periodic BC
    }
#endif

//namespace femera { namespace grid {
// Bars::calc_poly_func_pX and Bars::calc_poly_grad_pX evaluate the shape functions or
// gradients at 3D natural coordinate point x.
// Bars nonzero shape functions are only in the x-direction x[0].//TODO correct?
// When edge nodes are at their natural locations, the Jacobian is constant,
// and only calc_poly_func_p1 and calc_poly_grad_p1 are needed for constant Jacobian evaluation.

template <typename Y, typename G> inline
//TODO mixed precision support for single or double precision
Y* Bars::calc_poly_func_p1
(Y f[2u], const G x[3u]) {// x[1], x[2] not used for 1D bar element?
  f[ 0] = 1.0-x[0];
  f[ 1] = x[0];
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* Bars::calc_poly_grad_p1
(Y g[2u *3u], const G[]) {// x[3u] not used (constant gradient)
#if 1
  g[ 0]=-1.0; g[ 1]=1.0;// dN/dx (natural coords)
  g[ 2]= 0.0; g[ 3]=0.0;// dN/dy
  g[ 4]= 0.0; g[ 5]=0.0;// dN/dz
#else
  g[ 0]=-1.0; g[ 1]=1.0;// dN/dx (natural coords)
  g[ 2]=-1.0; g[ 3]=1.0;// dN/dy
  g[ 4]=-1.0; g[ 5]=1.0;// dN/dz
#endif
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* Bars::calc_poly_grat_p1
(Y g[2u *3u], const G[]) {// x[3u] not used (constant gradient)
#if 1
  g[ 0]=-1.0; g[ 3]=1.0;// dN/dx (natural coords)
  g[ 1]= 0.0; g[ 4]=0.0;// dN/dy
  g[ 2]= 0.0; g[ 5]=0.0;// dN/dz
#else
  g[ 0]=-1.0; g[ 3]=1.0;// dN/dx (natural coords)
  g[ 1]=-1.0; g[ 4]=1.0;// dN/dy
  g[ 2]=-1.0; g[ 5]=1.0;// dN/dz
#endif
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* Bars::calc_poly_func_p2
(Y f[3u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=1.0-L2;
  f[ 0] = 2.0*L1*L1 - L1;// Vertex nodes
  f[ 1] = 2.0*L2*L2 - L2;
  f[ 2] = 4.0*L1*L2;// Edge node
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* Bars::calc_poly_grad_p2
(Y g[3u *3u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=1.0-L2;
  // Term-by-term derivs
  const Y L1r=-1.0, L2r=1.0;
  const Y L1s=-1.0, L2s=0.0;
  const Y L1t=-1.0, L2t=0.0;
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
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* Bars::calc_poly_grat_p2// transposed
(Y g[3u *3u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=1.0-L2;
  // Term-by-term derivs
  const Y L1r=-1.0, L2r=1.0;
  const Y L1s=-1.0, L2s=0.0;
  const Y L1t=-1.0, L2t=0.0;
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
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* elem::Bars::calc_poly_func_p3
(Y f[4u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=(1.0-L2);
  f[ 0]= 0.5* L1 *(3.0* L1 -1.0)*(3.0* L1 -2.0);// Vertex nodes;
  f[ 1]= 0.5* L2 *(3.0* L2 -1.0)*(3.0* L2 -2.0);
  // Edge nodes
  f[ 2]=4.5*( L1*L2 *(3.0*L1 -1.0) );
  f[ 3]=4.5*( L1*L2 *(3.0*L2 -1.0) );
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* elem::Bars::calc_poly_grad_p3
(Y g[4u *3u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=(1.0-L2);
  // Term-by-term derivs
  const Y L1r=-1.0, L2r=1.0;
  const Y L1s=-1.0, L2s=0.0;
  const Y L1t=-1.0, L2t=0.0;
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
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> inline
Y* elem::Bars::calc_poly_grat_p3
(Y g[4u *3u], const G x[3u]) {
  const Y L2=x[0];
  const Y L1=(1.0-L2);
  // Term-by-term derivs
  const Y L1r=-1.0, L2r=1.0;
  const Y L1s=-1.0, L2s=0.0;
  const Y L1t=-1.0, L2t=0.0;
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
