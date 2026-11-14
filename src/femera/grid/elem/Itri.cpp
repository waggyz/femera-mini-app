#include "Itri.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    constexpr fmr::Local_int Itri::tris_vert_conn [];
    //constexpr fmr::Local_int Itri::quad_vert_conn [];
    constexpr fmr::Local_int Itri::bars_vert_conn [];
    constexpr fmr::Geom_float Itri::vert_coor [];
    constexpr fmr::Geom_float Itri::coor_vert [];
    
    } } }//end femera::grid::elem namespace
