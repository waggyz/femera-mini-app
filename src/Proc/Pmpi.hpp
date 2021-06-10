#ifndef FMR_HAS_WORK_PMPI_HPP
#define FMR_HAS_WORK_PMPI_HPP
/** */

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Proc;
class Pmpi final : public Proc{
  private:
    Proc::Team_id comm      = 0;//MPI_COMM_WORLD;//TODO could use this->team_id
    const int  mpi_required = 0;//MPI_THREAD_SERIALIZED;
    int        mpi_provided = 0;//TODO rqrd_mpi_thrd, prvd_mpi_thrd ?
  protected:
    int init_task (int* argc, char** argv ) final override;
    int exit_task (int err) final override;
  public:
    int  prep        ()   final override;
    int  chck        ()   final override;
    //
    int  get_proc_id ()   final override;
    int  get_proc_n  ()   final override;
    int  set_proc_n  (int)final override;
    bool is_master   ()   final override;
    bool is_in_parallel () final override;
    //
    inline Proc::Team_id      get_team_id ()final override;// can cast MPI_Comm
    inline int                barrier (    )final override;
    inline std::string        reduce  (std:: string        )final override;
    inline std::valarray<int> gather  (std:: valarray<int> )final override;
    //
    std::string get_host_name ();
    Proc::Team_id  get_comm ();
    int            set_comm ( Proc::Team_id );
    //
    virtual ~Pmpi() noexcept=default;
    Pmpi (Proc*,Data*);
    Pmpi (Pmpi const& )=delete;// not copyable
    Pmpi operator=(const Pmpi&)=delete;
  private:
    Pmpi() = default;
};
inline Proc::Team_id Pmpi:: get_team_id (){ return Proc::Team_id( this->comm );
}
}//end Femera namespace
#if 0
template<typename T>
static inline int fmr_mpi_gather(Femera::Pmpi* M,
  MPI_Datatype type_mpi, T send, T* recv ){
  int send_count=1, recv_count=send_count;
  return MPI_Gather( &send, send_count, type_mpi,
    &recv, recv_count, type_mpi, M->my_master, M->get_comm() );
};
#endif
#undef FMR_DEBUG
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Pmpi.hpp")
// end FMR_HAS_WORK_PMPI_HPP
#endif
