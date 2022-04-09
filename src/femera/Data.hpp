#ifndef FEMERA_TEST_HPP
#define FEMERA_TEST_HPP

#include "Work.hpp"

#include <valarray>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera { namespace data {
  using File_ptrs_t = std::vector <FILE*>;
#if 0
  struct Page_dims {//TODO regular blocked data, local (partitioned) data
    // for file and stream (e.g., stdout) reading and saving
  public:
// Data_ptrs_t data;// data handler; OR use std::vector <Task_type>?
//    std::vector <Task_type> task_type ={};// {task_cast (Plug_type::None)};
//    std::vector <File_type> file_type ={};// {File_type::Unknown};
    File_ptrs_t    file_ptrs ={};
    std::valarray <fmr::Local_int> item_dims ={1};// vals / item
    // vals are all the same type
    fmr::Local_int page_0id  = 0;// global index of first page in this
    fmr::Local_int line_n    = 0;// total lines in this
    fmr::Local_int page_size = 0;// lines / page; a page can be a partition
    fmr::Local_int line_size = 0;// items / line
    fmr::Local_int head_size = 0;// page header size in lines
    fmr::Local_int foot_size = 0;// page footer size in lines
  };
#endif
} }//end femera::data:: namespace
namespace femera {
  template <typename T>
  class Data : public Work {
  private:
    using This_spt = FMR_SMART_PTR<T>;
  public:
    std::string   get_base_abrv ()           noexcept final override;
    //
    fmr::Exit_int init (int*, char**)        noexcept final override;
    fmr::Exit_int exit (fmr::Exit_int err=0) noexcept final override;
#if 0
  private:
    std::unordered_map<std::string,FILE*> open_file_list ={};
  public:
    using Data_name_t = std:vector<Name_t>;
    // Memory operations
    ret::???  add (Data_name_t, ...);// Vals_t vals =nullptr);// in Vals?
    ret::??? size or init (Data_name_t, ...);// Vals_t size =nullptr);// in Vals?
    Vals_t   form or pack or bulk (Data_name_t, ...);// Vals_t vals =nullptr);// in Vals?
    Vals_t    get (Data_name_t);// in Vals?
    ret::???  clr (Data_name_t);// in Vals?
    //
    // I/O operations
    Vals_t   scan (File_name_t);// in File?
    //
    Vals_t   read (Data_name_t);// in Vals?
    Vals_t   sync (Data_name_t);// in Vals?
    ret::??? save (Data_name_t, ...);// Vals_t =nullptr);// in Vals?
    ret::??? send (Data_name_t, ...);// Vals_t =nullptr);// in Vals?
#endif
#if 0
  public:
    //TODO ? File operations are spawned from the local OpenMP master thread, and
    //     ? file methods use vector types for shared-memory access by proc_id, e.g.,
    // local_x = vec_x [proc_id % vec_x.size ()];
    // or better, contiguous blocks of vals for each proc_id
    //
    using Size_list_t = std::vector <fmr::Perf_int>;// bytes in or out
    using Vals_list_t = std::vector <Vals*>;
    //using Form_list_t = std::vector <std::valarray<Bulk_int>>;
    //-------------------------------------------------------------------------
#endif
  public:
    using Sims_list_t = std::deque <std::string>;
    using Path_list_t = std::deque <std::string>;
    //TODO Path_list_t can be a vector to disambiguate it from Sims_list_t?
    //NOTE An empty deque/vector in an argument means all sims or all files.
    // Path_list_t ({""}); means the current working directory in a path method.
    // Text files in Path_list_t may contain a list of model files.
#if 0
  private:
    //std::unordered_map <std::string, Path_list_t> sims_file_list ={};
    //TODO functions that take these are overloaded for std::string&, e.g.,
    //     static inline constexpr Sims_list_t add_sims (std::string& s="") {
    //       return add_sims (Sims_list_t({s});
    //     }
  private:
    class File_name {
    public:
      std::string dir  ="";// absolute or relative
      std::string base ="";// name without path and without (1-)indices.
      std::vector <fmr::Local_int> name_ix={};// 1-indices (model#, part#,...)
      std::string ext  ="";//NOTE includes the dot, e.g., ".txt"
    public:
      File_name (std::string&);
      std::string get_name ();
    };
    using File_list = std::deque <File_name>;
  private:
    Path_list_t Path_list_t ={""};// first is default dir; "" is current working dir
    Path_list_t name_list ={};
    Sims_list_t Sims_list_t ={};
  public:// Data source methods -----------------------------------------------
    //
    Path_list_t  add_path (Path_list_t& ={""});// add to filename search path
    //NOTE add_path ({}); does nothing.
    Path_list_t  add_tree (Path_list_t& ={""});// add dir & subdirs recursively
    //NOTE add_tree ({}); adds subdirs of dirs in the current path recursively.
    Path_list_t  clr_path (Path_list_t& ={});// clear filename search path
    Path_list_t scan_path (Path_list_t& ={});// get model filenames in path
    //
    Path_list_t  add_file (Path_list_t& ={});
    Sims_list_t scan_file (Path_list_t& ={}, Data_type =Data_type::As_needed);
    //                   registers the data in files
    // Data_type::As_needed avoids big data ops and may make false assumptions.
    // E.g., Gmsh (*.msh) files are assumed to contain node & element data.
    //
    fmr::Local_int get_sims_n (Path_list_t& ={});
    fmr::Local_int get_file_n (Path_list_t& ={});
    //
    Sims_list_t  add_sims (Sims_list_t& ={});// register more models
    Sims_list_t  clr_sims (Sims_list_t& ={});// clear model data
    Sims_list_t  get_sims (Sims_list_t& ={});// returns a list of sims loaded
    Size_list_t scan_sims (Sims_list_t& ={}, Data_type =Data_type::As_needed);
    //
    Vals_list_t  get_data (Sims_list_t& ={}, Data_type =Data_type::As_needed);
    Size_list_t  clr_data (Sims_list_t& ={}, Data_type =Data_type::All);
    Vals_list_t  ser_data (..);// returns serialized data
    //
    template <typename V>
    Vals<V>*     get_data (std::vector<Vals_name>,// path to named data item?
      //e.g., {"my_sims","named_sim","named_params","physics","material"}
      Data::Lump_type = Lump_type::All,// Part/Page/Line/Vals/Item
      fmr::Global_int lump_1x = 0);// part/page/line/vals/item (1-indexed)
    // Data read/write methods ------------------------------------------------
    //NOTE (..) means appropriate arguments
    Size_list_t read_data (..);// read whole dataset
    Size_list_t send_data (..);// append, write; new if destination does not exist
    Size_list_t save_data (..);// new, overwrite
    //
    Size_list_t read_page (..);// read data in blocks
    Size_list_t send_page (..);// append, write; new if destination does not exist
    Size_list_t save_page (..);// new, overwrite
    //
    Size_list_t read_line (..);// one line at a time
    Size_list_t send_line (..);// append, write; new if destination does not exist
    Size_list_t save_line (..);// new, overwrite
    //
    Size_list_t read_vals (..);// read data by column (SoA)//TODO or read_cols ?
    Size_list_t send_vals (..);// append write; new if destination does not exist
    Size_list_t save_vals (..);// new, overwrite
    //
    Size_list_t read_item (..);// read data item(s)
    Size_list_t send_item (..);// append write; new if destination does not exist
    Size_list_t save_item (..);// new, overwrite
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
    Data (Work::Core_ptrs_t) noexcept;
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
