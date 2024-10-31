#ifndef FEMERA_HAS_GRID_ELEM_TETS_IPP
#define FEMERA_HAS_GRID_ELEM_TETS_IPP

namespace femera { namespace grid { namespace elem {

// C++11 requires redundant declaration/definition for constexpr arrays to 
// reference them later.
constexpr fmr::Local_int Tets::tris_conn [];
//constexpr fmr::Local_int Tets::quad_conn [];
constexpr fmr::Local_int Tets::spar_conn [];
constexpr fmr::Geom_float Tets::vert_coor [];
constexpr fmr::Geom_float Tets::coor_vert [];

constexpr fmr::Phys_float Tets::intg_1_ptwt [];
constexpr fmr::Phys_float Tets::intg_4_ptwt [];
constexpr fmr::Phys_float Tets::intg_5_ptwt [];
constexpr fmr::Phys_float Tets::intg_10_ptwt [];
constexpr fmr::Phys_float Tets::intg_11_ptwt [];

} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_TETS_IPP
#endif
