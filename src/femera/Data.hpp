#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

// used in Data.ipp
#include "../fmr/form.hpp"
#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  protected:
    using File_ptrs_t = std::vector<FILE*>;
  public:
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
    std::string   get_base_name ()           noexcept final override;
#if 0
  public:
    void add_data (fmr::Test_data =fmr::Test_data::All);
    void add_data (fmr::Perf_data =fmr::Perf_data::All);
    //
    void set_sims (std::vector<std::strring>& sim_names);
    void add_sims (std::vector<std::strring>& sim_names);
    //
    void read_data ();// read whole dataset
    void send_data ();// append, write
    void save_data ();// new, overwrite
    //
    void read_line ();// one line at a time
    void send_line ();// append, write
    void save_line ();// new, overwrite
    //
    void read_page ();// read data in blocks
    void send_page ();// append, write
    void save_page ();// new, overwrite
    //
    void read_cols ();// by column (SoA)
    void send_cols ();// append write
    void save_cols ();// new, overwrite
#endif
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t)        noexcept;
    T*       get_task (fmr::Local_int)                 noexcept;
    T*       get_task (Work::Task_path_t)              noexcept;
    T*       get_task (Task_type, fmr::Local_int ix=0) noexcept;
    T*       get_task (Plug_type, fmr::Local_int ix=0) noexcept;
  private:
    T* this_cast (Data*) noexcept;
    T* this_cast (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
    Data ()            =default;
    Data (const Data&) =default;
    Data (Data&&)      =default;// shallow (pointer) copyable
    Data& operator =
      (const Data&)    =default;
    ~Data ()           =default;
  };
}//end femera:: namespace
#undef FMR_DEBUG

#include "Data.ipp"

//end FEMERA_TEST_HPP
#endif
