#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera { namespace data {
  using File_ptrs_t = std::vector <FILE*>;
  struct Page_dims {// regular blocked data, local (partitioned) data
    // for file and stream (e.g., stdout) reading and saving
#if 0
  public://TODO Remove? constructor
    Form_dims (fmr::Enum_int, fmr::Local_int =0, fmr::Enum_int =1,
      fmr::Dim_int =0, fmr::Dim_int =0);
#endif
  private:
//TODO Data_ptrs_t data;// data handler; OR use std::vector <task_type>?
//    std::vector <Task_type> task_type ={};// {task_cast (Plug_type::None)};
//    std::vector <File_type> file_type ={};// {File_type::Unknown};
    File_ptrs_t    file_ptrs ={};
    std::valarray <fmr::Local_int> item_size ={1};// vals / item
    // vals are all the same type
    fmr::Local_int line_n    = 0;// total lines in this
    fmr::Local_int page_0    = 0;// global index of first page in this
    fmr::Local_int page_size = 0;// lines / page; a page can be a partition
    fmr::Local_int line_size = 0;// items / line
    fmr::Local_int head_size = 0;// page header size in lines
    fmr::Local_int foot_size = 0;// page footer size in lines
    // head_ and foot_size can be used for (alignment) padding homogeneous data
  };
} }//end femera::data:: namespace
namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  public:
    std::string   get_base_name ()           noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
#if 0
  public:
    //TODO ? File operations are spawned from the local OpenMP master thread, and
    //     ? file methods use vector types for shared-memory access by proc_id, e.g.,
    // local_x = vec_x [proc_id % vec_x.size ()];
    // or better, contiguous blocks of vals for each proc_id
    //
    using Byte_list = std::vector <fmr::perf::Count>;// bytes in or out
    using Vals_list = std::vector <Vals*>;
    //-------------------------------------------------------------------------
    using Sims_list = std::deque <std::string>;
    using Path_list = std::deque <std::string>;
    //TODO Path_list can be a vector to disambiguate it from Sims_list?
    //NOTE An empty deque/vector in an argument means all sims or all files.
    // Path_list ({""}); means the current working directory in a path method.
    // Text files in Path_list may contain a list of model files.
    //
    //TODO functions that take these are overloaded for std::string&, e.g.,
    //     static inline constexpr Sims_list add_sims (std::string& s="") {
    //       return add_sims (Sims_list({s});
    //     }
  private:
    class File_name {
    public:
      std::string dir ="";// absolute or relative
      std::string base ="";// name without path and without (1-)indices.
      std::vector <fmr::Local_int> name_ix={};// 1-indices (model#, part#,...)
      std::string ext  ="";//NOTE includes the dot, e.g., ".txt"
    public:
      File_name (std::string&);
      std::string get_name ();
    };
    using File_list = std::deque <File_name>;
  private:
    Path_list path_list ={""};// first is default dir; "" is current working dir
    Path_list name_list ={};
    Sims_list sims_list ={};
  public:// Data source methods -----------------------------------------------
    //
    Path_list  add_path (Path_list& ={""});// add to filename search path
    //NOTE add_path ({}); does nothing.
    Path_list  add_tree (Path_list& ={""});// add dir & subdirs recursively
    //NOTE add_tree ({}); adds subdirs of dirs in the current path recursively.
    Path_list  clr_path (Path_list& ={});// clear filename search path
    Path_list scan_path (Path_list& ={});// get model filenames in path
    //
    Path_list  add_file (Path_list& ={});
    Sims_list scan_file (Path_list& ={}, Data_type =Data_type::As_needed);
    //                   registers the data in files
    // Data_type::As_needed avoids big data ops and may make false assumptions.
    // E.g., Gmsh (*.msh) files are assumed to contain node & element data.
    //
    fmr::Local_int get_sims_n (Path_list& ={});
    fmr::Local_int get_file_n (Path_list& ={});
    //
    Sims_list  add_sims (Sims_list& ={});// register more models
    Sims_list  set_sims (Sims_list& ={});// clear list then add models
    Sims_list  clr_sims (Sims_list& ={});// clear model data
    Sims_list  get_sims (Sims_list& ={});// returns a list of sims loaded
    //
    Sims_list  add_data (fmr::Test_type =fmr::Test_type::As_needed);
    // Data read/write methods ------------------------------------------------
    //
    Byte_list scan_data (Sims_list ={}, Data_type =Data_type::As_needed);
    //
    Vals_list  get_data (Sims_list ={}, Data_type =Data_type::As_needed);
    Byte_list  clr_data (Sims_list ={}, Data_type =Data_type::All);
    //
    //NOTE (..) means appropriate arguments
    Byte_list read_data (..);// read whole dataset
    Byte_list send_data (..);// append, write; new if destination does not exist
    Byte_list save_data (..);// new, overwrite
    //
    Byte_list read_line (..);// one line at a time
    Byte_list send_line (..);// append, write; new if destination does not exist
    Byte_list save_line (..);// new, overwrite
    //
    Byte_list read_page (..);// read data in blocks
    Byte_list send_page (..);// append, write; new if destination does not exist
    Byte_list save_page (..);// new, overwrite
    //
    Byte_list read_cols (..);// read data by column (SoA)//TODO or read_vals ?
    Byte_list send_cols (..);// append write; new if destination does not exist
    Byte_list save_cols (..);// new, overwrite
    //
    Byte_list read_item (..);// read data item(s)
    Byte_list send_item (..);// append write; new if destination does not exist
    Byte_list save_item (..);// new, overwrite
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
