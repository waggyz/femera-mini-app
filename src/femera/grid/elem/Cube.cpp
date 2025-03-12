#include "Cube.hpp"

namespace femera { namespace grid { namespace elem {
    /*
    C++11 requires redundant declaration/definition for constexpr arrays to 
    reference them later.
    */
    //constexpr fmr::Local_int Cube::tris_conn [];
    constexpr fmr::Local_int Cube::quad_conn [];
    constexpr fmr::Local_int Cube::spar_conn [];
    constexpr fmr::Geom_float Cube::vert_coor [];
    constexpr fmr::Geom_float Cube::coor_vert [];
    
    } } }//end femera::grid::elem namespace
