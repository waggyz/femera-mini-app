#ifndef FEMERA_HAS_DATA_HPP
#define FEMERA_HAS_DATA_HPP

#include "Work.hpp"

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  template <typename T>
  class Data : public Work {
  private:// typedefs
    using This_spt = FMR_SMART_PTR<T>;
  protected:// typedefs
    using Data_list_t = std::vector <fmr::Data_name_t>;
  protected:// member variables
    // Each data handler has a map of data/file names handled by each thread.
    // Set inp/out_name_list in each handler derived from data.
    std::unordered_map <fmr::Data_name_t, Data_list_t> inp_name_list ={};
    std::unordered_map <fmr::Data_name_t, Data_list_t> out_name_list ={};
#if 0
  private:// variables
    std::unordered_map <fmr::Data_name_t, std::vector <T*>> inp_data_task ={};
    std::unordered_map <fmr::Data_name_t, std::vector <T*>> out_data_task ={};
#endif
  public:// virtual methods
    std::string get_base_abrv () noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
  public:// methods
    static constexpr
    This_spt new_task (Work::Core_ptrs_t) noexcept;
    //
    T* get_task (fmr::Local_int)                 noexcept;
    T* get_task (Work::Task_path_t)              noexcept;
    T* get_task (Work_type, fmr::Local_int ix=0) noexcept;
    T* get_task (Task_type, fmr::Local_int ix=0) noexcept;
    //
    T* get_task (const fmr::Data_name_t&, fmr::Local_int ix=0) noexcept;
    //
  private:
#if 0
    T* get_task (fmr::Data_name_t, fmr::Vals_type,   fmr::Local_int ix=0) noexcept;
    T* get_task (fmr::Data_name_t, fmr::Vals_name_t, fmr::Local_int ix=0) noexcept;
#endif
    constexpr
    T* this_cast (Data*) noexcept;
    constexpr
    T* this_cast (Work*) noexcept;
  protected:// Make it clear this class needs to be inherited from.
    Data (Work::Core_ptrs_t) noexcept;// preferred constructor
    Data ()            =default;// early (init) constructor
    Data (Data&&)      =default;// movable
    Data& operator
      =  (const Data&) =delete; // not assignable
    Data (const Data&) =delete; // not copyable
    ~Data ()=default;// destructor
  };
}//end femera:: namespace
#undef FMR_DEBUG

#include "Data.ipp"

//end FEMERA_HAS_DATA_HPP
#endif
