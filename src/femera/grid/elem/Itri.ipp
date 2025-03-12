#ifndef FEMERA_HAS_GRID_ELEM_ITRI_IPP
#define FEMERA_HAS_GRID_ELEM_ITRI_IPP

namespace femera { namespace grid { namespace elem {
#if 0
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
constexpr fmr::Local_int Itri::tris_conn [];
//constexpr fmr::Local_int Itri::quad_conn [];
constexpr fmr::Local_int Itri::spar_conn [];
constexpr fmr::Geom_float Itri::vert_coor [];
constexpr fmr::Geom_float Itri::coor_vert [];
#endif
} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_ITRI_IPP
#endif
