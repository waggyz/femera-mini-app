#ifndef FEMERA_HAS_GRID_ELEM_TRISIPP
#define FEMERA_HAS_GRID_ELEM_TRISIPP

namespace femera { namespace grid { namespace elem {
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
constexpr fmr::Local_int Tris::tris_conn [];
//constexpr fmr::Local_int Tris::quad_conn [];
constexpr fmr::Local_int Tris::spar_conn [];
constexpr fmr::Geom_float Tris::vert_coor [];
constexpr fmr::Geom_float Tris::coor_vert [];

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_TRISIPP
#endif
