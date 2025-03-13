#include "Tets.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    constexpr fmr::Local_int Tets::tris_conn [];
    constexpr fmr::Local_int Tets::spar_conn [];
    //
    constexpr fmr::Geom_float Tets::vert_coor [];
    constexpr fmr::Geom_float Tets::coor_vert [];
    constexpr fmr::Geom_float Tets::coor_tet10 [];
    constexpr fmr::Geom_float Tets::tet10_coor [];
    constexpr fmr::Geom_float Tets::coor_tet20 [];
    constexpr fmr::Geom_float Tets::tet20_coor [];
    //
    constexpr fmr::Phys_float Tets::intg_1_ptwt [];
    constexpr fmr::Phys_float Tets::intg_4_ptwt [];
    constexpr fmr::Phys_float Tets::intg_5_ptwt [];
    constexpr fmr::Phys_float Tets::intg_10_ptwt [];
    constexpr fmr::Phys_float Tets::intg_11_ptwt [];
    //
} } }//end femera::grid::elem namespace
