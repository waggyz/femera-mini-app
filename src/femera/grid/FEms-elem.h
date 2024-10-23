namespace femera { namespace grid { namespace FEMs {

//================================== pts ======================================
static constexpr fmr::Local_int vert_d      = 0;
static constexpr fmr::Local_int node_d      = 0;

//================================== bars =====================================
static constexpr fmr::Local_int bars_d      = 1;// bars spatial dimension
static constexpr fmr::Local_int bars_vert_n = 2;
static constexpr
fmr::Local_int  bars_vert_conn [bars_vert_n] = {0,1};
//
static constexpr fmr::Geom_float bars_meas  = 2.0;// natural element length
static constexpr
fmr::Geom_float bars_vert_coor [bars_vert_n * bars_d ]// transposed coor_vert
  = {
 -1.0,
  1.0
};
//================================== tris =====================================
static constexpr fmr::Local_int tris_d      = 2;// tris spatial dimension
static constexpr fmr::Local_int tris_vert_n = 3;
static constexpr fmr::Local_int tris_edge_n = 3;
static constexpr
fmr::Local_int tris_vert_conn [tris_vert_n] = {0,1,2};
static constexpr
fmr::Local_int tris_vert_edge_bars [bars_vert_n * tris_edge_n]
= {
  0,1, 1,2, 2,0
};
static constexpr fmr::Geom_float tris_meas  = 0.5;// natural element area
static constexpr
fmr::Geom_float tris_vert_coor [tris_vert_n * tris_d ]// transposed coor_vert
  = {
  0.0, 0.0,
  1.0, 0.0,
  0.0, 1.0
};
//================================== tets =====================================
static constexpr fmr::Local_int tets_d      = 3;// tets spatial dimension
static constexpr fmr::Local_int tets_vert_n = 4;
static constexpr fmr::Local_int tets_edge_n = 6;
static constexpr fmr::Local_int tets_face_n = 4;
static constexpr
  fmr::Local_int tets_vert_conn [tets_vert_n] = {0,1,2,3};
//
//TODO alternate tets vertex numbering convention below.
//const RESTRICT Mesh::ints vert_edge={ 0,1, 1,2, 2,0, 0,3, 1,3, 2,3 };
//const RESTRICT Mesh::ints vert_face={ 0,1,2, 0,1,3, 1,2,3, 2,0,3 }
//
//NOTE The following match the gmsh convention for tets.
static constexpr
fmr::Local_int tets_vert_edge_bars [bars_vert_n * tets_edge_n]
= {
  0,1, 1,2, 2,0, 0,3, 2,3, 1,3
};
static constexpr
fmr::Local_int tets_vert_face_tris [tris_vert_n * tets_face_n]
= { 
  0,1,2, 0,1,3, 0,3,2, 1,2,3
};
static constexpr fmr::Geom_float tets_meas  = 1.0 / 6.0;// ntrl elem volume
static constexpr
fmr::Geom_float tets_vert_coor [tets_vert_n * tets_d ]// transposed coor_vert
  = {
  0.0, 0.0, 0.0,
  1.0, 0.0, 0.0,
  0.0, 1.0, 0.0,
  0.0, 0.0, 1.0
};
static constexpr
fmr::Geom_float tets_coor_vert [tets_d * tets_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
};
//static constexpr fmr::Geom_float tet4_node_coor [12] = tets_vert_coor;
//static constexpr fmr::Geom_float tet4_coor_node [12] = tets_coor_vert;

} } }//end femera::grid::FEMs:: namespace
