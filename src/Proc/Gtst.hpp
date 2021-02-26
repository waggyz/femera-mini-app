#ifndef FMR_HAS_WORK_GTST_HPP
#define FMR_HAS_WORK_GTST_HPP
/** */

#ifndef FMR_HAS_GTEST
#define FMR_HAS_GTEST
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Proc;
class Gtst final : public Proc{
  public:
    virtual ~Gtst () noexcept = default;
    Gtst (Proc*,Data*);
    Gtst ()=delete;
    Gtst (Gtst const& )=delete;// not copyable
    Gtst operator= (const Gtst&)=delete;
    //
    int prep () final override;
    int chck () final override;
  protected:
    int init_task (int* argc, char** argv) final override;
    int exit_task (int err) final override;
  private:
    bool is_enabled    = false;
    bool ran_all_tests = false;
};
}//end Femera namespace
#undef FMR_DEBUG
#endif
