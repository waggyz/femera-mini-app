namespace femera { namespace grid { namespace fems {

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
      // vert
 -1.0,// 0    0---1
  1.0 // 1             o--x
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
           // vert    2
  0.0, 0.0,// 0       |\      y
  1.0, 0.0,// 1       | \     |
  0.0, 1.0 // 2       0--1    o--x
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
            // vert
  -1.0,-1.0,// 0    3----2
   1.0,-1.0,// 1    |    |    y
   1.0, 1.0,// 2    |    |    |
  -1.0, 1.0 // 3    0----1    o--x
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
static constexpr fmr::Geom_float tets_face_meas  = 2.0;// elem surface area
static constexpr
fmr::Geom_float tets_vert_coor [tets_vert_n * tets_d ]// transposed coor_vert
  = {            //          3              //
                 // vert    /|\             //
  0.0, 0.0, 0.0, // 0      / | \            //
  1.0, 0.0, 0.0, // 1     2--|--1           //
  0.0, 1.0, 0.0, // 2      \ | /     y z x  //
  0.0, 0.0, 1.0  // 3       \|/       \|/   //
};               //          0         o    //
static constexpr
fmr::Geom_float tets_coor_vert [tets_d * tets_vert_n]// transposed vert_coor
  = {
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
};
//-------------------------- tet shape functions -----------------------------


//---------------------------- tet integration -------------------------------
  // Gauss-Legendre tetrahedral integration points and weights
  // For linear perturbations (edge nodes are interpolated),
  // the Jacobian is constant and independent of the int pt locations.
  // So only ONE 3x3+1 (Jacobian+det) is needed for each element
  // regardless of tet element order.
  // Volume of natural tet is 1/6.
  //
  static constexpr fmr::Local_int tets_intg_1_n = 1;// Preferred P1
  static constexpr
  fmr::Phys_float tets_intg_1_ptwt [4* tets_intg_1_n] = {
    0.25,0.25,0.25, 1.0/6.0
  };
  static constexpr fmr::Local_int tets_intg_4_n = 4;// Preferred P2
  static constexpr fmr::Phys_float a2 = 0.5854101966249685;
    // a2 = (5.0+3.0*std::sqrt(5.))/20.0;
  static constexpr fmr::Phys_float b2 = 0.1381966011250105;
    // b2 = (5.0-std::sqrt(5.))/20.0;
  static constexpr
  fmr::Phys_float tets_intg_4_ptwt [4* tets_intg_4_n] = {
    a2,b2,b2, 0.25/6.0,
    b2,a2,b2, 0.25/6.0,
    b2,b2,a2, 0.25/6.0,
    b2,b2,b2, 0.25/6.0
  };
  static constexpr fmr::Local_int tets_intg_5_n = 5;
  //NOTE tet20s don't converge with 5-point rule
  //NOTE Triple-checked these 5-point rule values
  static constexpr
  fmr::Phys_float tets_intg_5_ptwt [4* tets_intg_5_n] = {
    0.25   , 0.25   , 0.25   ,-4.0/ 30.0,
    0.5    , 1.0/6.0, 1.0/6.0, 9.0/120.0,
    1.0/6.0, 0.5    , 1.0/6.0, 9.0/120.0,
    1.0/6.0, 1.0/6.0, 0.5    , 9.0/120.0,
    1.0/6.0, 1.0/6.0, 1.0/6.0, 9.0/120.0
  };
  static constexpr fmr::Local_int tets_intg_10_n = 10;//Preferred B3
    // From Lee Shunn, Frank Ham, Symmetric quadrature rules for tetrahedra
    // based on a cubic close-packed lattice arrangement, 2012
    static constexpr
    fmr::Phys_float
      w0=0.0476331348432089/6.0, w1=0.1349112434378610/6.0,
      a0=0.0738349017262234    , a1=0.7784952948213300,
      b0=0.0937556561159491    , b1=0.4062443438840510;
    static constexpr
    fmr::Phys_float tets_intg_10_ptwt [4* tets_intg_10_n] = {
      a1,a0,a0, w0,
      a0,a1,a0, w0,
      a0,a0,a1, w0,
      a0,a0,a0, w0,
      b1,b1,b0, w1,
      b1,b0,b1, w1,
      b1,b0,b0, w1,
      b0,b1,b1, w1,
      b0,b1,b0, w1,
      b0,b0,b1, w1
    };
  static constexpr fmr::Local_int tets_intg_11_n = 11;//OLD P3
  //NOTE This converges tet20 meshes
  static constexpr fmr::Phys_float a3=0.3994035761667992;
  // a3 = (1.+std::sqrt(5./14.))/4.;
  static constexpr fmr::Phys_float b3=0.1005964238332008;
  // b3 = (1.-std::sqrt(5./14.))/4.;
  static constexpr
  fmr::Phys_float tets_intg_11_ptwt [4* tets_intg_11_n] = {
      0.25  , 0.25  , 0.25  , -74.0/ 5625.0,
     11.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
      1.0/14.0,11.0/14.0, 1.0/14.0, 343.0/45000.0,
      1.0/14.0, 1.0/14.0,11.0/14.0, 343.0/45000.0,
      1.0/14.0, 1.0/14.0, 1.0/14.0, 343.0/45000.0,
      a3,a3,b3, 56.0/2250.0,
      a3,b3,a3, 56.0/2250.0,
      a3,b3,b3, 56.0/2250.0,
      b3,a3,a3, 56.0/2250.0,
      b3,a3,b3, 56.0/2250.0,
      b3,b3,a3, 56.0/2250.0
  };
//================================== brck ====================================
static constexpr fmr::Local_int brck_d      =  3;// spatial dimension
static constexpr fmr::Local_int brck_vert_n =  8;
static constexpr fmr::Local_int brck_edge_n = 12;
static constexpr fmr::Local_int brck_face_n =  6;
//
static constexpr
//TODO brick elem edge and face conventions
fmr::Local_int brck_vert_edge_bars [bars_vert_n * brck_edge_n]
  = {
  0,1, 1,2, 2,3, 3,0,
  4,5, 5,6, 6,7, 7,4,
  0,4, 1,5, 2,6, 3,7
};
static constexpr
fmr::Local_int brck_vert_face_quad [quad_vert_n * brck_face_n]
  = { 
  0,1,2,3,
  7,6,5,4,
  0,4,5,1,
  1,5,6,2,
  2,6,7,3,
  3,7,4,0
};
static constexpr fmr::Geom_float brck_meas       =  8.0;// natural elem volume
static constexpr fmr::Geom_float brck_face_meas  = 24.0;// elem surface area
static constexpr
fmr::Geom_float brck_vert_coor [brck_vert_n * brck_d ]// transposed coor_vert
  = {
                  // vert     7---------6
  -1.0,-1.0,-1.0, // 0       /|        /|
   1.0,-1.0,-1.0, // 1      / |       / |
   1.0, 1.0,-1.0, // 2     /  |      /  |
  -1.0, 1.0,-1.0, // 3    4---------5   |
                  //      |   |     |   |
  -1.0,-1.0, 1.0, // 4    |   3-----|---2
   1.0,-1.0, 1.0, // 5    |  /      |  /      z  y
   1.0, 1.0, 1.0, // 6    | /       | /       | /
  -1.0, 1.0, 1.0  // 7    |/        |/        |/
};                //      0---------1         o--x
static constexpr
fmr::Geom_float brck_coor_vert [brck_d * brck_vert_n]// transposed vert_coor
  = {
 -1.0, 1.0, 1.0,-1.0, -1.0, 1.0, 1.0,-1.0,
 -1.0,-1.0, 1.0, 1.0, -1.0,-1.0, 1.0, 1.0,
 -1.0,-1.0,-1.0,-1.0,  1.0, 1.0, 1.0, 1.0
};
#if 0
  const Mesh::ints vert_conn={ 0,1,2,3, 4,5,6,7 };
  const Mesh::ints vert_edge// Bar2 connectivity
    ={ 0,1, 1,2, 2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7 };
  const Mesh::ints vert_face
    ={ 0,1,2,3, 7,6,5,4, 0,4,5,1, 2,3,7,6, 0,3,7,4, 1,5,6,2 };
  const Mesh::vals node_coor={
                   //       7---------6
     0.0, 0.0, 0.0,//      /|        /|
     1.0, 0.0, 0.0,//     / |       / |
     1.0, 1.0, 0.0,//    /  |      /  |
     0.0, 1.0, 0.0,//   4---------5   |
     0.0, 0.0, 1.0,//   |   |     |   |
     1.0, 0.0, 1.0,//   |   3-----|---2
     1.0, 1.0, 1.0,//   |  /      |  /
     0.0, 1.0, 1.0 //   | /       | /
                   //   |/        |/
  };               //   0---------1
#endif
//=========================== interface triangle =============================
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
  0,1,2, 5,4,3
};
static constexpr fmr::Geom_float fac3_meas  = 1.0;// ntrl elem surface area
static constexpr
fmr::Geom_float fac3_vert_coor [fac3_vert_n * fac3_d ]// transposed coor_vert
  = {
            // vertex             |   5-----4  -+
  0.0, 0.0, // 0                  |    \   /    |
  1.0, 0.0, // 1                  |     \ /     |   zero
  0.0, 1.0, // 2    5,2-----1,4   |      3      | thickness
            //         \   /      |             |
  0.0, 0.0, // 3        \ /       |   2-----1  -+
  1.0, 0.0, // 4         0,3      |    \   /        y z x
  0.0, 1.0  // 5                  |     \ /          \|/
};          //                    |      0            o
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
  0,1,2,3, 7,6,5,4
};
static constexpr fmr::Geom_float fac4_meas  = 8.0;// natural element area
static constexpr
fmr::Geom_float fac4_vert_coor [fac4_vert_n * fac4_d ]// transposed coor_vert
  = {
             // vertex                     |       7---------6  -+           //
  -1.0,-1.0, // 0                          |      /         /    |           //
   1.0,-1.0, // 1                          |     /         /     |   zero    //
   1.0, 1.0, // 2        7,3---------2,6   |    /         /      | thickenss //
  -1.0, 1.0, // 3         /         /      |   4---------5       |           //
             //          /         /       |                     |           //
  -1.0,-1.0, // 4       /         /        |       3---------2  -+           //
   1.0,-1.0, // 5    4,0---------1,5       |      /         /        z  y    //
   1.0, 1.0, // 6                          |     /         /         | /     //
  -1.0, 1.0  // 7                          |    /         /          |/      //
};           //                            |   0---------1           o--x    //
static constexpr
fmr::Geom_float fac4_coor_vert [fac4_d * fac4_vert_n]// transposed vert_coor
  = {
 -1.0, 1.0, 1.0,-1.0,  -1.0, 1.0, 1.0,-1.0,
 -1.0,-1.0, 1.0, 1.0,  -1.0,-1.0, 1.0, 1.0
};

//static constexpr fmr::Geom_float tet4_node_coor [12] = tets_vert_coor;
//static constexpr fmr::Geom_float tet4_coor_node [12] = tets_coor_vert;

} } }//end femera::grid::fems:: namespace
