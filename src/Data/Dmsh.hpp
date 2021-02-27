#ifndef FMR_HAS_DMSH_HPP
#define FMR_HAS_DMSH_HPP
#ifndef FMR_HAS_GMSH
#define FMR_HAS_GMSH
#endif

namespace Femera{
class Dmsh final : public Data {
  // Typedefs -----------------------------------------------------------------
  typedef double Optval;
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
  // Member variables ---------------------------------------------------------
  private:
    std::unordered_map<std::string,File_gmsh> file_info={};// key:path/filename
    // Defaults for new Gmsh file.
    Data::Encode encode = Data::Encode::Binary;
    int format          = 1;// .msh (see fmr::detail::format_gmsh_name map)
  // Methods ------------------------------------------------------------------
  protected:
    int init_task (int* argc, char** argv)      final override;
    int exit_task (int err)                     final override;
    std::deque<std::string> get_sims_names ()   final override;
    bool is_this_type (const std::string fname) final override;
    //
    Data::File_info get_file_info (const std::string fname)  final override;
    Data::File_info scan_file_data (const std::string fname) final override;
    //
    Dmsh::File_gmsh open (Dmsh::File_gmsh, Data::Access, Data::Concurrency);
  private:
    int prep  () final override;
    int close () final override;
    bool is_init = false;
    bool is_omp_parallel ();//NOTE Gmsh is not completely thread safe.
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