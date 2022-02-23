#ifndef FEMERA_HAS_TASK_HPP
#define FEMERA_HAS_TASK_HPP
/** */


namespace femera {
  enum class Task_type : fmr::Enum_int {};// strongly typed int; see:
  // https://stackoverflow.com/questions/18195312/what-happens-if-you-static-cast-invalid-value-to-enum-class
  enum class Base_type : fmr::Enum_int {
    None=0, Work, Proc, Data, Test, Sims,
    // Geom, Load, Phys, Cond, Solv, Sync, Post,
#ifdef _OPENMP_FIXME_DISABLED
    Fomp,// actually derived from Proc, but treated as a Base_type
#endif
    //Frun, Part,// Derived from Sims
    Plug// Must be last, derived from FIXME
  };
  enum class Plug_type : fmr::Enum_int;//TODO forward declare for definition in Plug.hpp?
  static inline constexpr Task_type task_cast (Task_type) noexcept;
  static inline constexpr Task_type task_cast (Base_type) noexcept;
  static inline constexpr Task_type task_cast (Plug_type) noexcept;

  enum class Plug_type : fmr::Enum_int {//TODO Move to Plug.hpp?
#ifdef FMR_HAS_OMP_FIXME_DISABLED
    Fomp = fmr::Enum_int(Base_type::Fomp),// Derived from Proc
#endif
    None = fmr::Enum_int(Base_type::None),
    Plug = fmr::Enum_int(Base_type::Plug),// Derived from FIXME
    Main, Fcpu, Node, Root,// Proc types
#ifndef FMR_HAS_OMP_FIXME_DISABLED
    Fomp,
#endif

#ifdef FMR_HAS_MPI
    Fmpi, // Proc type
#endif
#ifdef FMR_HAS_NVIDIA
    Nvid,
#endif
#ifdef FMR_HAS_GTEST
    Gtst, // Proc type
#endif
    File, Logs, // Data type
    Beds, Unit, Self, Perf, // Test types
#if 0
#ifdef FMR_HAS_CGNS
    Dcgn, // Data type
#endif
#ifdef FMR_HAS_GMSH
    Dmsh, // Data type
#endif
    ??? Fake,//  // Data type: default data for testing
    //
    Geom, // Move to base/core type?
    Mesh, Grid, Gcad,//TODO Gfem, Gfdm, Gfvm,// Geom types
    Cell,// FD, FV, SG // staggered-grid
    Surf,// BEM surfaces
    Elem,// FEM low-order (p:1,2,3) elements
    Elfs,// FEM Finite-strain elements (or just different material/physics?)
    Bars, Quad, Tris, Tets, Pyrm, Prsm, Cube,// Elem types
//    Beam, Shll,// Elem types
    Mtrl,
    Boco, Fix0, Diri, Neum,
    Elas, Ther,
    Cnd1, Cjac,// Preconditioners
    Grad, Spcg, Sncg,// Solvers
    Mono, Halo,
#endif
    Jobs, //Sims type
    User// Must be last.
  };

}//end femera::namespace

#include "type.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_TASK_HPP
#endif