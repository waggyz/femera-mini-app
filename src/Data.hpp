#ifndef FMR_HAS_DATA_HPP
#define FMR_HAS_DATA_HPP
/** */
#include <string>
#include <deque>
#include <unordered_set>
#include <valarray>
#include <tuple>       //std::pair

#include <unordered_map>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Data : public Work {//TODO change to File
  // typedefs ----------------------------------------------------------------
  public:
    enum class Encode : fmr::Enum_int {Unknown=0,
      ASCII, Binary
    };
    enum class Access : fmr::Enum_int {Unknown=0, Error,//TODO to data::fmr::
      New, Check, Read, Write, Modify, Close            //     in type.hpp
    };//TODO Find, Scan?
    enum class Concurrency : fmr::Enum_int {Error=0, Serial,
      Independent, Collective
    };
    enum class Compress : fmr::Enum_int {None=0, Unknown,//TODO
      ZIP, SZIP
    };
    enum class Scan : fmr::Enum_int {None=0, All, Tree, Dims, Size, Name,
      Gset, Sims, Part, Mesh, Load, Boco, Mtrl
    };
    typedef std::set<Scan> Scan_for;
#if 0
    struct Data_file {
      Data*       data =nullptr;
      std::string name ="";// path/filename
    };
#else
    typedef std::pair<Data*, std::string> Data_file;
#endif
    class File_info {//TODO Info_item
      /* File info and state: specialize for each File driver, e.g. in Dcgn:
       * std::unordered_map<std::string, std::vector<File_cgns>>
       *   file_cgns={};// key: path/filename
       */
      public:
        Data_file            data_file = Data_file ();//Data*,filename
        std::string            version ="";// Library version
        Compress              compress = Compress             :: None        ;
        Data::Encode            encode = Data::Encode         :: ASCII       ;
        fmr::data::Precision precision = fmr::data::Precision :: Float_double;
        Data::Concurrency  concurrency = Data::Concurrency    :: Serial      ;
        Data::Access            access = Data::Access         :: Check       ;
        fmr::data::State         state = fmr::data::State();// file state
      public:
        File_info            ()                 =default;
        File_info            (File_info const&) =default;// copyable
        File_info& operator= (const File_info&) =default;
        virtual ~File_info   ()        noexcept =default;
        };
    typedef std::unordered_map<fmr::Data_id, fmr::Enum_int_vals>
      Data_enum_vals;// key: data ID
    typedef std::unordered_map<fmr::Data_id, fmr::Local_int_vals>
      Data_local_vals;// key: data ID
    typedef std::unordered_map<fmr::Data_id, fmr::Global_int_vals>
      Data_global_vals;// key: data ID
  // member variables --------------------------------------------------------
  public:
    // fmr::perf::IOmeter  time_io = fmr::perf::IOmeter ();//TODO
    //
    Data_enum_vals     enum_vals ={};//key:: data ID
    Data_local_vals   local_vals ={};
    Data_global_vals global_vals ={};
    // These are for caching data that does not yet have a destination.
    // Use in Data classes to avoid multiple reads of the same data.
    // Use sparingly to avoid extra copies of the same data.
    //TODO Must be public for access from derived classes?
    //TODO private: variables; public: ptr/get/set/del, virtual read/save
    //TODO std::unordered_map is not thread safe!
  protected:
    std::string      default_file_name = "new-femera-data";
    std::vector<std::string> file_exts ={"csv"};
    // File extensions are case-insensitive; the default extension is first.
    //
    std::unordered_map<fmr::Data_id, std::vector<Data_file>>//TODO std::set?
      sims_data_file ={};// key: sim ID //TODO private for thread safety?
    //
    std::unordered_set<std::string> sims_names ={};
  private:
    fmr::Dim_int data_hier_max  = 32;// maximum partitioning/grouping depth
    //
    std::deque<std::string> chk_file_names ={};
    std::deque<std::string> out_file_names ={};
    std::deque<std::string> inp_file_names ={};
  // methods -----------------------------------------------------------------
  public:
    std::string print_details () final override;// Data handler information
    //
    // file operations -------------------------------------------------------
    //TODO batch file operations?
  // Keep most file handling operations hidden (protected or private).
    //
    // All data source and destination (file) targets for this run.
    int set_out_file (const std::string fname);//TODO data can go to >1 file
    int add_inp_file (const std::string fname);//TODO return File_info
    std::deque<std::string> get_inp_file_names ();
    std::deque<std::string> get_out_file_names ();
    //
    int chck_file_names ();// Check if files are valid.
    int chck_file_names (const std::deque<std::string> file_names);
    //
    // public interfaces to corresponding *(File_info,...) in derived classes
    File_info get_file_info  (Data_file);// Does not open file.//TODO Name?
    File_info scan_file_data (Data_file);// Root data: sims, gset, mtrl_defs,...
    // Hierarchical data structures can then be accessed by data ops, below.
    //
    virtual int close ();// Close all files.
    int clear ();// Clear all file data.
    //
    int print_sims_file_info (const std::string sim_name);
    // simulation data handling ----------------------------------------------
    fmr::Dim_int get_hier_max ();// Use to detect loops in a hierarchy.
    //TODO batch data operations?
    //
#if 1
    fmr::Data_id make_data_id (const fmr::Data_id tree_root,
      const fmr::Tree_type, const fmr::Tree_path,
      const fmr::Data=fmr::Data::None);
    // above called by below
    fmr::Data_id make_data_id (const fmr::Data_id tree_root,
      const fmr::Data=fmr::Data::None);
    //TODO replace below with above
#endif
    // Sims id : "<sim_name>[:<sim_name>,...]"
    // Part id : "<sim_name>:P<part#>[:P<part#>,...]"
    // Mesh id : "<sim_name>:P<part#>[:P<part#>,...]|M<mesh#>[|M<mesh#>,...]"
    // Gset id : "<sim_name>:G<gset#>[:G<gset#>,...]"// Geometry set
    // Data id : "<sim_name>:G<gset#>[...]:T<data type#>"
    //
    fmr::Local_int get_sims_n ();
    virtual std::deque<std::string> get_sims_names ();//do after scan_file_data
    //
    // int scan (id, Scan_for={Scan::Part,...}, int depth=0);
    int get_local_vals (const fmr::Data_id id, fmr::Local_int_vals &);
    // int read_local_vals (const std::string id, fmr::Local_int_vals &);
    // int save_local_vals (const std::string id, fmr::Local_int_vals &);
    // int free_local_vals (const std::string id, fmr::Local_int_vals &);
    //
  protected:// ---------------------------------------------------------------
    int init_task (int* argc, char** argv) override;
    int exit_task (int err) override;
    //
    Data* get_task_for_file (const std::string fname);//TODO replace w/next
    std::vector<Data*> get_tasks_for_file (const std::string fname);
    //virtual bool has_data_for (const Work_type);//TODO needed?
    //
    // called from public *(Data_file) in base class
    virtual File_info get_file_info (const std::string fname);
    virtual File_info scan_file_data (const std::string fname);
  private:
    int chck () final override;// Not yet used in Data classes.
    int prep () override;
    //
    Work_type   get_file_type (const std::string fname);
    virtual bool is_this_type (const std::string fname);
  public:
    Data           (Proc*);
    Data           (Data const&) =delete;// not copyable
    Data operator= (const Data&) =delete;
    virtual ~Data  ()   noexcept =default;
  protected:
    Data           ();// called implicitly by child constructors
};
}//end Femera namespace

namespace fmr {namespace data {//TODO change all to to fmr::data:: in type.hpp
  static const std::map<Femera::Data::Access,std::string> Access_name {
    {Femera::Data::Access:: Unknown,"uknown access"},//TODO makes sense?
    {Femera::Data::Access::   Error,"access error"},
    {Femera::Data::Access::     New,"new"},
    {Femera::Data::Access::   Check,"check"},
    {Femera::Data::Access::    Read,"read"},
    {Femera::Data::Access::   Write,"write"},
    {Femera::Data::Access::  Modify,"modify"},
    {Femera::Data::Access::   Close,"close"}
  };
} }//end fmr::data:: namespace

#undef FMR_DEBUG
#endif
