#include "Prmd.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    constexpr fmr::Local_int Prmd::tris_vert_conn [];
    constexpr fmr::Local_int Prmd::quad_vert_conn [];
    constexpr fmr::Local_int Prmd::bars_vert_conn [];
    constexpr fmr::Geom_float Prmd::vert_coor [];
    constexpr fmr::Geom_float Prmd::coor_vert [];
    
    } } }//end femera::grid::elem namespace
