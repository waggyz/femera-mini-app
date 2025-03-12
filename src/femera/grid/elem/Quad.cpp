#include "Quad.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    //constexpr fmr::Local_int Quad::tris_conn [];
    constexpr fmr::Local_int Quad::quad_conn [];
    constexpr fmr::Local_int Quad::spar_conn [];
    constexpr fmr::Geom_float Quad::vert_coor [];
    constexpr fmr::Geom_float Quad::coor_vert [];
    
    } } }//end femera::grid::elem namespace
