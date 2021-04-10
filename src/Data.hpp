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
    enum class Concurrency : fmr::Enum_int {Error=0, Serial,
      Independent, Collective
    };
    enum class Compress : fmr::Enum_int {None=0, Unknown,//TODO
      ZIP, SZIP
    };
    enum class Scan : fmr::Enum_int {None=0, All, Tree, Dims, Size, Name,
      Gset, Sims, Part, Mesh, Load, Boco, Mtrl
    };
#if 0
    typedef std::set<Scan> Scan_for;//TODO Needed?
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
        fmr::data::Access       access = fmr::data::Access    :: Check       ;
        fmr::data::State         state = fmr::data::State();// file state
      public:
        File_info            ()                 =default;
        File_info            (File_info const&) =default;// copyable
        File_info& operator= (const File_info&) =default;
        virtual   ~File_info ()        noexcept =default;
    };
#if 1
    typedef std::unordered_map<fmr::Data_id, fmr::Dim_int_vals>
      Data_dim_vals;// key: data ID
    typedef std::unordered_map<fmr::Data_id, fmr::Enum_int_vals>
      Data_enum_vals;// key: data ID
    typedef std::unordered_map<fmr::Data_id, fmr::Local_int_vals>
      Data_local_vals;// key: data ID
    typedef std::unordered_map<fmr::Data_id, fmr::Global_int_vals>
      Data_global_vals;// key: data ID
    //NOTE std::unordered_map is not thread safe.
    Data_dim_vals       dim_vals ={};//key: data ID
    Data_enum_vals     enum_vals ={};
    Data_local_vals   local_vals ={};
    Data_global_vals global_vals ={};
    //TODO Replace these with Dnew_*, below?
#endif
#if 0
    typedef std::map<fmr::Data,fmr::Dim_int_vals>    Dnew_dim_vals;
    typedef std::map<fmr::Data,fmr::Enum_int_vals>   Dnew_enum_vals;
    typedef std::map<fmr::Data,fmr::Local_int_vals>  Dnew_local_vals;
    typedef std::map<fmr::Data,fmr::Global_int_vals> Dnew_global_vals;
    //
    Dnew_dim_vals    dnew_dim_vals ={};//key: fmr::Data type
    Dnew_enum_vals   dnew_enum_vals ={};
    Dnew_local_vals  dnew_local_vals ={};
    Dnew_global_vals dnew_global_vals ={};
#endif
    // These are for caching data that do not yet have a destination,
    // and to marshal data for distribution.
    // Use in Data classes to avoid multiple reads of the same data.
    // Use sparingly to avoid extra copies of the same data.
    //TODO public: for access to/from derived classes?
    //TODO private: with public: ptr/get/set/del, virtual read/save ?
    // These are NOT thread safe. Should be private in derived classes,
    // so each derived driver can access it's private variables safely?
  public:
  // member variables --------------------------------------------------------
    // fmr::perf::IOmeter  time_io = fmr::perf::IOmeter ();//TODO
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
    // file operations -------------------------------------------------------
    int print_sims_file_info (const std::string sim_name);
    // Keep most file handling operations hidden (protected or private).
    //TODO batch file operations?
    //
    virtual int close ();// Close all files.
    int clear ();// Clear all file data.
    //
    // Data source and destination (file) targets for this run.
    int set_out_file (const std::string fname);//TODO data can go to >1 file
    int add_inp_file (const std::string fname);//TODO return File_info
    std::deque<std::string> get_inp_file_names ();
    std::deque<std::string> get_out_file_names ();
    //
    int chck_file_names ();// Check if files in this->chk_file_names are valid.
    int chck_file_names (const std::deque<std::string> file_names);
    //
    File_info get_file_info  (Data_file);// Does not open file.//TODO chk_?
    File_info scan_file_data (Data_file);// Root data: sims, gset, mtrl_defs,...
    // public interfaces to corresponding *(File_info,...) in derived classes
  protected:
    // called from public *(Data_file) in base class
    virtual File_info get_file_info (const std::string fname);
    virtual File_info scan_file_data (const std::string fname);//TODO chk_?
    // Hierarchical data structures can then be accessed by data ops, below.
  public:
    // simulation data handling ----------------------------------------------
    //TODO batch data operations?
    fmr::Dim_int get_hier_max ();// Use to detect loops in a hierarchy.
    //
    fmr::Data_id make_data_id (const fmr::Data_id base_path,
      const fmr::Tree_type, const fmr::Tree_path branch_path,
      const fmr::Data = fmr::Data::None);
    // above called by abbreviated version below
    fmr::Data_id make_data_id (const fmr::Data_id full_path,
      const fmr::Data = fmr::Data::None);
    //TODO should be S:<sims#>[:S<sims#...]:<sim_name>[:[P/G/...]<#>]... ?
    // Sims id : "<sim_name>[:<sim_name>,...]"
    // Part id : "<sim_name>:P<part#>[:P<part#>,...]"
    // Mesh id : "<sim_name>:P<part#>[:P<part#>,...]|M<mesh#>[|M<mesh#>,...]"
    // Gset id : "<sim_name>:G<gset#>[:G<gset#>,...]"// Geometry set
    // Data id : "<sim_name>:G<gset#>[...]:T<data type#>"
    //
    // Below valid after scan_file_data(..).
    fmr::Local_int get_sims_n ();
    virtual std::deque<std::string> get_sims_names ();
    virtual int make_mesh (const std::string model, const fmr::Local_int ix);
    //
    int get_dim_vals (const fmr::Data_id id, fmr::Dim_int_vals &);
    int get_enum_vals (const fmr::Data_id id, fmr::Enum_int_vals &);
    int get_local_vals (const fmr::Data_id id, fmr::Local_int_vals &);
    int get_global_vals (const fmr::Data_id id, fmr::Global_int_vals &);
#if 0
    int read_local_vals (const std::string id, fmr::Local_int_vals &);
    int save_local_vals (const std::string id, fmr::Local_int_vals &);
    int free_local_vals (const std::string id, fmr::Local_int_vals &);
    int scan (id, Scan_for={Scan::Part,...}, int depth=0);//TODO Needed?
#endif
  protected:// ---------------------------------------------------------------
    //
    int init_task (int* argc, char** argv) override;
    int exit_task (int err) override;
    //
    Data* get_task_for_file (const std::string fname);//TODO replace w/next
    std::vector<Data*> get_tasks_for_file (const std::string fname);
    //virtual bool has_data_for (const Work_type);//TODO needed?
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


#undef FMR_DEBUG
#endif
