#ifndef FMR_HAS_DATA_FAKE_HPP
#define FMR_HAS_DATA_FAKE_HPP
/** */

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Fake final : public Data {
  // typedefs ---------------------------------------------------------------
  // member variables -------------------------------------------------------
  public:
  protected:
  private:
    bool use_test_data = false;
  // methods ----------------------------------------------------------------
  protected:
    int init_task (int* argc, char** argv ) final override;
    int exit_task (int  err) final override;
    std::deque<std::string> get_sims_names () final override;
    bool is_this_type (const std::string fname) final override;
  private:
    int prep  () final override;
    int close () final override;
    int close (const std::string model) final override;
    //
    Fake ()=default;
    Fake (Fake const&)=delete;// not copyable
    Fake operator= (const Fake&)=delete;
  public:
    Fake (Proc*, Data*) noexcept;
    virtual ~Fake () noexcept=default;// Virtual destructor for virtual class
};
}//end Femera namespace
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Fake.hpp")
//end FMR_HAS_DATA_FAKE_HPP
#endif
