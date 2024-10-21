#ifndef FEMERA_HAS_TYPE_HPP
#define FEMERA_HAS_TYPE_HPP
/** */

namespace femera {
  enum class Work_type : fmr::Enum_int {};/* strongly typed integer; see:
  https://stackoverflow.com/questions/18195312
  /what-happens-if-you-static-cast-invalid-value-to-enum-class
  */
  enum class Base_type : fmr::Enum_int {             // Abstract CRTP base classes
    None=0, Work, Proc, Data, Test, Phys, Grid, Algo,// derived from Work
    // Geom, Load, Cond, Solv, Sync, Part?, Post?, View?
    Task // Must be last, Task_type numbers start after this.
  };
  enum class Task_type : fmr::Enum_int {// Built-in modules
    None = fmr::Enum_int (Base_type::None),
    Task = fmr::Enum_int (Base_type::Task),// start numbering for Task_type
    Main, Fcpu, Node, Root,      // Proc types
    Fomp,                        // Proc type
//#ifdef FMR_HAS_MPI
    Fmpi,                        // Proc type
//#endif
//#ifdef FMR_HAS_NVIDIA
    Nvid,                        // Proc type: NVIDIA GPU handler
//#endif
    File, Logs, Dlim, Text, Bank,// Data types
//#ifdef FMR_HAS_CGNS
    Cgns,                        // Data type: CGNS file handler
//#endif
//#ifdef FMR_HAS_MOAB
    Moab,                        // Data type: MOAB file handler
//#endif
//#ifdef FMR_HAS_GMSH
    Gmsh,                        // Data type: Gmsh handler
//#endif
    Petsc,                       // Data type: PETSc handler
    Beds, Unit, Self, Perf,      // Test types
//#ifdef FMR_HAS_GTEST
    Gtst,                        // Test type: Googletest handler
//#endif
    Jobs, Sims, Runs,            // Task types //TODO: Part?, Post?, View?
    Mtrl, Elas, Plas,            // Phys types
    Cell, Fems, FDms, FVms,      // Grid types
    Slvr, Spcg, Sncg,            // Algo types
#if 0
    Geom, // Move to base/core type?
    Mesh, Grid, Gcad,// Gfem, Gfdm, Gfvm,// Geom types
    Cell,// FD, FV, SG // SG: staggered grid
    Surf,// BEM surfaces
    Elem,// FEM low-order (p:1,2,3) elements
    Elfs,// FEM Finite-strain elements (or just different material/physics?)
    Bars, Quad, Tris, Tets, Pyrm, Prsm, Cube, Beam, Shll,// Elem types
    Cmat, Cuel,// C or C++ source code
    Fmat, Fuel,// Fortran source code (Umat, Uel ?)
    Boco, Fix0, Diri, Neum,
    Elas, Ther,
    Cnd1, Cjac,// Preconditioners
    Grad, Spcg, Sncg,// Solvers
    Mono, Halo,
#endif
    User// Must be last, User_type numbering starts after this.
  };
  enum class User_type : fmr::Enum_int;// Forward declare, known at build time
  // User_type::Plug is the last in User_type and starts Plug_type numbering
  enum class Plug_type : fmr::Enum_int;// Forward declare, for runtime plugins
  //
  static constexpr Work_type task_cast (Work_type) noexcept;
  static constexpr Work_type task_cast (Base_type) noexcept;
  static constexpr Work_type task_cast (Task_type) noexcept;
  static constexpr Work_type task_cast (User_type) noexcept;
  static constexpr Work_type task_cast (Plug_type) noexcept;
  //
}//end femera::namespace

#include "type.ipp"

#undef FMR_DEBUG
//end FEMERA_HAS_TYPE_HPP
#endif
