#include "core.h"

#ifdef FMR_HAS_LIBNUMA
#include <numa.h>     // numa_node_of_cpu, numa_num_configured_nodes
#endif

#include <unistd.h>   // getopt, *optarg
#include <cstdlib>    // strtol

#include <cstdio>     // std::printf
#include <sstream>    // std::istringstream
#include <vector>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
Proc::~Proc () noexcept {
  if (this->work_type == work_cast(Base_type::Proc)) {if (this->log) {
    delete this->log;log=nullptr;
} } }
Proc::Proc () {
  this->work_type = work_cast(Base_type::Proc);
  this->task_name ="Process";
#if 1
  this->     proc = this;//TODO safe?
#endif
  this->verblevel = 1;
  this->  hier_lv = 0;
  this->   logi_n = int (std::thread::hardware_concurrency());//physical+logical
  if (this->logi_n < 1) {this->logi_n = 1; }
#ifdef FMR_CORE_N
  this->core_n = FMR_CORE_N ;
#else
  this->core_n = this->logi_n;
#endif
#if 0
      this->core_n =int(std::thread::physical_concurrency());
#endif
  this->proc_n = this->core_n;
}
int Proc::prep () {// Set up the base processing environment stack.
  if (this->work_type == work_cast(Base_type::Proc)) {// Protect against
    // override; only first base Proc makes logger.
    this->proc = this;
    this->task.add (this);
    if (!this->log){
      this->log = new Flog (this);//deleted in ~Proc destructor
  } }
  return 0;
}
#if 0
// At top Proc level there is only one thread, so run_* are all the same.
int Proc::run_sync (Sims* F){int err=0;//run each sim on all threads
  const int sims_n = F->task.count();
    for (int i=0; i<sims_n; i++) {
      err+= F->task.get<Sims>(i)->run () ? 1 : 0;
  }
  return err;
}
int Proc::run_list (Sims* F){
  return this->run_sync (F);
}
int Proc::run_fifo (Sims* F){
  return this->run_sync (F);
}
#else
int Proc::run (Sims* F){int err=0;//run each sim on all threads
  const int sims_n = F->task.count();
    for (int i=0; i<sims_n; i++) {
      err+= F->task.get<Sims>(i)->run () ? 1 : 0;
  }
  return err;
}
#endif
int Proc::get_core_id (){// physical cores
#ifdef FMR_CORE_N
  if (this->core_n > 0){return ::sched_getcpu() % this->core_n; }
#endif
  return get_proc_id ();
}
int Proc::get_logi_id (){
  return ::sched_getcpu();
}
#ifdef FMR_HAS_LIBNUMA
int Proc::get_numa_id (){
  if (numa_available()!=-1){return ::numa_node_of_cpu (::sched_getcpu()); }
  return 0;
}
int Proc::get_numa_n (){
  if (numa_available()!=-1){return ::numa_num_configured_nodes(); }
  return 1;
}
#else
int Proc::get_numa_id (){return 0; }
int Proc::get_numa_n  (){return 1; }
#endif
int Proc::set_proc_n (const int){return this->proc_n;
}
int Proc::get_proc_n (){
  if (this->work_type == work_cast(Base_type::Proc) ){int n=1;
    const int tn=this->task.count();
    for(int i=0; i<tn; i++){
      Proc* P=this->task.get<Proc>(i);
      if (P != nullptr){
        if (P->work_type != work_cast(Base_type::Proc)){
          n *= P->get_proc_n ();
    } } }
    this->proc_n = n;
  }
  return this->proc_n;
}
int Proc::get_proc_id (){int id=0;
  if (this->work_type == work_cast(Base_type::Proc)){
    int n=1;
    const int tn=this->task.count();
    for (int i=0; i<tn; i++){
      Proc* P=this->task.get<Proc>(i);
      if (P != nullptr){
        if (P->work_type != work_cast(Base_type::Proc)){
          id += P->get_proc_id () * n;
          n  *= P->get_proc_n ();
  } } } }
  return id;
}
int Proc::lvl_thrd_n (const int lvl){int n=1;
  const int tn=this->task.count();
  if (lvl >= tn){ return 1; }
  else{
    for (int i=lvl; i<tn; i++){
      Proc* P=this->task.get<Proc>(i);
      if (P != nullptr){
        if (P->work_type != work_cast(Base_type::Proc)){
          n *= P->get_proc_n();
  } } } }
  return n;
}
int Proc::get_hier_lv (){
  return this->hier_lv;
}
int Proc::get_hier_n (){int n=0;
  if (this->work_type == work_cast(Base_type::Proc) ){
    const int tn=this->task.count();
    for(int i=0; i<tn; i++){
      Proc* P=this->task.get<Proc>(i);
      if (P != nullptr){
        n += (P->hier_lv < 0) ? 0 : 1;
  } } }
  return n;
}
bool Proc::is_in_parallel (){
  return (this->get_hier_n () > 1) || (this->get_proc_n () > 1);
}
int Proc::chck (){int err=0;
  int total=1;
  const int n=this->task.count();
  for(int i=0; i<n; i++){
    Proc* P=this->task.get<Proc>(i);
    if (!P){ return 1; }
    if (P->work_type != work_cast (Base_type::Proc)){
      total *= P->get_proc_n();
      P->hier ={};
  } }
  if( total > this->logi_n ){
    this->log->printf_err(
      "WARN""ING Threads requested (%i) exceed total cores available (%i).\n",
      total, this->logi_n);
  }else{
  if (total > this->core_n){
    this->log->printf_err(
      "WARN""ING Threads requested (%i) exceed physical cores available (%i).\n",
      total, this->core_n);
  } }
  if (total < this->core_n){
  this->log->printf_err(
    "NOTE Threads requested (%i) fewer than cores available (%i).\n",
    total, this->core_n);
  }
  const int h=this->get_hier_n ();
  if (h<1){
    err = 1;
    this->log->printf_err(
      "ERR""OR Processing hierarchy has %i levels.\n", h);
  }
  // Set up this->hier map.
  if (err){ return err; }
  this->hier.resize (h);
  for(int i=0; i<n; i++){
    Proc* P=this->task.get<Proc>(i);
    if(P){
      if (P->hier.size()==0){ P->hier.resize (h); }
      const int lvl = P->get_hier_lv ();
      if (lvl >=0 && lvl <h){
        this->hier[lvl] = P;
        P->hier[lvl] = P;
  } } }
  return err;
}
bool Proc::is_master (){bool this_is_master=true;
  const int n=this->task.count();
  for(int i=0; i<n; i++){
    Proc* P=this->task.get<Proc>(i);
    this_is_master &= P->get_proc_id() == P->my_master;
  }
  return this_is_master;
}
std::valarray<Proc::Locality> Proc::get_stat (){
  const int n = this->task.count();
  std::valarray<Proc::Locality> s(n);
  for(int ips=0; ips<n; ips++){
    Proc* P=this->task.get<Proc>(ips);
    s[ips].hier_lv = P->get_hier_lv();
    s[ips].thrd_n  = P->get_proc_n();
    s[ips].thrd_id = P->get_proc_id();
    s[ips].numa_id = P->get_numa_id();
    s[ips].phys_id = P->get_core_id();
    s[ips].logi_id = P->get_logi_id();
    s[ips].is_mast = int(P->is_master());
  }
  return s;
}
std::valarray<Proc::Locality> Proc::all_stat(){
  int nomp=1;
#ifdef _OPENMP
  if (!omp_in_parallel ()){//NOTE Assumes homogeneous thread hierarchy
    Proc* omp = this->task.first<Proc> (Base_type::Pomp);
    if (omp->get_proc_n () > 0){ nomp = omp->get_proc_n (); }
  }
#endif
  const int n=7;
  const int nps = this->task.count();
  std::valarray<int> thrd_stat (nps * nomp * n);// Package as int array for MPI.
  FMR_PRAGMA_OMP (omp parallel for schedule(static) num_threads(nomp))
  for(int iomp=0; iomp < nomp; iomp++){
    for(int ips=0; ips< nps; ips++){
      Proc* P=this->task.get<Proc>(ips);
      const auto row = n*(nps*iomp+ips);
      thrd_stat[row +0]= P->get_hier_lv();
      thrd_stat[row +1]= P->get_proc_n();
      thrd_stat[row +2]= P->get_proc_id();
      thrd_stat[row +3]= P->get_numa_id();
      thrd_stat[row +4]= P->get_core_id();
      thrd_stat[row +5]= P->get_logi_id();
      thrd_stat[row +6]= int(P->is_master());
  } }
  std::valarray<int> out = this-> gather (thrd_stat);
  std::valarray<Proc::Locality> stat( nps * nomp );
  int id=0; const bool do_calc_thrd_id = false;
  for(int iomp=0; iomp < nomp; iomp++){
    for(int ips=0; ips< nps; ips++){
      const int i = nps*iomp+ips;
      const auto row = n*i;
      stat[i].hier_lv = thrd_stat[row +0];
      stat[i].thrd_n  = thrd_stat[row +1];
      if(ips==0 && do_calc_thrd_id){ stat[i].thrd_id = id; }
      else{ stat[i].thrd_id = thrd_stat[row +2]; }
      id++;
      stat[i].numa_id = thrd_stat[row +3];
      stat[i].phys_id = thrd_stat[row +4];
      stat[i].logi_id = thrd_stat[row +5];
      stat[i].is_mast = thrd_stat[row +6];
  } }
  return stat;
}
std::string Proc::print_summary(){
  std::string info(""), line("");
  info += this->log->format_label_line(std::string("Verbosity"),
    std::to_string(this->log-> verbosity )+ std::string("/")
    + std::to_string( FMR_VERBMAX )+ std::string(", detail ")
    + std::to_string(this->log-> detail )+ std::string("/")
    + std::to_string( FMR_VERBMAX )+ std::string(", timing ")
    + std::to_string(this->log-> timing )+ std::string("/")
    + std::to_string( FMR_TIMELVL )+ std::string(" max")
  );
  line = std::to_string(this->core_n);
  if(this->core_n == this->logi_n ){
    line += std::string(" physical =");
  }else{
    line += std::string(" physical, ");
    line += std::to_string(this->logi_n);
  }
  line += std::string(" logical");
  info += this->log->format_label_line (std::string("Cores"), line );
  //
  const int n = Proc::task.count();
  line = std::string("");
  std::string label("");
  for(int i=0; i<n; i++){
    Proc* P = Proc::task.get<Proc>(i);
    if (i == 0){
      label = std::to_string(this->get_proc_n())+" "+P->task_name;
    }
    else{
      if (i > 1){line += std::string(" x "); }// u00d7: times
      line += std::to_string(P->get_proc_n());
      line += std::string(" ")+P->task_name;
#if 0
      if (P->version.size()){line += std::string(" ")+P->version; }
#endif
  } }
  info += this->log->format_label_line (label, line );
  this->log->printf (info);
  return info;
}
std::string Proc::print_details (){
  std::string pre(""), head(""), legend("");
  pre += this->log->print_heading ("Run-time details");
  pre += this->print_summary ();
  pre += this->log->print_center ("Processing hierarchy");
  int omp_proc_n=1;
#ifdef _OPENMP
  if (!omp_in_parallel()){
    Proc* omp = this->task.first<Proc> (Base_type::Pomp);
    if (omp->get_proc_n() > 0){omp_proc_n = omp->get_proc_n(); }
  }
#endif
  //TODO Use all_stat() for this?
  std::string spcs("                          ");
  const int n=this->get_hier_n (), tn=this->task.count ();
  std::vector<int> table_items(n);
  for (int i=0; i<n; i++){
    for (int j=0; j<tn; j++){
      auto P = Proc::task.get<Proc>(j);
      if (P) {if (P->get_hier_lv () == i){
        table_items[i] = j;
  } } } }
  for (int i : table_items ){
#if 0
    auto name = std::string("[")+ std::to_string(i) +"] "
      + Proc::task.get<Proc>(i)->task_name;
#else
    auto name = std::string("[")
      + std::to_string(Proc::task.get<Proc>(i)->get_hier_lv()) +"] "
      + Proc::task.get<Proc>(i)->task_name;
#endif
    if(name.length() >= spcs.length()){
      head += name.substr(1,spcs.length());
    }else{
      head += name + spcs.substr(1,spcs.length()-name.length());
  } }
  head += std::string("\n");
  legend += std::string("|  id/n    phy:log  id/n  | * Master\n");
  legend += std::string("| thread     cpu    numa  |\n");
  std::vector<std::string> info(omp_proc_n,"");
  FMR_PRAGMA_OMP (omp parallel for schedule(static))
  for(int omp_i=0; omp_i < omp_proc_n; omp_i++){
    std::vector<char> buf(16,0);
    // Each openmp thread needs one of these.
    for(int i : table_items ){
      Proc* P=Proc::task.get<Proc>(i);
      std::snprintf(&buf[0],12,"|%4i",P->get_proc_id());
      info[omp_i] += std::string( &buf[0] );
      info[omp_i] += std::string("/");
      std::snprintf(&buf[0],12,"%-4i",P->get_proc_n());
      info[omp_i] += std::string( &buf[0] );
      std::snprintf(&buf[0],12,"%4i",P->get_core_id());
      info[omp_i] += std::string( &buf[0] );
      if( P->is_master() ){ info[omp_i] += std::string("*"); }
      else{ info[omp_i] += std::string(":"); }
      std::snprintf(&buf[0],12,"%-4i",P->get_logi_id());
      info[omp_i] += std::string( &buf[0] );
      std::snprintf(&buf[0],12,"%3i",P->get_numa_id());
      info[omp_i] += std::string( &buf[0] );
      info[omp_i] += std::string("/");
      std::snprintf(&buf[0],12,"%-3i",P->get_numa_n());
      info[omp_i] += std::string( &buf[0] );
    }
    info[omp_i] += std::string( "| \n" );
  }
  std::string table("");
  for (const auto &line : info) {table += line;}// concatenate info
  table = this->reduce (table);
  this->log->printf (head + table + legend);
  return pre + head + table + legend;
}
int Proc:: opt_add (const int c) {
  handled_opts.insert(c);
  return 0;
}
bool Proc:: is_opt_handled (int c) {
  return handled_opts.count(c) > 0;
}
int Proc:: exit_task (int err) {
#if 0
  if (this->proc->log->detail >= this->verblevel) {
    this->proc->log->print_heading ("Done");
  }
  err= this->log->exit (err);
#endif
  if (this->verblevel <= log->timing) {
    this->log->proc_printf ("\"%s\",\"%s\",%lu,%lu\n",
      "Femera", "Proc", this->time.start, fmr::perf::get_now_ns());
  }
  return err;
}
int Proc:: init_task (int* argc, char** argv ){int err=0;
  bool iprint = this->is_master();//, exit_now = false;
#if 1
  this->log->init (argc,argv);
#endif
#if 0
    bool read_db_stdin = false, write_db_stdout = false;
#endif
#if 0
    setlocale(LC_ALL, "C");
    std::setlocale(LC_ALL, "C");// already done during C++ program startup
#endif
    const char* help_txt =
#include "build-data/mini-help.en.inc"
    ;
    const char* copyright_txt =
#include "build-data/copyright.inc"
    ;
    const char* built_by_txt =
#include "build-data/built-by.inc"
    ;
    const char* build_info_txt =
#include "build-data/build-info.inc"
    ;
    const char* build_detail_txt =
#include "build-data/build-detail.inc"
    ;
  FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
  int argc2=argc[0];// Copy getopt globals.
  auto opterr2=opterr; auto optopt2=optopt;
  auto optind2=optind; auto optarg2=optarg;
    opterr = 0; int optchar;
    while ( !err && (optchar = getopt( argc[0], argv,
      "d::v::t::hi:o:m:n:x:SMLXTD+-"//TODO Handle unrecognized opts in the last init.
      )) != -1){
      // i:  -i requires an argument
      // v:: -v argument is optional NOTE gcc extension
      switch (optchar) {
        case '-': {break;}// ignore --, so that --help does the same as -h
#if 0
        case '+':{ read_db_stdin = true; write_db_stdout = true;
          break; }// "-+" same as "-" "+"
#endif
        case 'd': {
          this->log->detail = -1;
          if (optarg!=nullptr) {this->log->detail = atoi(optarg);}
          break;}
        case 't': {
          this->log->timing = -1;
          if (optarg!=nullptr) {this->log->timing = atoi(optarg);}
          break;}
        case 'h': {
          if (iprint) {
            this->log->printf( build_info_txt );
            this->log->printf("\n");
            this->log->printf( help_txt );
          }
          err=-1; break;}
        case 'v':{
          if (optarg==nullptr) {
            if (iprint){
              this->log->printf ( FMR_VERSION"\n");
              if (log->detail>0) {
                this->log->printf (build_info_txt);
                if (strlen (built_by_txt) > 0) {
                  this->log->printf (built_by_txt);
                }
#if 0
                if (strlen (built_by_txt ) > 0) {
                  this->log->printf( modification_txt );}
#endif
                this->log->printf (copyright_txt);
                this->log->printf (build_detail_txt);
              } }
              err=-1; }
          else{// handle --version like --help, but try to parse its arg to int
            char* p=nullptr;
            const long v = strtol (optarg, &p, 10);// base 10
            if (*p) {// Argument is NOT an integer
              if(iprint){ this->log->printf ( FMR_VERSION"\n"); }
              this->log->verbosity=0;
              err=-1; }
            else{// Argument is an integer
              this->log->verbosity = int (v);
            } }
          break;}
#if 0
        // These are handled by other classes.
        case 'n':{ break; }// Set by Pomp::init(argc,argv)
        case 'i':{ break;}// handled by Data
        case 'o':{ break;}// handled by Data
        case 'm':{ break;}// handled by Data
        case 'D':{ break;}// handled by Data
        case 'T':{ break;}// handled by Gtst and ?
        case '0':{ break; }
        case '1':{ break; }
#endif
        case 'x':{this->redo_n = atoi (optarg); break;}
        // More cases
        case '?':{//TODO Handle unrecognized opts later.
          if (is_opt_handled (optopt)){ break; }
          else{
            if (isprint (optopt)){
              if (iprint){this->log->printf_err (
                "ERROR unknown command line option `%c'.\n", optopt); }
            }else{
              if (iprint){this->log->printf_err (
                "ERROR unknown command line option character `\\x%x'.\n",
                optopt); } }
            err=-1;
          } break; }
        default: {break;}
      }
    }// Done parsing command line options.
  // Restore getopt globals.
  argc[0]=argc2; opterr=opterr2; optopt=optopt2; optind=optind2; optarg=optarg2;
  }//end getopt omp master region
  if (err) {return err;}
    //if( exit_now ){ return 0; }
    //NOTE Parsing non-option arguments is done by Data::int( argc,argv )/
#if 0
    // Now parse non-option arguments.
    // args assumed names of database files, directories, stdout, or stdin.
    this->log-> fmrout = stdout;
    int dash_count = 0;
    for(; optind < argc[0]; optind++){
      const auto fname = std::string(argv[ optind ]);
      if( fname == "+"){ write_db_stdout = true; }
      if( fname == "-"){ read_db_stdin = true;
        dash_count+= 1;// two space seperated dashes turns off data to stdout.
        if( dash_count >1 ){ write_db_stdout = false; }
      }
      file_name_queue.push_back( fname );
    }
    if( write_db_stdout ){ file_name_queue.push_back( "+::stdout" ); }
    if( read_db_stdin ){ file_name_queue.push_back( "-::stdin" ); }
    if( write_db_stdout ){ this->log-> fmrout = stderr;
      if(iprint){
      this->log->printf("NOTE Messages have been redirectd to stderr.\n"); } }
    // Redirect stdout msgs to stderr if write_db_stdout==true.
    // Done parsing command line.
#endif
    // Do some sanity chcks on the options selected.
    if (log->verbosity > FMR_VERBMAX ) {
      if (iprint){this->log->printf_err (
        "W""ARNING Requested verbosity -v%i exceeds FMR_VERBMAX %i.\n",
        log->verbosity, FMR_VERBMAX ); }
      log->verbosity = FMR_VERBMAX;
      if (iprint){
        this->log->printf_err (
        "W""ARNING Set verbosity to %i.\n", FMR_VERBMAX ); }
    }
    if (log->detail < 0) {log->detail = log->verbosity;}
    if (log->detail > log->verbosity){
      if (iprint){this->log->printf_err (
        "W""ARNING Requested detail depth -d%i exceeds verbosity %i.\n",
        log->detail, log->verbosity); }
      log->detail = log->verbosity;
      if (iprint) {
        this->log->printf_err(
        "W""ARNING Set detail depth to %i.\n", log->verbosity); }
    }
    if (log->timing < 0) {log->timing = log->verbosity;}
    if (log->timing > FMR_TIMELVL ){
      if (iprint){this->log->printf_err (
        "W""ARNING Requested timing depth -t%i exceeds FMR_TIMELVL %i.\n",
        log->timing, FMR_TIMELVL ); }
      log->timing = FMR_TIMELVL ;
      if (iprint) {
        this->log->printf_err (
        "W""ARNING Set timing detail depth to %i.\n", FMR_TIMELVL ); }
    }
#if FMR_VERBMAX > 0
    if (log->verbosity>0 && log->detail>0) {
      this->log->print_heading ("Femera");
    }
    if (log->verbosity>0) {
      std::istringstream input;
      input.str (build_info_txt);
      std::string line;
      std::getline (input, line);
      this->log->print_label_line (std::string("Verision info"), line.c_str());
      std::getline (input, line);
      this->log->label_printf ("Build info","built on %s\n",line.c_str());
    }
    if (log->verbosity>0 && log->detail>0) {
      if (strlen (built_by_txt) > 0) {
        this->log->label_printf ("Build info","%s\n",built_by_txt);
      }
      this->log->print_center ("Copyright");
      this->log->printf (copyright_txt);
      this->log->print_center ("Build details");
      if (log->detail >= this->verblevel){
        this->log->printf (build_detail_txt);
    } }
    if (log->verbosity >= this->verblevel){
      std::string opts("");
#ifdef FMR_HAS_HDF5
      opts += " HDF5";
#endif
#ifdef FMR_HAS_LIBNUMA
      opts += " libnuma";
#endif
      opts += "\n";
      if (opts.size() > 1) {
        this->log->label_printf ("Femera opts", opts.substr(1).c_str());
    } }
#endif
  if (!err){err= this->chck ();}
  if (log->detail >= this->verblevel) {this->print_details ();}
  return err;
}

}// end Femera namespace
#undef FMR_DEBUG
