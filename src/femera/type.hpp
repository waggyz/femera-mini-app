#ifndef FEMERA_HAS_TYPE_HPP
#define FEMERA_HAS_TYPE_HPP
/** */

namespace femera {
  enum class Task_type : fmr::Enum_int {};/* strongly typed int; see:
  https://stackoverflow.com/questions/18195312
  /what-happens-if-you-static-cast-invalid-value-to-enum-class
  */
  enum class Base_type : fmr::Enum_int {//TODO Remove?
    None=0, Work, Proc, Data, Test, Task,
    // Geom, Load, Phys, Cond, Solv, Sync, Post,
    //Frun, Part,// Derived from Task
    Plug// Must be last, derived from TODO
  };
  enum class Plug_type : fmr::Enum_int;//TODO forward declare for definition in Plug.hpp?
  static constexpr Task_type task_cast (Task_type) noexcept;
  static constexpr Task_type task_cast (Base_type) noexcept;//TODO Remove?
  static constexpr Task_type task_cast (Plug_type) noexcept;

  enum class Plug_type : fmr::Enum_int {//TODO Move to Plug.hpp?
    None = fmr::Enum_int(Base_type::None),
    Plug = fmr::Enum_int(Base_type::Plug),// Derived from TODO
    Main, Fcpu, Node, Root,// Proc types
    Fomp, // Proc type
//#ifdef FMR_HAS_MPI
    Fmpi, // Proc type
//#endif
//#ifdef FMR_HAS_NVIDIA
    Nvid, // Proc type
//#endif
//#ifdef FMR_HAS_GTEST
    Gtst, // Proc type
//#endif
    File, Logs, // Data type
    Beds, Unit, Self, Perf, // Test types
//#ifdef FMR_HAS_CGNS
    Cgns, // Data type: CGNS file handler
//#endif
//#ifdef FMR_HAS_MOAB
    Moab, // Data type: MOAB file handler
//#endif
//#ifdef FMR_HAS_GMSH
    Gmsh, // Data type: Gmsh file handler
//f#endif
    Jobs, Sims, //Task type
    Petsc, // PETSc handler
#if 0
    Dflt, // Data type: default data for testing
    //
    Geom, // Move to base/core type?
    Mesh, Grid, Gcad,//TODO Gfem, Gfdm, Gfvm,// Geom types
    Cell,// FD, FV, SG // SG: staggered grid
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
    User// Must be last.
  };

}//end femera::namespace

#include "type.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_TYPE_HPP
#endif