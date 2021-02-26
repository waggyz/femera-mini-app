#include "gtest/gtest.h"
#include "core.h"
#include <unistd.h> // usleep

#if 0
//TODO move to AFTER Plug->init(..) has loaded MPI, OpenMP, and Gtst
//NOTE Tests assume:
// mpiexec -np <mpi_n> -bind-to core -map-by node:pe=<omp_n> ompexec Proc.gtest


inline int fmr_proc_count_base_masters(Femera::Proc* W ){int master_n=0;
  auto proc_stat = W->all_stat();
  if(W->is_master()){// proc_stat gathered to master mpi thread
    int proc_n = W->task.count();
    int thrd_n = int(proc_stat.size())/proc_n;
    for(int i=0;i<thrd_n; i++){
      master_n += proc_stat[proc_n* i ].is_mast;
    }
    return master_n;
  }
  return 1;
}
inline int fmr_proc_count_core_diff(Femera::Proc* W ){int diff_n=0;
  auto proc_stat = W->all_stat();
  if(W->is_master()){// proc_stat gathered to master mpi thread
    int proc_n = W->task.count();
    int thrd_n = int(proc_stat.size())/proc_n;
    for(int i=0; i<thrd_n; i++){
      int id = proc_stat[proc_n*i].logi_id;
      for(int j=1; j<proc_n; j++){
        diff_n += int( proc_stat[proc_n*i+j].logi_id != id );
    } }
  }
  return diff_n;
}
inline int fmr_proc_count_core_reuse(Femera::Proc* W ){int repeat_n=0;
  auto proc_stat = W->all_stat();
  if(W->is_master()){// proc_stat gathered to master mpi thread
    std::set<int> cores_used = {};
    int proc_n = W->task.count();
    size_t thrd_n = proc_stat.size()/proc_n;
    for(size_t i=0; i<thrd_n; i++){
      int core_id = proc_stat[proc_n*i].logi_id;
      repeat_n += int( cores_used.find( core_id ) != cores_used.end());
      cores_used.insert( core_id );
    }
  }
  return repeat_n;
}
inline int fmr_thread_sleep_test(Femera::Proc* W ){int migrate_n=0;
  auto orig_stat = W->all_stat();
  int proc_n = W->task.count();
  int thrd_n = int(orig_stat.size())/proc_n/4;
  int omp_n=1;
  Femera::Proc* omp = W->task.get_task( Femera::Plug_type::Pomp );
  if(omp){ omp_n = omp->get_proc_n(); };
  std::valarray<int> sleep_usec( omp_n );
  int rand_factor = RAND_MAX/16384;
  for(int j=0; j<10; j++){
    for(int i=0; i<omp_n; i++){ sleep_usec[i]= rand()/rand_factor; }
#pragma omp parallel for schedule(static)
    for(int i=0; i<omp_n; i++){
      usleep( sleep_usec[i] );
    }
    auto this_stat = W->all_stat();
    for(int i=0; i<thrd_n; i++){ migrate_n
      += int( this_stat[proc_n* i].phys_id // physical cores
        != orig_stat[proc_n* i].phys_id); }
  }
  return migrate_n;
}
#if 0
int fmr_int_gather_test(Femera::Work W ){
  int s=0; s= W->gather( int(1) ); return s; }
#endif

//Femera::Proc* fmr_proc = new Femera::Proc();
Femera::Proc* fmr_proc = fmr::detail::main->proc;

TEST( Proc, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
TEST( ProcAffinity, OnlyOneBaseMaster){
  EXPECT_EQ( fmr_proc_count_base_masters( fmr_proc ), 1 );
}
TEST( ProcAffinity, SameCoreInProcStack){
  EXPECT_EQ( fmr_proc_count_core_diff( fmr_proc ), 0 );
}
TEST(ProcAffinity, OnlyOneThreadPerCore){
  EXPECT_EQ( fmr_proc_count_core_reuse( fmr_proc ), 0 );
}
#if 1
TEST(ProcAffinity, ThreadsDoNotMigrate){
  //NOTE Looks like this fails with hardware hyperthreading
  EXPECT_EQ( 0, fmr_thread_sleep_test( fmr_proc ) );
}
#endif
TEST( ProcGtst, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#ifdef _OPENMP
TEST( ProcOpenMP, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
#endif

#ifdef FMR_HAS_MPI
TEST( ProcMPI, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}
TEST( ProcMPI, StringReduceTest ){
  EXPECT_STREQ( fmr_proc->reduce(std::string("spam")).c_str(),
    fmr_proc->is_master() ? "spamspam" : "spam" );
}
#if 0
TEST( ProcMPI, IntGatherTest ){
  EXPECT_EQ( fmr_int_gather_test( fmr_proc ),
    fmr_proc->is_master() ? 2 : 0 );// gathered only on master
}
#endif
TEST( ProcMPI, IntValarrayGatherTest ){
  EXPECT_EQ( int(fmr_proc->gather(std::valarray<int>(1,1)).size()),
    fmr_proc->is_master() ? 2 : 0 );// gathered only on master
}

//end FMR_HAS_MPI
#endif
int main(int argc, char** argv ){
  //return fmr_proc->exit( fmr_proc->init( &argc,argv ) );//fmr_proc declared above.
  return fmr::exit (fmr::init (&argc,argv) );
}
#endif
#if 1
int main(int, char**){return 0; }
#endif
