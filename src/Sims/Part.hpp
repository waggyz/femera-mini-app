#ifndef FMR_HAS_PART_HPP
#define FMR_HAS_PART_HPP
/** */

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
#if 0
class Geom;// forward declares
class Load;
class Mtrl;
class Phys;
class Sync;
class Cond;
class Solv;
class Post;
#endif
class Part final : public Sims {
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  // this->task contains Geom or Elem objects derived from Sims?
  public:
  protected:
    // part_dims, phys_dims inherited from Sims
    fmr::Enum_int_vals elem_type// size: mesh_n
      = fmr::Enum_int_vals(fmr::Data::Elem_type,0);
    fmr::Enum_int_vals cell_type// size: grid_n
      = fmr::Enum_int_vals(fmr::Data::Cell_type,0);
#if 0
    Sims* parent =nullptr;// parent sim/part
    Load* load =nullptr;
    Phys* phys =nullptr;
    Mtrl* mtrl =nullptr;
    Cond* cond =nullptr;
    Solv* solv =nullptr;
    Sync* sync =nullptr;
    Post* post =nullptr;
#endif
  private:
//    fmr::Data_id   part_id = "(unknown)";
    fmr::Local_int part_ix = 0;//TODO needed?
    fmr::Dim_int   part_lv = 1;// Parent sim level is 0.
  // methods ----------------------------------------------------------------
  public:
    fmr::Data_id make_id() override;//TODO virtual Sims::make_id()
    fmr::Data_id get_id() override;//TODOvirtual Sims::get_id()
  protected:
    int prep () final override;
    int chck () final override;
    int run  () final override;
    int exit_task (int  err) final override;
  private:
  public:
    virtual ~Part ()    noexcept=default;// Virtual destructor for virtual class
    Part (Sims*)        noexcept;
    Part ()                     =delete;
    Part (Part const&)          =delete;// not copyable
    Part operator= (const Part&)=delete;
};
}//end Femera namespace
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Part.hpp")
//end FMR_HAS_PART_HPP
#endif
