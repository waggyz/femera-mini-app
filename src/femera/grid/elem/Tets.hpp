#ifndef FEMERA_HAS_GRID_ELEM_TETS_HPP
#define FEMERA_HAS_GRID_ELEM_TETS_HPP

#include "Tris.hpp"

namespace femera { namespace grid { namespace elem {

struct Tets : public Elem<Tets> {//TODO Tet?
  static constexpr fmr::Local_int sims_d = 3;
  static constexpr fmr::Local_int elem_d = 3;
  static constexpr fmr::Local_int vert_n = 4;
  static constexpr fmr::Local_int edge_n = 6;
  static constexpr fmr::Local_int tris_n = 4;
  static constexpr fmr::Local_int quad_n = 0;
  static constexpr fmr::Local_int vols_n = 1;
  static constexpr fmr::Local_int conn_n = 4;
  //
  static constexpr fmr::Geom_float edge_l  // total length of edges
    = 3.0 + 3.0*(std::sqrt(2.0));
  static constexpr fmr::Geom_float face_a // total surface area
    = 1.5 + std::sqrt(2.0) * std::sqrt(1.5);
  static constexpr fmr::Geom_float elem_v  // natural element volume
    = 1.0 / 6.0;
  //
  //NOTE Gmsh element conventions
  static constexpr
    fmr::Local_int vert_conn [vert_n] = {0,1,2,3};
  static constexpr
  fmr::Local_int spar_conn [Spar::vert_n * edge_n]
    = {
    0,1, 1,2, 2,0, 0,3, 2,3, 1,3
  };
  static constexpr
  fmr::Local_int tris_conn [Tris::vert_n * tris_n]
    = { //NOTE the normals point inward. This might be wrong.
    0,1,2, 0,3,1, 0,2,3, 1,3,2
  };
  static constexpr
  fmr::Local_int* quad_conn = nullptr;
  static constexpr
  fmr::Geom_float vert_coor [vert_n * elem_d ]// transposed coor_vert
    = {            //            3               //
                   // vertex    /|\              //
    0.0, 0.0, 0.0, // 0        / | \             //
    1.0, 0.0, 0.0, // 1       2--|--1            //
    0.0, 1.0, 0.0, // 2        \ | /     y z x   //
    0.0, 0.0, 1.0  // 3         \|/       \|/    //
  };               //            0         o     //
  static constexpr
  fmr::Geom_float coor_vert [elem_d * vert_n]// transposed vert_coor
    = {
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };
  fmr::Geom_float cube_coor [3* 8]// same as cube elem below
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
                    //      0---------1         o--x       //
  };
  fmr::Local_int cube_conn_6 [4* 6]// 6-tet fill, tilable w/out rotation
    = {// These are all conformal and differ only by rotation.
    0,1,5,6,
    0,1,2,6,
    0,5,6,4,
    3,0,7,6,
    0,4,7,6,
    0,3,2,6
  };
  //fmr::Local_int tet5_cube_conn =[4* 6]//  5-tet fill
  fmr::Local_int cube_conn_5 [4* 5]// 5-tet fill, tilable with rotation
    = {
    1,3,4,6,// This one is twice the volume of the rest.
    0,1,3,4,// Identical to the natural tet.
    2,3,1,6,
    5,6,1,4,
    7,6,4,3
  };
//TODO tets patch test mesh
};

} } }//end femera::grid::elem:: namespace

#include "Tets.ipp"

//end FEMERA_HAS_GRID_ELEM_TETS_HPP
#endif
