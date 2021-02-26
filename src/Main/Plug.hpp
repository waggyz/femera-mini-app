#ifndef FMR_HAS_PLUG_HPP
#define FMR_HAS_PLUG_HPP
/** */
#ifdef FMR_EXPOSE_LEAVES
//#include "../core.hpp"
#else
#include "../main.hpp"
#endif

#ifdef _OPENMP
#ifndef FMR_HAS_OMP
#define FMR_HAS_OMP
#endif
#include "../Proc/Pomp.hpp"
#endif

//#ifdef FMR_HAS_MPI
#ifndef FMR_HAS_MPI
#define FMR_HAS_MPI
#endif
#include "../Proc/Pmpi.hpp"
//#endif

//#ifdef FMR_HAS_GTEST
#ifndef FMR_HAS_GTEST
#define FMR_HAS_GTEST
#endif
#include "../Proc/Gtst.hpp"
//#endif

//#ifdef FMR_HAS_CGNS
#ifndef FMR_HAS_CGNS
#define FMR_HAS_CGNS
#endif
#ifndef FMR_HAS_HDF5
#define FMR_HAS_HDF5
#endif
#include "../Data/Dcgn.hpp"
//#endif

#ifndef FMR_HAS_GMSH
#define FMR_HAS_GMSH
#include "../Data/Dmsh.hpp"
#endif

#include "../Data/Fake.hpp"
#include "../Sims/Part.hpp"
#include "../Sims/Frun.hpp"
#include "../Sims/Geom.hpp"
#include "../Sims/Mesh.hpp"

#if 0
//#include "../Sims/Cpu1.hpp"
//#include "../Sims/Fset.hpp"
//#include "../Sims/Fifo.hpp"
#endif

#include "../plug.hpp"

namespace Femera{
  enum class Plug_type :fmr::Enum_int{//TODO Move to fmr::detail:: or Femera:: ?
#ifdef FMR_HAS_OMP
    Pomp=fmr::Enum_int(Base_type::Pomp),// Derived from Proc
#endif
    None=fmr::Enum_int(Base_type::None),
    Plug=fmr::Enum_int(Base_type::Plug),// Derived from Main

#ifdef FMR_HAS_MPI
    Pmpi, // Proc type
#endif
#ifdef FMR_HAS_GTEST
    Gtst, // Proc type
#endif
#ifdef FMR_HAS_DFMR
    Dfmr, // File type
#endif
#ifdef FMR_HAS_CGNS
    Dcgn, // File type
#endif
#ifdef FMR_HAS_GMSH
    Dmsh, // File type
#endif
    Fake,//  // File type: default data for testing
    //
    Geom, // Move to base/core type?
    Mesh, Grid,//TODO Gfem, Gfdm, Gfvm,// Geom types
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
    Sgdm, Spcg, Sncg,// Solvers
    Mono, Halo,
#ifdef FMR_HAS_TESTS
    Test,
#endif
    User// Must be last.
  };
class Proc; class Data;
class Plug final : public Main {
  protected:
#if 0
    std::vector<std::string> loaded_names={};//TODO std::set?
#else
    std::set<std::string> loaded_names={};// std::set?
#endif
  public:
    virtual ~Plug();// noexcept = default;
    Plug (Proc*, Data*);
    Plug ()=delete;
    Plug (Plug const&)=delete;// not copyable
    Plug operator= (const Plug&)=delete;
//    int add_new_task (fmr::Distribute, Work* add_to)final override;//TODO remove
//    template<typename T> int add_new (T type, Work* add_to);//TODO not template
    int add_new_task (Femera::Base_type, Work* add_to) final override;
    int add_new_task (Femera::Plug_type, Work* add_to) final override;
    int add_new_task (Femera::Work_type, Work* add_to) final override;
  protected:
    int init_task (int* argc, char** argv) final override;
    int exit_task (int err) final override;
  private:
    int prep () final override;
    template<class T> void add (T* derived, T* base);
};
template<class T> void Plug::add (T* derived, T* base){
  base->task.add (derived);
  this->loaded_names.insert (derived->task_name);
}
}//end Femera namespace
//end FMR_HAS_PLUG_HPP
#endif
