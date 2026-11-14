#include "Tris.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    constexpr fmr::Local_int Tris::tris_vert_conn [];
    //constexpr fmr::Local_int Tris::quad_vert_conn [];
    constexpr fmr::Local_int Tris::bars_vert_conn [];
    constexpr fmr::Geom_float Tris::vert_coor [];
    constexpr fmr::Geom_float Tris::coor_vert [];
    
    } } }//end femera::grid::elem namespace
