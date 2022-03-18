#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

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
    std::string   get_base_name ()           noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
#if 0
  public:
    // File operations are spawned from the local OpenMP master thread, and
    // file methods use vector types for shared-memory access by proc_id, e.g.,
    // local_x = vec_x [proc_id % vec_x.size ()];
    // or better, contiguous blocks of vals for each proc_id
    //
    using Byte_n      = std::vector <fmr::perf::Count>;// bytes in or out
    using Vals_t      = std::vector <Vals*>;
    //-------------------------------------------------------------------------
    using Sims_name_t = std::deque <std::string>;
    using Path_file_t = std::deque <std::string>;
    //TODO Path_file_t can be a vector to disambiguate it from Sims_name_t?
    //NOTE An empty deque/vector in an argument means all sims or all files.
    // Path_file_t ({""}); means the current working directory in a path method
    // Text files in Path_file_t may contain a list of model files.
    //
    //TODO functions that take these are overloaded for std::string&, e.g.,
    //     static inline constexpr Sims_name_t add_sims (std::string& s="") {
    //       return add_sims (Sims_name_t({s});
    //     }
    // Data source methods ----------------------------------------------------
    // Data_type::As_needed avoids big data ops and may make false assumptions.
    // E.g., Gmsh (*.msh) files are assumed to contain node & element data.
    //
    Path_file_t  add_path (Path_file_t& ={""});// add to filename search path
    //NOTE add_path ({}); does nothing.
    Path_file_t  add_tree (Path_file_t& ={""});// add dir & subdirs recursively
    //NOTE add_tree ({}); adds subdirs of dirs in the current path recursively.
    Path_file_t  clr_path (Path_file_t& ={});// clear filename search path
    Path_file_t scan_path (Path_file_t& ={});// get model filenames in path
    //
    Path_file_t  add_file (Path_file_t& ={});
    Sims_name_t scan_file (Path_file_t& ={}, Data_type =Data_type::As_needed);
    // scan_file (..) registers the data in files
    //
    fmr::Local_int get_sims_n (Path_file_t& ={});
    fmr::Local_int get_file_n (Path_file_t& ={});
    //
    Sims_name_t  add_sims (Sims_name_t& ={});// register more models
    Sims_name_t  set_sims (Sims_name_t& ={});// clear list then add models
    Sims_name_t  clr_sims (Sims_name_t& ={});// clear model data
    Sims_name_t  get_sims (Sims_name_t& ={});// returns a list of sims loaded
    //
    Sims_name_t  add_data (fmr::Test_type =fmr::Test_type::As_needed);
    // Data read/write methods ------------------------------------------------
    //
    Byte_n scan_data (Sims_name_t ={}, Data_type =Data_type::As_needed);
    //
    Vals_t  get_data (Sims_name_t ={}, Data_type =Data_type::As_needed);
    Byte_n  clr_data (Sims_name_t ={}, Data_type =Data_type::All);
    //
    //NOTE (..) means appropriate arguments
    Byte_n read_data (..);// read whole dataset
    Byte_n send_data (..);// append, write; new if destination does not exist
    Byte_n save_data (..);// new, overwrite
    //
    Byte_n read_line (..);// one line at a time
    Byte_n send_line (..);// append, write; new if destination does not exist
    Byte_n save_line (..);// new, overwrite
    //
    Byte_n read_page (..);// read data in blocks
    Byte_n send_page (..);// append, write; new if destination does not exist
    Byte_n save_page (..);// new, overwrite
    //
    Byte_n read_cols (..);// read data by column (SoA)//TODO or read_vals ?
    Byte_n send_cols (..);// append write; new if destination does not exist
    Byte_n save_cols (..);// new, overwrite
    //
    Byte_n read_item (..);// read data item(s)
    Byte_n send_item (..);// append write; new if destination does not exist
    Byte_n save_item (..);// new, overwrite
    //-------------------------------------------------------------------------
#endif
  public:
    static constexpr
    This_spt new_task (const Work::Core_ptrs_t)        noexcept;
    T*       get_task (fmr::Local_int)                 noexcept;
    T*       get_task (Work::Task_path_t)              noexcept;
    T*       get_task (Task_type, fmr::Local_int ix=0) noexcept;
    T*       get_task (Plug_type, fmr::Local_int ix=0) noexcept;
  private:
    constexpr
    T* this_cast (Data*) noexcept;
    constexpr
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
