#ifndef FMR_HAS_DMSH_HPP
#define FMR_HAS_DMSH_HPP
#ifndef FMR_HAS_GMSH
#define FMR_HAS_GMSH
#endif

#include <mutex>

namespace Femera{
class Dmsh final : public Data {
  // Typedefs -----------------------------------------------------------------
  typedef double Optval;
  typedef const std::basic_string<char> Thrown;
  private:
    class File_gmsh final : public Data::File_info {
    //TODO specialize to handle unmeshed geometry (CAD) files,
      public:
        int format      = 1;
        int mesh_done_d =-1;
      public:
        File_gmsh () {
          precision   = fmr::data::Precision::Float_double;
        }
        File_gmsh (Data::Data_file df) {
          data_file   = df;
          encode      = Data::Encode::Binary;
        };
        File_gmsh (Data::File_info& in) {
          data_file   = in.data_file  ;
          encode      = Data::Encode::Binary;
          version     = in.version    ;
          compress    = in.compress   ;
          encode      = in.encode     ;
          precision   = fmr::data::Precision::Float_double;
          concurrency = in.concurrency;
          access      = in.access     ;
          state       = in.state      ;
        }
    };
    struct Elem_gmsh_info {
      std::string     model ="";
      std::vector<int> tags ={};
      size_t           task = 0;
      int              type = 0;
      Elem_gmsh_info (const std::string m, const std::vector<int> tg,
        const size_t tk, const int ty)
        : model(m), tags(tg), task(tk), type(ty){};
      Elem_gmsh_info                                 () =default;
      Elem_gmsh_info            (Elem_gmsh_info const&) =default;// copyable
      Elem_gmsh_info& operator= (const Elem_gmsh_info&) =default;
      virtual ~Elem_gmsh_info               () noexcept =default;
    };
  // Member variables ---------------------------------------------------------
  private:
    std::mutex liblock {};
    // Below are defaults for new Gmsh file.
    std::unordered_map<std::string,File_gmsh> file_info={};// key:path/filename
    //
    std::unordered_map<fmr::Data_id, Elem_gmsh_info> elem_gmsh_info={};
    int format = 1;// .msh (see fmr::detail::format_gmsh_name map)
    Data::Encode       encode = Data::Encode::Binary;
    fmr::Local_int max_open_n = 2;// Limit number of files open/MPI.
    fmr::Local_int     open_n = 0;
    bool is_init = false;
    bool is_xwin_open = false;
  // Methods ------------------------------------------------------------------
  protected:
    int init_task (int* argc, char** argv)      final override;
    int exit_task (int err)                     final override;
    std::deque<std::string> get_sims_names ()   final override;
    bool is_this_type (const std::string fname) final override;
    //
    Data::File_info get_file_info (const std::string fname) final override;
    Data::File_info scan_file_data (const std::string fname) final override;
    //
    int read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals)
      final override;
    int read_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals &vals)
      final override;
    //
    int make_mesh (const std::string, const fmr::Local_int) final override;
#if 0
    int make_part (const std::string, const fmr::Local_int) final override;
#endif
    int make_part (const std::string, const fmr::Local_int,
        const fmr::Local_int) final override;
    Dmsh::File_gmsh open (Dmsh::File_gmsh,
      fmr::data::Access, Data::Concurrency);
    int close (const std::string model) final override;
  private:
    int scan_model (const std::string name);
    int prep  () final override;
    int close () final override;
    int set_color (const std::string name, const int c[3]);
    bool is_omp_parallel ();//NOTE Gmsh is not completely thread safe.
    int label_gmsh_err (const std::string,const std::string, Thrown);
  public:
    Dmsh           (Proc*,Data*) noexcept;
    Dmsh           ()            =delete;
    Dmsh           (Dmsh const&) =delete;// not copyable
    Dmsh operator= (const Dmsh&) =delete;
    ~Dmsh          ()   noexcept =default;
};
}//end Femera namespace

//end FMR_HAS_DMSH_HPP
#endif
