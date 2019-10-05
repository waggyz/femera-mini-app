#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <unordered_map>
#include <chrono>
#include <valarray>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "femera.h"
#ifdef HAS_TEST
#include "test.h"
#endif
int main( int argc, char** argv ){
  const float ns=1e-9;
#if VERB_MAX>1
  const float sec=1.0, ms=1e-3,us=1e-6, Meg=1e6, pct=100.0;// k=1e3,Gig=1e9,
#endif
  // defaults
  int comp_n     = 0;//, numa_n=0;
  int verbosity  = 1;
  int step_n     = 1;
  int iter_max   =-1;
  FLOAT_SOLV rtol= 1e-4, atol=-1.0;
  int solv_meth  = Solv::SOLV_CG;
  int solv_cond  = Solv::COND_JACO;
  FLOAT_SOLV solv_init= 0.0;// Start at u = solv_init * exact iso cube solution
  uint simd_n    = 1;
#ifdef _OPENMP
  int numa_n     = 0;
#if VERB_MAX>1
  int halo_mod   = 1;
#endif
#endif
#if VERB_MAX>1
  FLOAT_SOLV test_u=0.001; INT_DOF test_dir=0;
#endif
  bool is_part    = false;
  INT_MESH_PART part_n=0;
  char* bname      = NULL;//FIXME Store this in Femera or Mesh instance?
  const char* iname= NULL;
  // Parse Command Line =============================================
  //FIXME Consider using C++ for parsing command line options.
  opterr = 0; int c;
  while ((c = getopt (argc, argv, "v:pP:h:c:m:s:I:i:r:a:x:y:z:V:d:u:")) != -1){
    // x:  -x requires an argument
    switch (c) {
      case 'c':{ comp_n   = atoi(optarg); break; }
      case 'v':{ verbosity= atoi(optarg); break; }
      case 'V':{ simd_n   = atoi(optarg); break; }
#ifdef _OPENMP
      case 'm':{ numa_n   = atoi(optarg); break; }//FIXME Not yet used.
#if VERB_MAX>1
      case 'h':{ halo_mod = atoi(optarg); break; }
#endif
#endif
      case 'I':{ step_n   = atoi(optarg); break; }
      case 'i':{ iter_max = atoi(optarg); break; }
      case 'r':{ rtol     = atof(optarg); break; }
      case 'a':{ atol     = atof(optarg); break; }
      case 's':{ solv_meth= atoi(optarg); break; }
      case 'd':{ solv_cond= atoi(optarg); break; }
      case 'u':{ solv_init= atof(optarg); break; }
#if VERB_MAX>1
      // Cube test applied displacement
      // FIXME These are not needed
      case 'x':{ test_u   = atof(optarg); test_dir=0; break; }
      case 'y':{ test_u   = atof(optarg); test_dir=1; break; }
      case 'z':{ test_u   = atof(optarg); test_dir=2; break; }
#endif
      case 'p':{ is_part  = true; break;}
      case 'P':{ is_part  = true; part_n = atoi(optarg); break; }
      case '?':{
        if (optopt == 'P'){// specify number of partitions
          //FIXME Does nothing.
          fprintf (stderr, "ERROR Option -%c requires an integer argument.\n",
            optopt); return 1; }
        else if (isprint(optopt)){
          fprintf (stderr,
            "ERROR unknown command line option `-%c'.\n",
            //"WARNING Ignoring unknown command line option `-%c'.\n",
            optopt); return 1; }//FIXME segfaults if continued
        else{
          fprintf(stderr,
            "ERROR unknown command line option character `\\x%x'.\n",
            //"WARNING Ignoring unknown command line option character `\\x%x'.\n",
            optopt); return 1; }
        //return 1;
      }
      default:{ abort(); }
    }
  }
  for (int i = optind; i < argc; i++){
    if(i<(argc-1)){
      fprintf (stderr, "WARNING Ignoring command line option: %s.\n", argv[i]);
    }else{
      bname = argv[i];
    }
  }// Done parsing command line options.
  // Print Info =====================================================
  if( verbosity > 2 ){// Print compile-time information
    //std::vector<int> p={sizeof(FLOAT_MESH),sizeof(FLOAT_PHYS),sizeof(FLOAT_SOLV)};
    for(uint i=0;i<3;i++){
      std::string ps,ns; int p;
      switch(i){
        case(0):{ns="Mesh";p=sizeof(FLOAT_MESH); break; }
        case(1):{ns="Solv";p=sizeof(FLOAT_SOLV); break; }
        case(2):{ns="Phys";p=sizeof(FLOAT_PHYS); break; }
      }
      switch(p){
        case( 2):{ps="half"  ; break; }
        case( 4):{ps="single"; break; }
        case( 8):{ps="double"; break; }
        case(16):{ps="quad"  ; break; }
      }
      std::cout << ns << ": " << ps << " precision" <<'\n';
    }
    std::cout << "Maximum Elements and Nodes: "
      << std::numeric_limits<INT_MESH>::max() <<'\n';
    std::cout << "Maximum Mesh Partitions: "
      << (std::numeric_limits<INT_MESH_PART>::max()-1) <<'\n';
    std::cout << "Maximum Nodes/Element: "
      << size_t(std::numeric_limits<INT_ELEM_NODE>::max()) <<'\n';
    std::cout << "Maximum DOFs/Node: "
      << size_t(std::numeric_limits<INT_DOF>::max()) <<'\n';
    std::cout <<"Verbosity: "<<verbosity<<" / "<< VERB_MAX <<" maximum "<<'\n';
  }
  if( verbosity > VERB_MAX ){
    std::cout <<"WARNING Verbosity "<< verbosity
    <<" requested is more than compiled verbosity maximum "
    << VERB_MAX <<". Downgrading to "<<VERB_MAX <<"."<<'\n';
    verbosity=VERB_MAX;
  }
  // Find Mesh Files ================================================
  if(bname == NULL){
    std::cerr << "ERROR Mesh partition base filename not provided." << '\n';
    return 1; }
  else{
    if(is_part){
#if VERB_MAX>1
    if(verbosity>1){
      printf ("Looking for Femera partitions of %s...\n", bname);
    }
#endif
    bool fok=true; INT_MESH_PART part_i=1;
    while( fok ){
    //for(int part_i=1; part_i<(part_n+1);part_i++){
      std::stringstream ss;
      ss << bname << "_" << part_i << ".fmr" ;
      std::string pname = ss.str();
      // Check if can open file.
      FILE* pfile;
      pfile = fopen(pname.c_str(),"r");
      if (pfile==NULL){
        fok = false;
      }else{
        part_i++; iname=bname;
#if VERB_MAX>3
        if(verbosity>3){
          std::cout << "Found " << pname << "..." <<'\n';
        }
#endif
        fclose (pfile);
        }
      }
    part_n = part_i-1;
    }else{
      std::stringstream ss;
      ss << bname ;
      std::string pname = ss.str();
      // Check if can open file.
      FILE * pfile;
      pfile = fopen(pname.c_str(),"r");
      if (pfile==NULL){
        ss << ".fmr" ;
        pname = ss.str();
        pfile = fopen(pname.c_str(),"r");
        if (pfile==NULL){
          std::cout << "ERROR opening " << pname << " for reading." << '\n';
          return 1; }
        else{ part_n=1; pname=pname.c_str();
#if VERB_MAX>3
          if(verbosity>3){
          std::cout << "Found " << pname << "..." << '\n';
          };
#endif
          iname = pname.c_str();
          fclose (pfile); }
      }else{
        part_n=1; iname=pname.c_str();
#if VERB_MAX>3
        if(verbosity>3){
        std::cout << "Found " << pname << "..." << '\n';
        }
#endif
        fclose (pfile);
        }
      }
    }
  if(part_n>0){
    if(part_n>1){
#if VERB_MAX>1
  if(verbosity>1){
    printf ("Found %u mesh partitions.\n", part_n); }
#endif
    };
  }else{
    std::cerr << "ERROR No mesh partition files could be opened for reading."
      << '\n'; 
    return 1;
  }
  // Read and Setup =============================================
  int iter=0;
#if VERB_MAX>0
  float read_sec=0.0,init_sec=0.0,loop_sec=0.0;
#endif
  int iter_info_n = 1;
#ifdef _OPENMP
  if( comp_n <1){ comp_n = omp_get_max_threads(); }//omp_get_max_threads();
  if( numa_n==0){ numa_n = comp_n / 2; }//FIXME just a guess...and not yet used
  if( numa_n <2 ){ numa_n=2; }
  if( comp_n >int(part_n) ){ comp_n=int(part_n); }
  if( numa_n >int(part_n) ){ numa_n=int(part_n); }
#if VERB_MAX>1
  if(verbosity>1){
  std::cout <<"Parallel OpenMP " << "using "  <<comp_n<< " threads";
    if(auto c=std::getenv("OMP_PROC_BIND")){ std::cout <<" "<< c; }
    if(auto c=std::getenv("OMP_PLACES")){ std::cout << " to " <<c; }
  if(simd_n>1){std::cout <<" and starting physics block width of "<<simd_n; }
  std::cout  <<"..."<<'\n';
    //<<comp_n<< " compute and "<<numa_n<<" NUMA threads..."<<'\n';
  }
#endif
#endif
  if(comp_n<1){ comp_n = 1; }
  int part_0=1;//FIXME unpartitioned mesh in mesh_part[0]
#if VERB_MAX>1
  if(verbosity>1){
    std::cout << "Reading and setting up "<<(part_n-part_0+1)<<" partitions";
  if(verbosity!=2){ std::cout <<"..." <<'\n'; }
  }
#endif
  //printf("****** SOLVER: %i: %i,%i,%i\n",solv_meth,
  //  Solv::SOLV_GD,Solv::SOLV_CG,Solv::SOLV_CR);
  Mesh* M;
  switch( solv_meth ){
    case(Solv::SOLV_CG):{ M=new HaloPCG(part_n+part_0,iter_max,rtol); break; }
    case(Solv::SOLV_CR):{ M=new HaloPCR(part_n+part_0,iter_max,rtol); break; }
    case(Solv::SOLV_NG):{ M=new HaloNCG(part_n+part_0,iter_max,rtol); break; }
    default            :{ M=new HaloPCG(part_n+part_0,iter_max,rtol); }
  }
  M->solv_cond=solv_cond;
  M->cube_init=solv_init;
  M->base_name=iname;
  M->comp_n=comp_n;
  M->simd_n=simd_n;
  M->glob_atol = atol; M->glob_ato2 = atol*atol;
  M->verbosity=verbosity;
  M->time_secs.resize(10);
  M->load_step_n=step_n; M->step_scal=1.0/FLOAT_SOLV(step_n);
#if VERB_MAX>0
  std::chrono::high_resolution_clock::time_point
    read_start, setu_done, init_done;
  std::chrono::nanoseconds read_time, init_time;// setu_time,
  { auto sometime=std::chrono::high_resolution_clock::now();
    read_time = std::chrono::duration_cast<std::chrono::nanoseconds>
      (sometime-sometime);
    //read_time=read_time;// setu_time=read_time;
  }
  read_start = std::chrono::high_resolution_clock::now();
#endif
  M->Setup();
//if VERB_MAX>1
  {// scope local variables
  //int sugg_max=3000;
  iter_info_n =   1;// sugg_max = M->udof_n;
  if      ( M->udof_n>int(1e8) ){ iter_info_n =1000;// sugg_max =M->udof_n/1000;
  }else if( M->udof_n>int(1e4) ){ iter_info_n = 100;// sugg_max =M->udof_n/10;
  }else if( M->udof_n>int(1e2) ){ iter_info_n =  10;// sugg_max =M->udof_n/10;
  }else                         { iter_info_n =   1;// sugg_max =M->udof_n; 
  }
  if(iter_max<0){
    iter_max = M->udof_n / iter_info_n *10;
    if( iter_max > int(M->udof_n) ){ iter_max=M->udof_n; }
#if 0
    iter_max*= M->load_step_n;
#endif
  }
  }// end variable scope
//endif
#if VERB_MAX>0
  setu_done = std::chrono::high_resolution_clock::now();
  read_time = std::chrono::duration_cast<std::chrono::nanoseconds>
    (setu_done-read_start);
  read_sec=float(read_time.count())*1e-9;
  if(verbosity==1){
  std::cout<< M->elem_n<<","<<M->node_n<<","<<M->udof_n<<","<<(part_n-part_0+1);
  }
#endif
#if VERB_MAX>1
  if(verbosity>1){
  std::cout <<"System:     "<<M->elem_n<<" Elems, "
    <<M->node_n<<" Nodes, "<<M->udof_n<<" DOF "<<'\n';
  std::cout <<"Bounds:    [";
  for(int i=0; i<3; i++){ std::cout<<M->glob_bbox[i]; if(i<2){std::cout<<",";} }
  std::cout <<"] [";
  for(int i=3; i<6; i++){ std::cout<<M->glob_bbox[i]; if(i<5){std::cout<<",";} }
  std::cout<<"]"<<'\n';
  printf("Read and set up                         in %f s\n", read_sec );
  if(verbosity>1){
    std::cout<< " Initializing:  ";
    switch(solv_cond){
      case(Solv::COND_NONE): std::cout<< "no"; break;
      case(Solv::COND_JACO): std::cout<< "Jacobi"; break;
      case(Solv::COND_ROW1): std::cout<< "element row one-norm"; break;
      case(Solv::COND_STRA): std::cout<< "element strain"; break;
      default: std::cout<< "unknown";
    }
    std::cout<< " preconditioner..." <<'\n';
  }
  }
#endif
  // Solve parallel =================================================
#ifdef _OPENMP
#if VERB_MAX>2
  if(halo_mod!=1){
    std::cout<<"with halo updates every "
    <<halo_mod<< " iterations..."<<'\n'; }
    //<<comp_n<< " compute and " <<numa_n<< " NUMA threads." <<'\n';
#endif
#endif
  // Initialize ---------------------------------------------------
  {// load step scope
  int step_n=M->load_step_n;//NOTE M->load_step is 1-indexed
  for(M->load_step=1; M->load_step <= step_n; M->load_step++){
    iter=0;
  {// load step init scope
    if( M->load_step > 1 ){
      setu_done=std::chrono::high_resolution_clock::now();
      read_sec=0.0;
    }
    M->time_secs=0.0;
    M->Init();
    if( (step_n > 1) & (M->load_step == 1) & (M->glob_atol<=0.0) ){
      M->glob_atol = std::sqrt(M->glob_rto2);
      M->glob_ato2 = M->glob_rto2; }// Stick with initial relative tolerance
    if( M->glob_atol>0.0 ){//FIXME Move to Init?
      M->glob_rto2=M->glob_ato2; }
#if VERB_MAX>1
    if( verbosity > 1 ){ if( step_n > 1 ){
      printf("    Load Step:  %i of %i scaled by%5.2f and\n",
        M->load_step, M->load_step_n,
        M->step_scal * FLOAT_SOLV(M->load_step) ); }
    }
#endif
#if VERB_MAX>0
    if(verbosity>0){
    init_done = std::chrono::high_resolution_clock::now();
    init_time = std::chrono::duration_cast<std::chrono::nanoseconds>
      (init_done-setu_done);
    }
    init_sec =float(init_time.count())*ns;
#endif
#if VERB_MAX>2
    float prec_sec =M->time_secs[0];
    float gmap_sec =M->time_secs[1];
    float gat0_sec =M->time_secs[2];
    float gat1_sec =M->time_secs[3];
    float scat_sec =M->time_secs[4];
    float solv_sec =M->time_secs[5];
    float work_sec = prec_sec+gmap_sec+gat0_sec+gat1_sec+scat_sec+solv_sec;
    //
    float sc = ms; const char* ss="ms";
    if( work_sec < 100.0*ms ){ sc=us; ss="μs"; }// U+03BC
    else if( work_sec > 100.0*sec ){sc=sec; ss=" s"; }
    //
    if(verbosity>2){
      if(prec_sec/init_sec/float(comp_n)*pct > 0.03){//FIXME
      printf("%9.0f %s  %6.2f%% Combined Thread Preconditioner Time\n",
        prec_sec/sc, ss, prec_sec/init_sec/float(comp_n)*pct); };
      printf("%9.0f %s  %6.2f%% Combined Global Node ID Map Init Time\n",
        gmap_sec/sc, ss, gmap_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Gather Time\n",
        (gat0_sec+gat1_sec)/sc, ss,
        (gat0_sec+gat1_sec)/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Scatter Time\n",
        scat_sec/sc, ss, scat_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Physics and Solve Init Time\n",
        solv_sec/sc, ss, solv_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Init Sync and Overhead Time\n",
        (init_sec-work_sec/float(comp_n))/sc, ss,
        (init_sec-work_sec/float(comp_n))/init_sec*pct );
    }
#endif
#if VERB_MAX>1
    if(verbosity>1){
    std::cout << "  Starting at:  ";
    if(M->load_step >1){
      if( M->next_scal > 0.0 ){
        if( M->next_scal == 1.0 ){
          std::cout<<"linear extrapolation of previous solution"; }
        else{
          std::cout<<M->next_scal<<"x from the previous load step"; }
      }else{
        std::cout<<"previous load step solution"; }
    }else{
      if(M->cube_init==0.0){ std::cout<<"zero"; }
      else if(M->cube_init==1.0){ std::cout<<"isotropic block solution"; }
      else{ std::cout<<M->cube_init<<" times isotropic block solution"; }
    }
    std::cout <<" with"<<'\n'<<"  Boundary at: [";
    //FIXME Should include thermal, too
    for(int i=0; i<3; i++){
      std::cout<< M->glob_bmax[i] * M->step_scal * FLOAT_SOLV(M->load_step);
      if(i<2){std::cout<<","; } }
    std::cout <<"], then"<<'\n' << " Iterating to: <";
    if( M->glob_atol > 0.0 ){
       std::cout <<M->glob_atol<<" absolute ";
    }else{
       std::cout <<M->glob_rtol<<" relative ";
    }
    std::cout <<"tolerance or"<<'\n';
    std::cout <<"  Stopping at:  "<<iter_max
      <<" "<<M->meth_name<<" iterations..."<<'\n';
    std::cout <<"Solving..."<<'\n';
    const char* c="=";
    if(M->glob_chk2 < M->glob_rto2){ c="<"; }
    else if(M->glob_chk2 > M->glob_rto2){ c=">"; }
    printf("     init ||R||%9.2e %s%9.2e tol in %f s\n",
      std::sqrt(M->glob_chk2), c, std::sqrt(M->glob_rto2),init_sec );
    }
#endif
  }// end init scope
  {// iter scope
    M->time_secs=0.0;
    // Iterate ------------------------------------------------------
    auto loop_start = std::chrono::high_resolution_clock::now();
    do{ M->Iter(); iter++;
      //for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      //  Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->mesh_part[part_i];
      //  S->Init(); S->Iter();
      //} M->Init();
#if VERB_MAX>1
      if(verbosity>1){
        if(!((iter) % iter_info_n) | (iter<2)){
          float iter_sec=M->time_secs[5];
          printf("%9i ||R||%9.2e @ %.0f iter/s\n",
            iter, std::sqrt(M->glob_chk2),
            float(iter)/iter_sec*float(comp_n) );
        }
        if(M->glob_res2 <= 0.0){
            printf("%9i ||R||%9.2e NCG Reset\n",
              iter, std::sqrt(M->glob_chk2) );
        }
      }
#endif
#if VERB_MAX>10
      if(!((iter) % iter_info_n) ){// Print partition Residuals
        for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
          Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->mesh_part[part_i];
          printf("Part %4i ||R||%9.2e\n", part_i, std::sqrt(S->loca_res2));
        } }
#endif
    }while( (iter < iter_max) & (M->glob_chk2 > M->glob_rto2) );
    // End iteration loop ===========================================
#if VERB_MAX>0
    auto loop_done = std::chrono::high_resolution_clock::now();
    auto loop_time = std::chrono::duration_cast<std::chrono::nanoseconds>
      (loop_done - loop_start);
    loop_sec=float(loop_time.count())*ns;
    if(verbosity==1){
    std::cout <<","<<iter<<","<<iter_max;
    std::cout <<","<<std::sqrt(M->glob_chk2)<<","<< std::sqrt(M->glob_rto2);
    std::cout <<","<<comp_n;
    std::cout <<","<<read_sec<<","<<init_sec<<","<<loop_sec;
    std::cout <<","<<float(M->udof_n)*float(iter)/loop_sec<<'\n';
    }
#endif
#if VERB_MAX>1
    float sc = ms; const char* ss="ms";
    if(verbosity>1){
    if( loop_sec < 100.0*ms ){ sc=us; ss="μs"; }
    else if( loop_sec > 100.0*sec ){sc=sec; ss=" s"; }
    const char* c="=";
    if(M->glob_chk2 < M->glob_rto2){ c="<"; }
    else if(M->glob_chk2 > M->glob_rto2){ c=">"; }
    printf("%9i ||R||%9.2e %s%9.2e tol in %f s\nDone.\n", iter,
      std::sqrt(M->glob_chk2), c, std::sqrt(M->glob_rto2), loop_sec );
    }
#endif
#if VERB_MAX>1
    if(verbosity>1){
    float phys_sec=M->time_secs[0];
    float gat0_sec=M->time_secs[1];
    float gat1_sec=M->time_secs[2];
    float scat_sec=M->time_secs[3];
    float solv_sec=M->time_secs[4];
    float iter_sec=M->time_secs[5];
    //
    printf("%9.0f %s  %6.2f%% Combined Thread Physics Time\n",
      phys_sec /sc, ss, phys_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Halo Gather Initialize Time\n",
      gat0_sec /sc, ss, gat0_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Halo Gather Summation Time\n",
      gat1_sec /sc, ss, gat1_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Halo Scatter Time\n",
      scat_sec /sc, ss, scat_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Solve Time\n",
      solv_sec /sc, ss, solv_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Work Time\n",
      iter_sec /sc, ss, (phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec)
      /iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Sync and Overhead Time\n",
      (iter_sec-(phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec)) /sc, ss,
      (iter_sec-(phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec))
      / iter_sec*pct );
    printf("%9.0f %s  %6.2f%% Wall Clock Solve Time and Load Balance Percentage\n",
      loop_sec /sc, ss, (iter_sec/comp_n)/loop_sec *pct );
    //
    //printf("Effective FPU Performance\n");
    printf("    Physics:%8.2f  GFLOP/s @ %4.0f/%.0f /elem =%5.2f FLOP/Byte\n",
      float(M->phys_flop)*float(iter)*float(comp_n)/phys_sec/1e9,
      float(M->phys_flop)/float(M->elem_n),float(M->phys_band)/float(M->elem_n),
      M->phys_flop/M->phys_band );
    printf("      Solve:%8.2f  GFLOP/s @ %4.0f/%.0f /DOF  =%5.2f FLOP/Byte\n",
      float(M->solv_flop)*float(iter)*float(comp_n)/solv_sec/1e9,
      float(M->solv_flop)/float(M->udof_n),float(M->solv_band)/float(M->udof_n),
      M->solv_flop/M->solv_band );
    //
    printf("Performance:%8.2f  MDOF/s\n",
      float(M->udof_n)*float(iter)/loop_sec /Meg );
    }
#endif
  }// end iter scope
#if 0
// Output integration point strains and stresses =====================
#pragma omp parallel num_threads(comp_n)
{
    for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
#pragma omp critical(minmax)
{
    Y->ElemStrainStress( std::cout, E, S->part_u );
}
    }//end part loop
}
#endif
#ifdef HAS_TEST
    // Check solution ===============================================
#if VERB_MAX>1
    if( verbosity>1 ){
    // Displacement errors ------------------------------------------
    Phys::vals errtot; int node_d=3;
    auto tY = std::get<1>(M->mesh_part[1]);
    if(tY->ther_cond.size()>0){ node_d=4; }
    errtot.resize(3*(node_d+1)+1); errtot=0.0;// printf("NODED: %i\n",node_d);
    for(int i= 0; i< 1*(node_d+1); i++){ errtot[i] = 99e99; }
    for(int i= 2*(node_d+1); i< 3*(node_d+1); i++){ errtot[i] =-99e99; }
    //FLOAT_PHYS scale=1.0,smin= 99e9,smax=-99e9;//FIXME only works for cubes
    FLOAT_PHYS scax=1.0,minx= 99e9,maxx=-99e9;
    FLOAT_PHYS scay=1.0,miny= 99e9,maxy=-99e9;
    FLOAT_PHYS scaz=1.0,minz= 99e9,maxz=-99e9;
    FLOAT_PHYS youn_voig=0.0, ther_pres=0.0, test_amt=0.0;
#pragma omp parallel num_threads(comp_n)
{
#pragma omp for schedule(static)
    for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
#pragma omp critical(minmax)
{
      //smin=std::min( smin, E->node_coor.min() );
      //smax=std::max( smax, E->node_coor.max() );
      Phys::vals t = E->node_coor[std::slice(0,E->node_n,3)];
      minx=std::min(minx, t.min() );
      maxx=std::max(maxx, t.max() );
      t = E->node_coor[std::slice(1,E->node_n,3)];
      miny=std::min(miny, t.min() );
      maxy=std::max(maxy, t.max() );
      t = E->node_coor[std::slice(2,E->node_n,3)];
      minz=std::min(minz, t.min() );
      maxz=std::max(maxz, t.max() );
}
  INT_MESH n; INT_DOF f; FLOAT_PHYS v;
  for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
    if( (f<3) & (std::abs(v) > std::abs(test_amt)) ){
#pragma omp critical(bcs)
{     test_dir = f; test_amt = v; }
    }
  }
      test_u=test_amt * S->load_scal;
#pragma omp critical(youngs)
{
    FLOAT_PHYS A=0.0,B=0.0,C=0.0;
    //if(Y->mtrl_matc.size()==3){
    if(Y->mtrl_dirs.size()==0){
      A=Y->mtrl_matc[0]; B=Y->mtrl_matc[1]; C=Y->mtrl_matc[2];
    //}else if(Y->mtrl_matc.size()==9){
    }else{
      A=(Y->mtrl_matc[0]+Y->mtrl_matc[1]+Y->mtrl_matc[2])/3.0;
      B=(Y->mtrl_matc[3]+Y->mtrl_matc[4]+Y->mtrl_matc[5])/3.0;
      C=(Y->mtrl_matc[6]+Y->mtrl_matc[7]+Y->mtrl_matc[8])/3.0;
    }
    youn_voig=(A-B+3.0*C)*(A+2.0*B)/(2.0*A+3.0*B+C);
    //printf("c11:%e, c12:%e, c44:%e\n",A,B,C);
    //printf("ELEM NODES: %u\n",uint(E->elem_conn_n));
    //printf("mtrl_prop size: %u\n",uint(Y->mtrl_prop.size()));
}
    }
#pragma omp single
{
    //scale = 1.0/(smax-smin);
    scax = 1.0/(maxx-minx);
    scay = 1.0/(maxy-miny);
    scaz = 1.0/(maxz-minz);
}
}//end parallel region
    if( M->load_step==1 ){
    printf("Solution Error (Compared to Isotropic)\n");
    Test* T = new Test();//FIXME should this be inside the parallel region?
#pragma omp parallel num_threads(comp_n)
{
#pragma omp for schedule(static)
    for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
      const int Dm=3; const int Dn=Y->node_d;
      Phys::vals errors;
      FLOAT_PHYS nu=Y->mtrl_prop[1];
      Phys::vals coor(E->node_coor.size());
      const auto Nn=E->node_n;
      switch(test_dir){
      case(1):{
        coor[std::slice(0,Nn,Dm)]=E->node_coor[std::slice(1,Nn,Dm)];
        coor[std::slice(1,Nn,Dm)]=E->node_coor[std::slice(2,Nn,Dm)];
        coor[std::slice(2,Nn,Dm)]=E->node_coor[std::slice(0,Nn,Dm)];
        break;}
      case(2):{
        coor[std::slice(0,Nn,Dm)]=E->node_coor[std::slice(2,Nn,Dm)];
        coor[std::slice(1,Nn,Dm)]=E->node_coor[std::slice(0,Nn,Dm)];
        coor[std::slice(2,Nn,Dm)]=E->node_coor[std::slice(1,Nn,Dm)];
        break;}
      case(0):
      default:{ coor = E->node_coor; break;}
      }
      for(uint i=0;i<(Nn*Dm);i+=Dm){
        coor[i+0]-=minx; coor[i+1]-=miny; coor[i+2]-=minz;
        coor[i+0]*=scax; coor[i+1]*=scay; coor[i+2]*=scaz;
      }
      //test_u=Y->udof_magn[test_dir];
      FLOAT_PHYS test_T=Y->udof_magn[3];
      //
      Solv::vals norm_u(Nn*Dn);
      if(Dn<4){
        for(INT_MESH i=0;i<Nn;i++){
          for(int j=0;j<Dn;j++){
            norm_u[Dn* i+j] = S->part_u[Dn* i+j] / FLOAT_SOLV(test_u);
          }
        }
      }else{// Adjust for thermal expansion
        //FLOAT_PHYS kappa=(Y->ther_expa[0]+Y->ther_expa[1]+Y->ther_expa[2])/3.0;
        FLOAT_PHYS kappa = Y->ther_expa[0];//FIXME
        FLOAT_PHYS ther_u = scax * kappa * test_T;// Unconstrained expansion displacement
        //FLOAT_PHYS delt_u = ther_u - test_u;
        ther_pres = youn_voig * ther_u / scax;// Thermal pressure @ 0 displacement
        //printf("KAPPA: %f, THERU: %f, THERP: %f\n", kappa,ther_u,ther_pres);
        for(INT_MESH i=0;i<Nn;i++){
          // Don't adjust x displacements
          norm_u[Dn* i+0] = S->part_u[Dn* i+0] / FLOAT_SOLV(test_u);
          // Adjust transverse displacements
          for(int j=1;j<Dm;j++){
            norm_u[Dn* i+j] =(
              S->part_u[Dn* i+j]
              - kappa * test_T * coor[Dm* i+j]// thermal expansion
              - nu * ther_pres / youn_voig * coor[Dm* i+j]//pressure-induced expansion
              )/ FLOAT_SOLV(test_u);
          }
          norm_u[Dn* i+Dm] = S->part_u[Dn* i+Dm] / FLOAT_SOLV(test_T);
        }
      }
      //errors.resize(3*(Dn+1)+1); errors=0.0;
      T->CheckCubeError( errors, nu, coor, norm_u );
#pragma omp critical(errs)
{
      for(int i= 0*(Dn+1); i< 1*(Dn+1); i++){ errtot[i] = std::min(errtot[i],errors[i]); };
      for(int i= 1*(Dn+1); i< 2*(Dn+1); i++){ errtot[i]+= errors[i]; };
      for(int i= 2*(Dn+1); i< 3*(Dn+1); i++){ errtot[i] = std::max(errtot[i],errors[i]); };
      errtot[3*(Dn+1)]+=errors[3*(Dn+1)];
#if VERB_MAX > 2
      if(verbosity>2){
        if(S->udof_n<300){
          printf("------ Node Coordinates ---------------- Displacements -------");
          if(Dn>3){ printf("  Temperature"); }
          printf("\n");
          for(uint i=0;i<Nn;i++){
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",E->node_coor[Dm* i+j]); }
            printf(" | ");
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",S->part_u[Dn* i+j]); }
            if(Dn>Dm){ printf("  %+9.2e",S->part_u[Dn* i+Dm]); }//FIXME Temperature
            printf("\n");
          }
        }
      }
#endif
#if VERB_MAX > 3
      if(verbosity>3){
        if(S->udof_n<300){
          printf("---- Normalized Coordinates ------ Normalized Displacements --");
          if(Dn>3){ printf("  Temperature"); }
          printf("\n");
          for(uint i=0;i<Nn;i++){
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",coor[Dm* i+j]); }
            printf(" | ");
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",norm_u[Dn* i+j]); }
            if(Dn>Dm){ printf("  %+9.2e",S->part_u[Dn* i+Dm]/test_T); }
            printf("\n");
          }
        }
      }
#endif
#if VERB_MAX > 3
      if(verbosity>3){
        if(S->udof_n<300){
          printf("------- Node Coordinates -------------- Preconditioner -------");
          if(Dn>3){ printf("  Temperature"); }
          printf("\n");
          for(uint i=0;i<Nn;i++){
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",E->node_coor[Dm* i+j]); }
            printf(" | ");
            for(int j=0;j<Dm;j++){ printf("%+9.2e ",S->part_d[Dn* i+j]); }
            if(Dn>Dm){ printf("  %+9.2e",S->part_d[Dn* i+Dm]); }
            printf("\n");
          }
        }
      }
#endif
}
#if VERB_MAX>2
      if(verbosity>2){
#pragma omp critical(print)
{
      printf(" ux        uy        uz        mag       ");
      if(node_d>3){ printf("Temp      Normalized Error Part %i", part_i ); }
      else{ printf("Normalized Error in Partition %i", part_i ); }
      for(int i=0;i<int(errors.size());i++){
        if(!(i%(node_d+1))){
          if(     i==1*(node_d+1)){ printf(" Min"); }
          else if(i==2*(node_d+1)){ printf(" Avg"); }
          else if(i==3*(node_d+1)){ printf(" Max"); }
          else if(i!=0){ printf("    "); }
          printf("\n"); };
        printf("%+9.2e ",errors[i]);
      };
    printf(" R2       ");
    if(node_d>3){ printf("          "); }
    printf("          %9.2e  ||R||\n",
      std::sqrt(errtot[errtot.size()-1]) );
}
      }
#endif
    }//end parallel for
}//end parallel region
    for(int i= 4; i< 2*(node_d+1); i++){ errtot[i]/=part_n; }
    printf(" ux        uy        uz        mag       ");
    //printf("Normalized Error in %i Partitions", part_n );
    if(node_d>3){ printf("Temp      Normalized Error in %i Parts", part_n ); }
    else{ printf("Normalized Error in %i Partitions", part_n ); }
    for(int i=0;i<int(errtot.size());i++){
      if(!(i%(node_d+1))){
        if(     i==1*(node_d+1)){ printf(" Min"); }
        else if(i==2*(node_d+1)){ printf(" Avg"); }
        else if(i==3*(node_d+1)){ printf(" Max"); }
        else if(i!=0){ printf("    "); }
        printf("\n"); }
      printf("%+9.2e ",errtot[i]);
    }
    printf(" R2       ");
    if(node_d>3){ printf("          "); }
    printf("          %9.2e  ||R||\n",
      std::sqrt(errtot[errtot.size()-1]) );
    }//end if load_step==1
    // Effective modulus --------------------------------------------
    // Calculate nodal forces from displacement solution
    FLOAT_PHYS reac_x=0.0;// polycrystal approx.
    //FIXME should do this only for elems with prescribed BCS
#pragma omp parallel num_threads(comp_n)
{
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
    const auto n = S->udof_n;
    for(uint i=0;i<n;i++){ S->part_f[i]=0.0; }
    Y->ElemLinear( E,0,E->halo_elem_n, S->part_f, S->part_u );
    // sync part_f
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){
      auto f = Dn* E->node_haid[i];
      for(uint j=0; j<Dn; j++){
#pragma omp atomic write
        M->halo_val[f+j] = S->part_f[Dn* i+j]; }
    }
  }
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){
#pragma omp atomic update
        M->halo_val[f+j]+= S->part_f[Dn* i+j]; }
    }
  }// finished gather, now scatter back to elems
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    const INT_MESH hnn=E->halo_node_n;
    for(INT_MESH i=0; i<hnn; i++){
      auto f = Dn* E->node_haid[i];
      for( uint j=0; j<Dn; j++){//NOTE appears not to be critical
#if 0
#pragma omp atomic read
#endif
        S->part_f[Dn* i+j] = M->halo_val[f+j]; }
    }
    Y->ElemLinear( E,E->halo_elem_n,E->elem_n, S->part_f, S->part_u );;
  }
  // Now, sum the reactions on BCS fixed-displacemnt nodes in the test direction.
  // Also, compute the polycrystal effective Young's modulus
#pragma omp for schedule(static) reduction(+:reac_x)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->priv_part[part_i];
    const INT_MESH Dn=uint(Y->node_d);
    uint dof=test_dir;
    INT_MESH n,f; FLOAT_MESH v;
    INT_MESH r=E->halo_remo_n;
    for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
      // Don't duplicate halo nodes
      if(n>=r){ if(f==dof){ reac_x+=S->part_f[Dn* n+dof]; } }
    }
  }
  }// end parallel
  FLOAT_PHYS A=1.0, L=1.0;
  //FLOAT_PHYS A=1.0/(scale*scale);
  switch(test_dir){
    case(1): A=1.0/(scax*scaz); L=scay; break;
    case(2): A=1.0/(scax*scay); L=scaz; break;
    case(0):
    default: A=1.0/(scay*scaz); L=scax;
  }
  FLOAT_PHYS reac_ther = ther_pres * A;
  printf("Model Response\n");
  printf("        Load / Area  : %+9.2e /%9.2e  N/m2 = %+12.5e Pa\n",
    reac_x, A, reac_x/A );
  if(reac_ther!=0.0){
  printf("Thermal Load / Area  : %+9.2e /%9.2e  N/m2 = %+12.5e Pa\n",
    reac_ther, A, reac_ther/A ); }
  printf("Displacement / Length: %+9.2e /%9.2e  m/m  = %+12.5e strain\n",
    test_u, 1.0/L, test_u*L );
    //test_u, 1.0/scale, test_u*scale );
  printf("              Modulus: %9.2e /%9.2e Pa/Pa Voigt Average\n",
    (reac_x+reac_ther)/A/(test_u*L), youn_voig );
    //reac_x/A/(test_u*scale), youn_voig );
  { float e=( (reac_x+reac_ther)/A/(test_u*L))/youn_voig -1.0;
  //{ float e=( reac_x/A/(test_u*scale))/youn_voig -1.0;
  printf("        Modulus Error:");
  if( std::abs(e)<test_u ){ printf(" %+9.2e\n",e);
  }else{ printf("%+6.2f%%\n",100.*e); }
  }
    }//end if verbosity > 1
#endif
#endif //HAS_TEST
  }//load step loop
  }//load step scope
  return 0;
}
