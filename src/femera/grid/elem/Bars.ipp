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

constexpr fmr::Local_int Bars::bars_conn [];
constexpr fmr::Geom_float Bars::vert_coor [];
constexpr fmr::Geom_float Bars::coor_vert [];

#endif
} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_BARS_IPP
#endif
