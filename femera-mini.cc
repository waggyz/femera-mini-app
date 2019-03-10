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
  int iter_max   =-1;
  FLOAT_SOLV rtol= 1e-4;
  int solv_meth  = Solv::SOLV_CG;
  int solv_cond  = Solv::COND_JACO;
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
  while ((c = getopt (argc, argv, "v:pP:h:c:m:s:i:r:x:y:z:V:d:")) != -1){
    // x:  -x requires an argument
    switch (c) {
      case 'c':{ comp_n   = atoi(optarg); break;}
      case 'v':{ verbosity= atoi(optarg); break;}
      case 'V':{ simd_n   = atoi(optarg); break;}
#ifdef _OPENMP
      case 'm':{ numa_n   = atoi(optarg); break;}//FIXME Not yet used.
#if VERB_MAX>1
      case 'h':{ halo_mod = atoi(optarg); break;}
#endif
#endif
      case 'i':{ iter_max = atoi(optarg); break;}
      case 'r':{ rtol     = atof(optarg); break;}
      case 's':{ solv_meth= atoi(optarg); break;}
      case 'd':{ solv_cond= atoi(optarg); break;}
#if VERB_MAX>1
      // Cube test applied displacement
      case 'x':{ test_u   = atof(optarg); test_dir=0; break;}
      case 'y':{ test_u   = atof(optarg); test_dir=1; break;}
      case 'z':{ test_u   = atof(optarg); test_dir=2; break;}
#endif
      case 'p':{ is_part  = true; break;}
      case 'P':{ is_part  = true; part_n = atoi(optarg); break;}// pstr = optarg;
      case '?':{
        if (optopt == 'P'){// specify number of partitions
          //FIXME Does nothing.
          fprintf (stderr, "ERROR Option -%c requires an integer argument.\n",
            optopt); return 1; }
        else if (isprint(optopt)){
          fprintf (stderr,
            "ERROR unknown command line option `-%c'.\n",
            //"WARNING Ignoring unknown command line option `-%c'.\n",
            optopt); return 1;}//FIXME segfaults if continued
        else{
          fprintf(stderr,
            "ERROR unknown command line option character `\\x%x'.\n",
            //"WARNING Ignoring unknown command line option character `\\x%x'.\n",
            optopt); return 1;}
        //return 1;
      }
      default:{ abort();}
    };
  };
  for (int i = optind; i < argc; i++){
    if(i<(argc-1)){
    fprintf (stderr, "WARNING Ignoring command line option: %s.\n", argv[i]);
    }else{
    bname = argv[i];
    };
  };// Done parsing command line options.
  // Print Info =====================================================
  if( verbosity > 2 ){// Print compile-time information
    //std::vector<int> p={sizeof(FLOAT_MESH),sizeof(FLOAT_PHYS),sizeof(FLOAT_SOLV)};
    for(uint i=0;i<3;i++){
      std::string ps,ns; int p;
      switch(i){
        case(0):{ns="Mesh";p=sizeof(FLOAT_MESH); break;}
        case(1):{ns="Solv";p=sizeof(FLOAT_SOLV); break;}
        case(2):{ns="Phys";p=sizeof(FLOAT_PHYS); break;}
      };
      switch(p){
        case( 2):{ps="half"  ; break;}
        case( 4):{ps="single"; break;}
        case( 8):{ps="double"; break;}
        case(16):{ps="quad"  ; break;}
      };
      std::cout << ns << ": " << ps << " precision" <<'\n';
    };
    std::cout << "Maximum Elements and Nodes: "
      << std::numeric_limits<INT_MESH>::max() <<'\n';
    std::cout << "Maximum Mesh Partitions: "
      << (std::numeric_limits<INT_MESH_PART>::max()-1) <<'\n';
    std::cout << "Maximum Nodes/Element: "
      << size_t(std::numeric_limits<INT_ELEM_NODE>::max()) <<'\n';
    std::cout << "Maximum DOFs/Node: "
      << size_t(std::numeric_limits<INT_DOF>::max()) <<'\n';
    std::cout <<"Verbosity: "<<verbosity<<" / "<< VERB_MAX <<" maximum "<<'\n';
  };
  if( verbosity > VERB_MAX ){
    std::cout <<"WARNING Verbosity "<< verbosity
    <<" requested is more than compiled verbosity maximum "
    << VERB_MAX <<". Downgrading to "<<VERB_MAX <<"."<<'\n';
    verbosity=VERB_MAX;
  };
  // Find Mesh Files ================================================
  if(bname == NULL){
    std::cerr << "ERROR Mesh partition base filename not provided." << '\n';
    return 1;}
  else{
    if(is_part){
#if VERB_MAX>1
     if(verbosity>1){
     printf ("Looking for Femera partitions of %s...\n", bname);
     };
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
        };
#endif
        fclose (pfile);
        };
      };
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
          fclose (pfile); };
      }else{
        part_n=1; iname=pname.c_str();
#if VERB_MAX>3
        if(verbosity>3){
        std::cout << "Found " << pname << "..." << '\n';
        };
#endif
        fclose (pfile);
        };
      };
    };
  if(part_n>0){
    if(part_n>1){
#if VERB_MAX>1
  if(verbosity>1){
    printf ("Found %u mesh partitions.\n", part_n);};
#endif
    };
  }else{
    std::cerr << "ERROR No mesh partition files could be opened for reading."
      << '\n'; 
    return 1;
  };
  // Read and Setup =============================================
  int iter=0;
#if VERB_MAX>0
  float read_sec=0.0,init_sec=0.0,loop_sec=0.0;
#endif
//if VERB_MAX>1
  int iter_info_n = 1;
//endif
#ifdef _OPENMP
  if( comp_n <1){ comp_n = omp_get_max_threads(); };//omp_get_max_threads();
  if( numa_n==0){ numa_n = comp_n / 2; };//FIXME just a guess...and not yet used
  if( numa_n <2 ){ numa_n=2; };
  if( comp_n >int(part_n) ){ comp_n=int(part_n); };
  if( numa_n >int(part_n) ){ numa_n=int(part_n); };
#if VERB_MAX>1
  if(verbosity>1){
  std::cout <<"Parallel OpenMP " << "using "  <<comp_n<< " threads";
  if(simd_n>1){std::cout <<" and starting physics block width of "<<simd_n;};
  std::cout  <<"..."<<'\n';
    //<<comp_n<< " compute and "<<numa_n<<" NUMA threads..."<<'\n';
  };
#endif
#endif
  if(comp_n<1){ comp_n = 1; };
  //INT_ORDER pord=1;
  int part_0=1;//FIXME unpartitioned mesh in mesh_part[0]
#if VERB_MAX>1
  if(verbosity>1){
    std::cout << "Reading and setting up "<<(part_n-part_0+1)<<" partitions";
  //int dots_mod=0;
  if(verbosity!=2){ std::cout <<"..." <<'\n'; };
  };
#endif
  //printf("****** SOLVER: %i: %i,%i,%i\n",solv_meth,
  //  Solv::SOLV_GD,Solv::SOLV_CG,Solv::SOLV_CR);
  Mesh* M;
  switch( solv_meth ){
    case(Solv::SOLV_CG):{ M=new HaloPCG(part_n+part_0,iter_max,rtol); break;}
    case(Solv::SOLV_CR):{ M=new HaloPCR(part_n+part_0,iter_max,rtol); break;}
    default            :{ M=new HaloPCG(part_n+part_0,iter_max,rtol); }
  };
  //M->solv_meth = solv_meth;//FIXME set in constructor?
  M->solv_cond=solv_cond;
  M->base_name=iname;
  M->comp_n=comp_n;
  M->simd_n=simd_n;
  //M->glob_rtol = rtol;
  M->verbosity=verbosity;
  //M->mesh_part.resize(part_n+part_0);
  M->time_secs.resize(10);
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
  //std::string solv_name="FIXME";
  {// scope local variables
  //int sugg_max=3000;
  iter_info_n =   1;// sugg_max = M->udof_n;
  if      ( M->udof_n>int(1e8) ){ iter_info_n =1000;// sugg_max =M->udof_n/1000;
  }else if( M->udof_n>int(1e4) ){ iter_info_n = 100;// sugg_max =M->udof_n/10;
  }else if( M->udof_n>int(1e2) ){ iter_info_n =  10;// sugg_max =M->udof_n/10;
  }else                         { iter_info_n =   1;// sugg_max =M->udof_n; 
  };
  if(iter_max<0){
    iter_max=M->udof_n/iter_info_n*10;
    if(iter_max>int(M->udof_n)){ iter_max=M->udof_n; }; };
  }// end variable scope
//endif
#if VERB_MAX>0
  setu_done = std::chrono::high_resolution_clock::now();
  read_time = std::chrono::duration_cast<std::chrono::nanoseconds>
    (setu_done-read_start);
  read_sec=float(read_time.count())*1e-9;
  if(verbosity==1){
  std::cout << M->elem_n<<","<<M->node_n<<","<<M->udof_n<<","<<(part_n-part_0+1);
  };
#endif
#if VERB_MAX>1
  if(verbosity>1){
  std::cout <<"System:     "<<M->elem_n<<" Elems, "
    <<M->node_n<<" Nodes, "<<M->udof_n<<" DOF "<<'\n';
  printf("Read and set up                         in %f s\n", read_sec );
  if(verbosity>1){
    std::cout<< "  Initializing: ";
    switch(solv_cond){
      case(Solv::COND_NONE): std::cout<< "no"; break;
      case(Solv::COND_JACO): std::cout<< "Jacobi"; break;
      case(Solv::COND_ROW1): std::cout<< "element row one-norm"; break;
      case(Solv::COND_STRA): std::cout<< "element strain"; break;
      default: std::cout<< "unknown";
    };
    std::cout<< " preconditioner..." <<'\n';
  };
  };
#endif
  // Solve parallel =================================================
  bool halo_update=true;
#ifdef _OPENMP
#if VERB_MAX>2
  if(halo_mod!=1){
    std::cout<<"with halo updates every "
    <<halo_mod<< " iterations..."<<'\n'; };
    //<<comp_n<< " compute and " <<numa_n<< " NUMA threads." <<'\n';
#endif
#endif
  // Initialize ---------------------------------------------------
  {// init scope
    M->time_secs=0.0;//FIXME conditional?
    M->Init();
#if VERB_MAX>0
    if(verbosity>0){
    init_done = std::chrono::high_resolution_clock::now();
    init_time = std::chrono::duration_cast<std::chrono::nanoseconds>
      (init_done-setu_done);
    };
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
    else if( work_sec > 100.0*sec ){sc=sec; ss=" s"; };
    //
    if(verbosity>2){
      if(prec_sec/init_sec/float(comp_n)*pct > 0.03){//FIXME
      printf("%9.0f %s  %6.2f%% Combined Thread Jacobi Preconditioner Time\n",
        prec_sec/sc, ss, prec_sec/init_sec/float(comp_n)*pct); };
      printf("%9.0f %s  %6.2f%% Combined Global Node ID Map Init Time\n",
        gmap_sec/sc, ss, gmap_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Thread Gather Time\n",
        (gat0_sec+gat1_sec)/sc, ss,
        (gat0_sec+gat1_sec)/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Thread Scatter Time\n",
        scat_sec/sc, ss, scat_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Combined Physics and Solve Init Time\n",
        solv_sec/sc, ss, solv_sec/init_sec/float(comp_n)*pct );
      printf("%9.0f %s  %6.2f%% Init Sync and Overhead Time\n",
        (init_sec-work_sec/float(comp_n))/sc, ss,
        (init_sec-work_sec/float(comp_n))/init_sec*pct );
    };
#endif
#if VERB_MAX>1
    if(verbosity>1){
    std::cout<< "  Iterating to: "<<rtol<<" relative tolerance,"<<'\n'
      <<"or stopping at: "<<iter_max<<" "<<M->meth_name<<" iterations..."<<'\n';
    std::cout <<"Solving..."<<'\n';
    printf("     init ||R||%9.2e /%9.2e tol in %f s\n",
      std::sqrt(M->glob_chk2), std::sqrt(M->glob_rto2),init_sec );
    };
#endif
  }// end init scope
  {// iter scope
    M->time_secs=0.0;//FIXME conditional?
    // Iterate ------------------------------------------------------
    auto loop_start = std::chrono::high_resolution_clock::now();
    do{ M->Iter(); iter++;
      //for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      //  Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->mesh_part[part_i];
      //  S->Init(); S->Iter();
      //}; M->Init();
#if VERB_MAX>1
      if(verbosity>1){
      if(!((iter) % iter_info_n) ){
        float iter_sec=M->time_secs[5];
        printf("%9i ||R||%9.2e @ %.0f iter/s\n",
          iter, std::sqrt(M->glob_chk2),
          float(iter)/iter_sec*float(comp_n) ); };
      };
#endif
#if VERB_MAX>10
    // Partition Residuals
    if(!((iter) % iter_info_n) ){
      for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
        Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=M->mesh_part[part_i];
        printf("Part %4i ||R||%9.2e\n", part_i, std::sqrt(S->loca_res2));
      }; };
#endif
    }while( ( (iter < iter_max) & (M->glob_chk2 > M->glob_rto2) ) | !halo_update );
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
    };
#endif
#if VERB_MAX>1
    float sc = ms; const char* ss="ms";
    if(verbosity>1){
    if( loop_sec < 100.0*ms ){ sc=us; ss="μs"; }
    else if( loop_sec > 100.0*sec ){sc=sec; ss=" s"; };
    printf("%9i ||R||%9.2e /%9.2e tol in %f s\nDone.\n", iter,
      std::sqrt(M->glob_chk2), std::sqrt(M->glob_rto2), loop_sec );
    };
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
    printf("%9.0f %s  %6.2f%% Combined Thread Halo Scatter Time\n",
      scat_sec /sc, ss, scat_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Thread Solve Time\n",
      solv_sec /sc, ss, solv_sec/iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Thread Work Time\n",
      iter_sec /sc, ss, (phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec)
      /iter_sec *pct );
    printf("%9.0f %s  %6.2f%% Combined Sync and Overhead Time\n",
      (iter_sec-(phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec)) /sc, ss,
      (iter_sec-(phys_sec+gat0_sec+gat1_sec+scat_sec+solv_sec))
      / iter_sec*pct );
    printf("%9.0f %s  %6.2f%% Wall Clock Solve Time (Load Balance)\n",
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
    };
#endif
  }// end iter scope
#ifdef HAS_TEST
    // Check solution ===============================================
#if VERB_MAX>1
    if(verbosity>1){
    // Displacement errors ------------------------------------------
    Phys::vals errtot; errtot.resize(13); errtot=0.0;
    for(int i= 0; i< 4; i++){ errtot[i] = 99e99; };
    for(int i= 8; i<12; i++){ errtot[i] =-99e99; };
    printf("Solution Error (Compared to Isotropic)\n");
    Test* T = new Test(); int mesh_d=3;
    //FLOAT_PHYS scale=1.0,smin= 99e9,smax=-99e9;//FIXME only works for cubes
    FLOAT_PHYS scax=1.0,minx= 99e9,maxx=-99e9;
    FLOAT_PHYS scay=1.0,miny= 99e9,maxy=-99e9;
    FLOAT_PHYS scaz=1.0,minz= 99e9,maxz=-99e9;
#pragma omp parallel
{  std::vector<part> P;
   P.resize(this->mesh_part.size());
   std::copy(this->mesh_part.begin(), this->mesh_part.end(), P.begin());
#pragma omp for schedule(static)
    for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
#pragma omp critical(minmax)
{
      //smin=std::min( smin, E->vert_coor.min() );
      //smax=std::max( smax, E->vert_coor.max() );
      Phys::vals t = E->vert_coor[std::slice(0,E->node_n,3)];
      minx=std::min(minx, t.min() );
      maxx=std::max(maxx, t.max() );
      t = E->vert_coor[std::slice(1,E->node_n,3)];
      miny=std::min(miny, t.min() );
      maxy=std::max(maxy, t.max() );
      t = E->vert_coor[std::slice(2,E->node_n,3)];
      minz=std::min(minz, t.min() );
      maxz=std::max(maxz, t.max() );
}
    };
#pragma omp single
{
    //scale = 1.0/(smax-smin);
    scax = 1.0/(maxx-minx);
    scay = 1.0/(maxy-miny);
    scaz = 1.0/(maxz-minz);
}
#pragma omp for schedule(static)
    for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
      Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
      Phys::vals errors={};
      FLOAT_PHYS nu=Y->mtrl_prop[1];
      Phys::vals coor(E->vert_coor.size());
      switch(test_dir){
      case(1):{
        coor[std::slice(0,E->node_n,3)]=E->vert_coor[std::slice(1,E->node_n,3)];
        coor[std::slice(1,E->node_n,3)]=E->vert_coor[std::slice(2,E->node_n,3)];
        coor[std::slice(2,E->node_n,3)]=E->vert_coor[std::slice(0,E->node_n,3)];
        break;}
      case(2):{
        coor[std::slice(0,E->node_n,3)]=E->vert_coor[std::slice(2,E->node_n,3)];
        coor[std::slice(1,E->node_n,3)]=E->vert_coor[std::slice(0,E->node_n,3)];
        coor[std::slice(2,E->node_n,3)]=E->vert_coor[std::slice(1,E->node_n,3)];
        break;}
      case(0):
      default:{ coor = E->vert_coor; break;}
      };
      for(uint i=0;i<E->node_n*3;i+=3){
        coor[i]*=scax; coor[i+1]*=scay; coor[i+2]*=scaz; };
      //coor[std::slice(0,E->node_n,3)]=coor[std::slice(0,E->node_n,3)]*scax;
      //coor[std::slice(1,E->node_n,3)]=coor[std::slice(1,E->node_n,3)]*scay;
      //coor[std::slice(2,E->node_n,3)]=coor[std::slice(2,E->node_n,3)]*scaz;
      T->CheckCubeError( errors, nu,
        coor, S->sys_u/FLOAT_SOLV(test_u) );
        //E->vert_coor*scale, S->sys_u/FLOAT_SOLV(test_u) );
#pragma omp critical(errs)
{
      for(int i= 0; i< 4; i++){ errtot[i] = std::min(errtot[i],errors[i]); };
      for(int i= 4; i< 8; i++){ errtot[i]+= errors[i]; };
      for(int i= 8; i<12; i++){ errtot[i] = std::max(errtot[i],errors[i]); };
      errtot[12]+=errors[12];
#if VERB_MAX>3
      if(S->sys_u.size()<300){
        //printf("Node Coordinates:");
        //for(uint i=0;i<E->vert_coor.size();i++){
        //  if((i%3)==0){printf("\n");};
        //  printf("%+9.2e ",E->vert_coor[i]);
        //}; printf("\n Displacements:");
        for(uint i=0;i<S->sys_u.size();i++){
          if((i%3)==0){printf("\n");};
          printf("%+9.2e ",S->sys_u[i]);
        }; printf("\n");
      };
#endif
}
#if VERB_MAX>2
      if(verbosity>2){
#pragma omp critical(print)
{
      printf(" ux        uy        uz        mag       ");
      printf("Normalized Error in Partition %i", part_i );
      for(size_t i=0;i<errors.size();i++){
        if(!(i%(mesh_d+1))){
          switch(i){
            case( 4): printf(" Min"); break;
            case( 8): printf(" Avg"); break;
            case(12): printf(" Max"); break;
            default : printf("    "); break;
        }; printf("\n"); };
        printf("%+9.2e ",errors[i]);
      };
    printf(" R2                 %9.2e  ||R||\n",
      std::sqrt(errtot[errtot.size()-1]) );
}
      };
#endif
    };//end parallel for
}//end parallel region
    for(int i= 4; i< 8; i++){ errtot[i]/=part_n; };
    printf(" ux        uy        uz        mag       ");
    printf("Normalized Error in %i Partitions", part_n );
    for(size_t i=0;i<errtot.size();i++){
      if(!(i%(mesh_d+1))){
        switch(i){
          case( 4): printf(" Min"); break;
          case( 8): printf(" Avg"); break;
          case(12): printf(" Max"); break;
          default : printf("    "); break;
      }; printf("\n"); };
      printf("%+9.2e ",errtot[i]);
    };
    printf(" R2                 %9.2e  ||R||\n",
      std::sqrt(errtot[errtot.size()-1]) );
    // Effective modulus --------------------------------------------
    // Calculate nodal forces from displacement solution
    FLOAT_PHYS reac_x=0.0, youn_voig=0.0;// polycrystal approx.
    //FIXME should do this only for elems with prescribed BCS
#pragma omp parallel num_threads(comp_n)
{
  std::vector<Mesh::part> P;  P.resize(M->mesh_part.size());
  std::copy(M->mesh_part.begin(), M->mesh_part.end(), P.begin());
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    S->sys_f=0.0;
    E->do_halo=true; Y->ElemLinear( E, S->sys_f, S->sys_u );
    //E->do_halo=false; Y->ElemLinear( E, S->sys_f, S->sys_p );
    // sync sys_f
    const INT_MESH d=uint(Y->ndof_n);
    const INT_MESH hnn=E->halo_node_n,hrn=E->halo_remo_n;
    for(INT_MESH i=hrn; i<hnn; i++){
      auto f = d* E->node_haid[i];
      for(uint j=0; j<d; j++){
#pragma omp atomic write
        M->halo_val[f+j] = S->sys_f[d* i+j]; };
    };
  };
#pragma omp for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    const INT_MESH hrn=E->halo_remo_n;
    for(INT_MESH i=0; i<hrn; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){
#pragma omp atomic update
        M->halo_val[f+j]+= S->sys_f[d* i+j]; };
    };
  };// finished gather, now scatter back to elems
#pragma omp for schedule(static) reduction(+:reac_x)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    const INT_MESH hnn=E->halo_node_n;
    for(INT_MESH i=0; i<hnn; i++){
      auto f = d* E->node_haid[i];
      for( uint j=0; j<d; j++){//NOTE appears not to be critical
//#pragma omp atomic read
        S->sys_f[d* i+j] = M->halo_val[f+j]; };
    };
    E->do_halo=false; Y->ElemLinear( E, S->sys_f, S->sys_u );
  };
  // Now, sum the reactions on BCS fixed-displacemnt nodes in the x-direction.
  // Also, compute the polycrystal effective Young's modulus
#pragma omp for schedule(static) reduction(+:reac_x)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    Elem* E; Phys* Y; Solv* S; std::tie(E,Y,S)=P[part_i];
    const INT_MESH d=uint(Y->ndof_n);
    uint dof=0;// x-direction
    INT_MESH n,f; FLOAT_MESH v;
    INT_MESH r=E->halo_remo_n;
    for(auto t : E->bcs_vals ){ std::tie(n,f,v)=t;
      // Don't duplicate halo nodes
      if(n>=r){ reac_x+=S->sys_f[d* n+dof]; };
    };
#pragma omp critical
{
    FLOAT_PHYS A=0.0,B=0.0,C=0.0;
    if(Y->mtrl_matc.size()==3){
      A=Y->mtrl_matc[0]; B=Y->mtrl_matc[1]; C=Y->mtrl_matc[2];
    }else if(Y->mtrl_matc.size()==9){
      A=(Y->mtrl_matc[0]+Y->mtrl_matc[1]+Y->mtrl_matc[2])/3.0;
      B=(Y->mtrl_matc[3]+Y->mtrl_matc[4]+Y->mtrl_matc[5])/3.0;
      C=(Y->mtrl_matc[6]+Y->mtrl_matc[7]+Y->mtrl_matc[8])/3.0;
    };
    youn_voig=(A-B+3.0*C)*(A+2.0*B)/(2.0*A+3.0*B+C);
    //printf("c11:%e, c12:%e, c44:%e\n",A,B,C);
    //printf("ELEM NODES: %u\n",uint(E->elem_conn_n));
}
  };
  }// end parallel 
  //FLOAT_PHYS A=1.0/(scale*scale);
  FLOAT_PHYS A=1.0/(scay*scaz);
  printf("Model Response\n");
  printf("        Load / Area  : %+9.2e /%9.2e  N/m2 = %+9.2e Pa\n",
    reac_x, A, reac_x/A );
  printf("Displacement / Length: %+9.2e /%9.2e  m/m  = %+9.2e strain\n",
    test_u, 1.0/scax, test_u*scax );
    //test_u, 1.0/scale, test_u*scale );
  printf("              Modulus: %9.2e /%9.2e Pa/Pa Voigt Average\n",
    reac_x/A/(test_u*scax), youn_voig );
    //reac_x/A/(test_u*scale), youn_voig );
  { float e=( reac_x/A/(test_u*scax))/youn_voig -1.0;
  //{ float e=( reac_x/A/(test_u*scale))/youn_voig -1.0;
  printf("        Modulus Error:");
  if( std::abs(e)<test_u ){ printf(" %+9.2e\n",e); 
  }else{ printf("%+6.2f%%\n",100.*e); };
  }
    };
#endif
#endif //HAS_TEST
  return 0;
};
