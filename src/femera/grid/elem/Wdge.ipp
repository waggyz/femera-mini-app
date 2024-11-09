#ifndef FEMERA_HAS_GRID_ELEM_WDGE_IPP
#define FEMERA_HAS_GRID_ELEM_WDGE_IPP

namespace femera { namespace grid { namespace elem {
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
constexpr fmr::Local_int Wdge::tris_conn [];
constexpr fmr::Local_int Wdge::quad_conn [];
constexpr fmr::Local_int Wdge::spar_conn [];
constexpr fmr::Geom_float Wdge::vert_coor [];
constexpr fmr::Geom_float Wdge::coor_vert [];

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_WDGE_IPP
#endif
