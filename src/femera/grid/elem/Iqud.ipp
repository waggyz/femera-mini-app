#ifndef FEMERA_HAS_GRID_ELEM_IQUD_IPP
#define FEMERA_HAS_GRID_ELEM_IQUD_IPP

namespace femera { namespace grid { namespace elem {
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
//constexpr fmr::Local_int Iqud::tris_conn [];
constexpr fmr::Local_int Iqud::quad_conn [];
constexpr fmr::Local_int Iqud::spar_conn [];
constexpr fmr::Geom_float Iqud::vert_coor [];
constexpr fmr::Geom_float Iqud::coor_vert [];

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_IQUD_IPP
#endif
