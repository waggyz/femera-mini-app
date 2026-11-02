#include "Ibar.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    //constexpr fmr::Local_int Ibar::tris_conn [];
    constexpr fmr::Local_int Ibar::quad_conn [];
    constexpr fmr::Local_int Ibar::bars_conn [];
    constexpr fmr::Geom_float Ibar::vert_coor [];
    constexpr fmr::Geom_float Ibar::coor_vert [];
    
    } } }//end femera::grid::elem namespace
    
    //end FEMERA_HAS_GRID_ELEM_IQUD_IPP
