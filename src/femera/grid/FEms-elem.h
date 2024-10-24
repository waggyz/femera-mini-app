namespace femera { namespace grid { namespace FEMs {

//================================== pts =====================================
static constexpr fmr::Local_int vert_d      = 0;
static constexpr fmr::Local_int node_d      = 0;

//================================== bars ====================================
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
//================================== tris ====================================
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
//================================== quad ====================================
static constexpr fmr::Local_int quad_d      = 2;// tris spatial dimension
static constexpr fmr::Local_int quad_vert_n = 4;
static constexpr fmr::Local_int quad_edge_n = 4;
static constexpr
fmr::Local_int quad_vert_conn [quad_vert_n] = {0,1,2,4};
static constexpr
fmr::Local_int quad_vert_edge_bars [bars_vert_n * quad_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0
};
static constexpr fmr::Geom_float quad_meas  = 4.0;// natural element area
static constexpr
fmr::Geom_float quad_vert_coor [quad_vert_n * quad_d ]// transposed coor_vert
  = {
  -1.0,-1.0,
   1.0,-1.0,
   1.0, 1.0,
  -1.0, 1.0
};
static constexpr
fmr::Geom_float quad_coor_vert [quad_d * quad_vert_n]// transposed vert_coor
  = {
 -1.0, 1.0, 1.0,-1.0,
 -1.0,-1.0, 1.0, 1.0,
};
//================================== tets ====================================
static constexpr fmr::Local_int tets_d      = 3;// tets spatial dimension
static constexpr fmr::Local_int tets_vert_n = 4;
static constexpr fmr::Local_int tets_edge_n = 6;
static constexpr fmr::Local_int tets_face_n = 4;
static constexpr
  fmr::Local_int tets_vert_conn [tets_vert_n] = {0,1,2,3};
//
//NOTE alternate tets vertex numbering convention below.
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
//================================== brck ====================================
static constexpr fmr::Local_int brck_d      =  3;// spatial dimension
static constexpr fmr::Local_int brck_vert_n =  8;
static constexpr fmr::Local_int brck_edge_n = 12;
static constexpr fmr::Local_int brck_face_n =  6;
//
#if 0
static constexpr
//TODO brick elem edge and face conventions
fmr::Local_int brck_vert_edge_bars [bars_vert_n * brck_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0,
//  4,5, 5,6, 6,7, 7,0,
  0,7, 7,6, 6,5, 5,4,
  0,4, 1,5. 2,6, 3,7
};
static constexpr
fmr::Local_int tets_vert_face_quad [quad_vert_n * brck_face_n]
  = { 
  0,1,2,3,
  7,6,5,4,
  0,1,5,4,
  1,2,6,5,
  2,3,7,6,
  3,0,4,7
};
#endif
static constexpr fmr::Geom_float brck_meas  = 8.0;// natural elem volume
static constexpr
fmr::Geom_float brck_vert_coor [brck_vert_n * brck_d ]// transposed coor_vert
  = {
  -1.0,-1.0,-1.0,
   1.0,-1.0,-1.0,
   1.0, 1.0,-1.0,
  -1.0, 1.0,-1.0,
  //
  -1.0,-1.0, 1.0,
   1.0,-1.0, 1.0,
   1.0, 1.0, 1.0,
  -1.0, 1.0, 1.0
};
static constexpr
fmr::Geom_float brck_coor_vert [brck_d * brck_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0,  0.0, 1.0, 0.0,
  0.0, 0.0, 1.0,  0.0, 0.0, 1.0,
 -1.0,-1.0,-1.0,  1.0, 1.0, 1.0
};
//=========================== interface triangles ============================
// e.g., cohesive elements
static constexpr fmr::Local_int fac3_d      = 2;// spatial dimension
static constexpr fmr::Local_int fac3_vert_n = 6;
static constexpr fmr::Local_int fac3_edge_n = 6;
static constexpr fmr::Local_int fac3_face_n = 2;
static constexpr
  fmr::Local_int fac3_vert_conn [fac3_vert_n] = {0,1,2, 3,4,5};
//
static constexpr
fmr::Local_int fac3_vert_edge_bars [bars_vert_n * fac3_edge_n]
  = {
  0,1, 1,2, 2,0,
  3,4, 4,5, 5,3
};
static constexpr
fmr::Local_int fac3_vert_face_tris [tris_vert_n * fac3_face_n]
  = { 
  0,1,2, 3,4,5
};
static constexpr fmr::Geom_float fac3_meas  = 0.5;// ntrl elem area
static constexpr
fmr::Geom_float fac3_vert_coor [fac3_vert_n * fac3_d ]// transposed coor_vert
  = {
  0.0, 0.0,
  1.0, 0.0,
  0.0, 1.0,
  //
  0.0, 0.0,
  1.0, 0.0,
  0.0, 1.0
};
static constexpr
fmr::Geom_float fac3_coor_vert [fac3_d * fac3_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0,  0.0, 1.0, 0.0,
  0.0, 0.0, 1.0,  0.0, 0.0, 1.0
};
//=========================== interface quadangles ===========================
// e.g., cohesive elements
static constexpr fmr::Local_int fac4_d      = 2;// spatial dimension
static constexpr fmr::Local_int fac4_vert_n = 8;
static constexpr fmr::Local_int fac4_edge_n = 8;
static constexpr fmr::Local_int fac4_face_n = 2;
static constexpr
  fmr::Local_int fac4_vert_conn [fac4_vert_n] = {0,1,2,3, 4,5,6,7};
//
static constexpr
fmr::Local_int fac4_vert_edge_bars [bars_vert_n * fac4_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4
};
static constexpr
fmr::Local_int fac4_vert_face_quad [quad_vert_n * fac4_face_n]
  = { 
  0,1,2,3, 4,5,6,7
};
static constexpr fmr::Geom_float fac4_meas  = 4.0;// natural element area
static constexpr
fmr::Geom_float fac4_vert_coor [fac4_vert_n * fac4_d ]// transposed coor_vert
  = {
  -1.0,-1.0,
   1.0,-1.0,
   1.0, 1.0,
  -1.0, 1.0,
  //
  -1.0,-1.0,
   1.0,-1.0,
   1.0, 1.0,
  -1.0, 1.0
};
static constexpr
fmr::Geom_float fac4_coor_vert [fac4_d * fac4_vert_n]// transposed vert_coor
  = {
 -1.0, 1.0, 1.0,-1.0,  -1.0, 1.0, 1.0,-1.0,
 -1.0,-1.0, 1.0, 1.0,  -1.0,-1.0, 1.0, 1.0
};

//static constexpr fmr::Geom_float tet4_node_coor [12] = tets_vert_coor;
//static constexpr fmr::Geom_float tet4_coor_node [12] = tets_coor_vert;

} } }//end femera::grid::FEMs:: namespace
