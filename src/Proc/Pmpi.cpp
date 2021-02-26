#include "mpi.h"
#include "../Main/Plug.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf DEBUG
#endif

namespace Femera {
Pmpi::  Pmpi(Proc* P,Data* D):
comm {Proc::Team_id (MPI_COMM_WORLD) },
mpi_required {int (MPI_THREAD_SERIALIZED) }{
  this->proc=P; this->data=D; this->log=proc->log;
  //this-> comm         = Proc::Team_id( MPI_COMM_WORLD );
  //this-> mpi_required = MPI_THREAD_SERIALIZED;
  this-> work_type = work_cast (Plug_type::Pmpi);
//  this-> base_type = work_cast (Base_type::Proc);
  this-> task_name ="MPI";
  this-> verblevel = 2;
  this-> hier_lv   = 1;
  }
int Pmpi:: prep (){
  this-> version =
     std::to_string( MPI_VERSION )
    +std::string(".")
    +std::to_string( MPI_SUBVERSION );
  return 0;
}
int Pmpi::init_task( int* argc, char** argv ){ int err=0;
  fmr::perf:: timer_resume (&this->time);
  this-> prep ();
  int mpi_is_init=0;
  MPI_Initialized(& mpi_is_init);
  if( !err && !mpi_is_init ){
    err= MPI_Init_thread(argc,&argv, MPI_THREAD_SERIALIZED,&this->mpi_provided);
    MPI_Comm c;
    if( !err ){// good practice: use a copy of MPI_COMM_WORLD
      err= MPI_Comm_dup (MPI_COMM_WORLD, &c); }// exit_task() frees
    if( !err ){
      this->comm = Proc::Team_id(c);
      err= this->chck();
  } }
  if( !err ){this->get_proc_n(); }
  fmr::perf:: timer_pause (&this->time);
  return err;
}
int Pmpi::exit_task (int err){
  fmr::perf:: timer_resume (&this->time);
  int mpi_is_init=0;
  MPI_Initialized(& mpi_is_init);
  if( err>0 ){ this->proc->log-> printf_err("ERROR Femera returned %i\n",err); }
  err=0;// Exit from mpi normally when Femera exits on error.
  if( mpi_is_init ){
    if(this-> comm != Proc::Team_id(MPI_COMM_WORLD) ){
      if(this-> comm){
        MPI_Comm f=MPI_Comm(this-> comm);
        err=MPI_Comm_free(& f );
        if( err>0 ){this->proc->log-> printf_err
          ("ERROR MPI_Comm_free(%lu) returned %i\n",err,f);
    } } }
    err=MPI_Finalize();
      if( err>0 ){this->proc->log-> printf_err
        ("ERROR MPI_Finalize() returned %i\n",err);
  } }
  fmr::perf:: timer_pause (&this->time);
  return err;
}
bool Pmpi::is_in_parallel (){
  int is_init=false, n=0;
  ::MPI_Initialized (& is_init);
  if (is_init) {n=this->get_proc_n ();}
  return is_init && (n > 1);
}
bool Pmpi::is_master(){
  bool this_is_master =( this->get_proc_id() == this->my_master );
  return this_is_master;
}
int Pmpi::chck (){
  if( this->mpi_provided < this->mpi_required ){
    if(this->get_proc_id()==0){
      this->proc->log-> fprintf(this->proc->log-> fmrerr,
        "ERROR: MPI_THREAD_SERIALIZED not supported.\n"); }
    return 1;
  }
  return 0;
}
Proc::Team_id Pmpi:: get_comm (){
  return Proc::Team_id(this-> comm);
}
int Pmpi:: set_comm ( Proc::Team_id c ){int err=0;
  if(this-> comm != Proc::Team_id(MPI_COMM_WORLD)
    && this-> comm != Proc::Team_id(MPI_COMM_NULL) ){
    //err= MPI_Comm_free (&this-> comm );
    MPI_Comm f=MPI_Comm(this-> comm);
    err=MPI_Comm_free(& f );
  }
  this->comm = c;
  return err;
}
int Pmpi::set_proc_n( int ){//TODO Can change size of communicator?
  return this->get_proc_n();
}
int Pmpi::get_proc_n(){int err=0;
  err= MPI_Comm_size(MPI_Comm(this->comm) ,& this->proc_n  );
  if( err ){return 1; }
  return this->proc_n;
}
int Pmpi::get_proc_id(){int err, proc_id=0;
  err= MPI_Comm_rank( MPI_Comm(this->comm),& proc_id );
  if( err ){return 0; }
  return proc_id;
}
std::string Pmpi::get_host_name(){
  int err=0, len=0;
  char hostname [ MPI_MAX_PROCESSOR_NAME ];
  err= MPI_Get_processor_name( hostname,& len );
  if( err ){return std::string(""); }
  return std::string( hostname );
}
inline int Pmpi:: barrier (){int err=0;
  err = MPI_Barrier (MPI_Comm(this-> comm) );
  return err;
}
inline std::valarray<int> Pmpi:: gather (std::valarray<int> vecs ){
  int send_count = int(vecs.size()), recv_count=send_count;
  std::valarray<int> recv={};
  if(this->is_master ()){
    recv.resize (this->proc_n * send_count);
  }
  MPI_Gather (&vecs[0], send_count, MPI_INT,
    &recv[0], recv_count, MPI_INT, this->my_master, MPI_Comm(this->comm) );
  return recv;
}
inline std::string Pmpi::reduce( std::string strings ){
#ifdef FMR_DEBUG
  printf("Pmpi::reduce(string)\n");
#endif
#ifdef FMR_DEBUG
      printf("string size: %lu, length: %lu, strlen: %lu.\n",
        strings.size(),strings.length(),strlen(strings.c_str()));
#endif
  const int tag=0;
  for(int src=0; src < this->proc_n; src++){
    if((this->get_proc_id() == this->my_master) && (src !=this->my_master) ){
      // master recieves
      MPI_Status stat;
      uint len=0;
      MPI_Recv(&len, 1, MPI_UNSIGNED, src, tag, MPI_Comm(this->get_comm()),
        &stat);
      if( len > 0 ){
        std::string str("");
        std::vector<char> tmp(len);
        MPI_Recv(tmp.data(), len, MPI_CHAR, src, tag,
          MPI_Comm(this->get_comm()), &stat);
        str.assign(tmp.begin(), tmp.end());
        strings += str;
    } }
    else{// all others send
      const int dest=this->my_master;
      uint len = uint(strings.size());// bytes
      MPI_Send(&len, 1, MPI_UNSIGNED, dest, tag, MPI_Comm(this->comm));
      if( len > 0 ){ MPI_Send(
        strings.data(), len, MPI_CHAR, dest, tag, MPI_Comm(this->comm)); }
  } }
  return strings;
}

}// end Femera namespace
#undef FMR_DEBUG
