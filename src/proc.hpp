#ifndef FMR_API_PROC_HPP
#define FMR_API_PROC_HPP
/** */
#ifdef FMR_EXPOSE_LEAVES
//#include "Main/Plug.hpp"
#else
#include "main.hpp"
#endif

#if 1
namespace fmr{ namespace proc{
extern inline bool is_master ();
extern inline int  wait_for_team ();
} }

// Thin wrappers are inline and defined here.
bool fmr::proc:: is_master (){return fmr::detail::main->proc-> is_master (); }
int  fmr::proc:: wait_for_team(){return fmr::detail::main->proc-> barrier ();}

#endif
//end FMR_API_PROC_HPP
#endif
