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
  // methods ----------------------------------------------------------------
  protected:
    int prep () final override;
    int chck () final override;
    int run  () final override;
    int exit_task (int  err) final override;
  private:
  public:
    virtual ~Part  ()     noexcept=default;//Virtual destructor for virt. class
    Part           (Sims*)noexcept;
    Part           ()             =delete;
    Part           (Part const&)  =delete;// not copyable
    Part operator= (const Part&)  =delete;
};
}//end Femera namespace
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Part.hpp")
//end FMR_HAS_PART_HPP
#endif
