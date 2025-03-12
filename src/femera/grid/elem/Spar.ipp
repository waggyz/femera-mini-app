#ifndef FEMERA_HAS_GRID_ELEM_SPAR_IPP
#define FEMERA_HAS_GRID_ELEM_SPAR_IPP

namespace femera { namespace grid { namespace elem {
#if 0
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
//constexpr fmr::Local_int Spar::tris_conn [];
//constexpr fmr::Local_int Spar::quad_conn [];

constexpr fmr::Local_int Spar::spar_conn [];
constexpr fmr::Geom_float Spar::vert_coor [];
constexpr fmr::Geom_float Spar::coor_vert [];

#endif
} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_SPAR_IPP
#endif
