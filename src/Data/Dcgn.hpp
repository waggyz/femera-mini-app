#ifndef FMR_HAS_DATA_DCGN_HPP
#define FMR_HAS_DATA_DCGN_HPP
/** */
#ifndef FMR_HAS_CGNS
#define FMR_HAS_CGNS
#endif
//#ifndef FMR_HAS_HDF5
//#define FMR_HAS_HDF5
//#endif

#include <queue>
#include <map>
#include <mutex>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

// Do NOT start names with cg or cgp (upper/lowercase)

namespace Femera{
class Dcgn final : public Data {
  // typedefs -----------------------------------------------------------------
  public:
    enum class Concurrency : fmr::Enum_int {Error=0, Serial,
      Independent, Collective
    };
    enum class File_format : fmr::Enum_int {
      Dcgn_none    = 0,// CG_FILE_NONE,// 0
      Dcgn_ADF1    = 1,// CG_FILE_ADF ,// 1
      Dcgn_HDF5    = 2,// CG_FILE_HDF5,// 2
      Dcgn_ADF2    = 3,// CG_FILE_ADF2 // 3
      Dcgn_unknown = 4
    };
  private:
    typedef int   File_cgid;
    typedef float Ver_cgns_float;
    class File_cgns final : public Data::File_info {
      // specialized member variables
      public:// for access by Dcgn //TODO getters and setters
        // CGNS default comm: Team_id(MPI_COMM_WORLD)
        Proc::Team_id          comm = 0;//TODO use this
        Dcgn::File_cgid   file_cgid = 0;
        Dcgn::File_format    format = Dcgn::File_format::Dcgn_unknown;
      // specialized methods
      public:
        File_cgns () noexcept (false) {// Construct from defaults.
          encode      = Data::Encode::Binary;// CGNS is a binary data format.
        }
        File_cgns (Data::Data_file df) noexcept (false) {// from data_file pair.
          data_file   = df;
          encode      = Data::Encode::Binary;
        }
        File_cgns (Data::File_info& base) noexcept (false) {// base instance.
          data_file   = base.data_file  ;
          encode      = Data::Encode::Binary;
          version     = base.version    ;
          compress    = base.compress   ;
          encode      = base.encode     ;
          precision   = base.precision  ;
          concurrency = base.concurrency;
          access      = base.access     ;
          state       = base.state      ;
        }
        File_cgns            (File_cgns const&) =default;// copyable
        File_cgns& operator= (const File_cgns&) =default;
        virtual   ~File_cgns ()        noexcept =default;
    };
    struct Path_cgns {
      Proc::Team_id comm = 0;
      fmr::Data     type = fmr::Data::Unknown;
      // the rest roughly matches signature of cg_where(..) and cg_golist(..)
      Dcgn::File_cgid     file_cgid = 0;
      int                      base = 0;
      int                      deep = 0;
      std::vector<char*>       labs ={};
      std::vector<int>         inds ={};
      std::vector<std::string> strs ={};
#if 0
      Path_cgns            (Path_cgns const&) =default;// copyable
      Path_cgns& operator= (const Path_cgns&) =default;
      virtual   ~Path_cgns ()        noexcept =default;
#endif
    };
  // Member variables --------------------------------------------------------
  private:
    std::mutex liblock {};
    std::unordered_map<std::string, File_cgns>file_info ={};// key: path/fname
    std::unordered_map<fmr::Data_id, Path_cgns> data_path = {};
    int add_cgns_here (const std::string fname, const fmr::Data_id cid,
      const fmr::Data type);
    //
    // Defaults for new CGNS file.
    File_format             format = File_format::Dcgn_HDF5;
    fmr::data::Precision precision = fmr::data::Precision::Float_double;
    // Defaults for new File_cgns instances.//TODO
    Concurrency        concurrency = Concurrency::Serial;
    Proc::Team_id comm             = 0;// Set in constructors.
  //
    static const short  label_size = 32;//NOTE Does NOT include terminal null.
  // Methods -----------------------------------------------------------------
  // fmr::Tree_path in part tree:
  // Part data: { Zone#, [Zone#,...] }
  // Mesh data: { Zone#, [Zone#,...], [Section#] }
  // Mtrl data: { Zone#, [Zone#,...] }*
  //
  // fmr::Tree_path in geometry set tree:
  // Boco data: { Family#, [Family#,...] } when nFamBC is 1//TODO Check this.
  // Mtrl data: { Family#, [Family#,...] }*
  //
  // Solv data: in CGNS reference state nodes at base or zone level.
  //
  // * when cg_narrays()>0 and an arrayname starts with "fmr::Material"
  //   or matches the name of a known material
  private:
    int prep  () final override;
    int close () final override;
#if 0
    Concurrency get_concurrency (File_cgns, );// TODO
    Concurrency set_concurrency (File_cgns, Concurrency);// TODO
#endif
  protected:
    int init_task (int* argc, char** argv)           final override;
    int exit_task (int err)                          final override;
    bool is_this_type (const std::string fname)      final override;
    //
    Data::File_info get_file_info (const std::string fname)  final override;
    Data::File_info scan_file_data (const std::string fname) final override;
    //
    int read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals)
      final override;
    int read_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals &vals)
      final override;
    //
    std::deque<std::string> get_sims_names ()        final override;
    //
    File_cgns open  (const std::string fname,
      fmr::data::Access, Data::Concurrency);
    int close (const std::string model) final override;
    File_cgns close_file (const std::string fname);
    int make_mesh (const std::string, const fmr::Local_int) final override;
#if 0
    int make_part (const std::string, const fmr::Local_int) final override;
#endif
    int make_part (const std::string, const fmr::Local_int,
      const fmr::Local_int) final override;
  public:
    Dcgn           (Proc*,Data*) noexcept;
    Dcgn           ()            =delete;
    Dcgn           (Dcgn const&) =delete;// not copyable
    Dcgn operator= (const Dcgn&) =delete;
    ~Dcgn          ()   noexcept =default;
  private:
};
}//end Femera namespace
/*
err_func(int is_error, char *errmsg) called automatically in the case of a
warning or error; set with cg_configure(..)

NOTE if it exists, cg_free(StateDescription) MUST be called after closing a
CGNS file.

Femera partition information
Most Femera data are stored in CGNS ReferenceState_t nodes, with a
CGNS StateDescription corresponding to the type of data it has. Reference
state nodes may be stored at base (model) or zone (partition) levels.

Material properties are assigned to CGNS families.
Element types are in sections (Mesh).
A homogeneous Femera Mesh is an intersection of a CGNS family
(material) with a CGNS section. However, synchronization
information (halo nodes, rind) is stored at the CGNS Zone (Part)
level.
*/
#undef FMR_DEBUG
//end FMR_HAS_DATA_DCGN_HPP
#endif
