#ifndef FEMERA_HAS_GRID_ELEM_CUBE_HPP
#define FEMERA_HAS_GRID_ELEM_CUBE_HPP

#include "Quad.hpp"

namespace femera { namespace grid { namespace elem {

struct Cube : public Elem<Cube> {//TODO Hex?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 8;
  static constexpr fmr::Local_int edge_n =12;
  static constexpr fmr::Local_int tris_n = 0;
  static constexpr fmr::Local_int quad_n = 6;
  static constexpr fmr::Local_int vols_n = 1;
//  static constexpr fmr::Local_int conn_n = 8;
  //
  static constexpr fmr::Geom_float edge_l =12.0;// total length of edges
  static constexpr fmr::Geom_float face_a = 6.0;// total surface area
  static constexpr fmr::Geom_float elem_v = 1.0;// natural element volume
  //
  static constexpr
  fmr::Local_int vert_conn [vert_n] = {0,1,2,4};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
  };
  static constexpr fmr::Local_int* tris_conn = nullptr;
  static constexpr fmr::Local_int  quad_conn [Quad::vert_n * quad_n]
    = { 
    0,1,2,3,
    7,6,5,4,
    0,4,5,1,
    1,5,6,2,
    2,6,7,3,
    3,7,4,0
  };
  fmr::Geom_float vert_coor [vert_n * sims_d]// transposed coor_vert
    = {
                   // vertex   7---------6                //
    0.0, 0.0, 0.0, // 0       /|        /|                //
    1.0, 0.0, 0.0, // 1      / |       / |                //
    1.0, 1.0, 0.0, // 2     /  |      /  |                //
    0.0, 1.0, 0.0, // 3    4---------5   |                //
                   //      |   |     |   |                //
    0.0, 0.0, 1.0, // 4    |   3-----|---2                //
    1.0, 0.0, 1.0, // 5    |  /      |  /      z  y       //
    1.0, 1.0, 1.0, // 6    | /       | /       | /        //
    0.0, 1.0, 1.0  // 7    |/        |/        |/         //
  };               //      0---------1         o--x       //
  fmr::Geom_float coor_vert [vert_n * sims_d]// transposed vert_coor
    = {
    0.0, 1.0, 1.0, 0.0,  0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 1.0,  0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 0.0, 0.0,  1.0, 1.0, 1.0, 1.0
  };
};

} } }//end femera::grid::elem:: namespace

#include "Cube.ipp"

//end FEMERA_HAS_GRID_ELEM_CUBE_HPP
#endif

#if 0
  const Mesh::ints vert_conn={ 0,1,2,3, 4,5,6,7 };
  const Mesh::ints vert_edge// Bar2 connectivity
    ={ 0,1, 1,2, 2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7 };
  const Mesh::ints vert_face
    ={ 0,1,2,3, 7,6,5,4, 0,4,5,1, 2,3,7,6, 0,3,7,4, 1,5,6,2 };
  const fmr::Geom_float node_coor={
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
