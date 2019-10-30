#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include "gmsh.h"
int main( int argc, char** argv ) {
  typedef std::tuple<INT_DOF,INT_DOF,FLOAT_MESH> ati;
  typedef std::tuple<INT_DOF,INT_DOF,FLOAT_MESH,FLOAT_MESH> atv;
  typedef std::set<ati> atiset;
  typedef std::set<atv> atvset;
  atiset sli_at={};
  atiset bc0_at={};
  atvset bcs_at={};
  atvset rhs_at={};
  //FIXED Parse command line args
  //FIXED Include specifying BCs and RHS based on Gmsh physical ID tags
  //FIXME Add -s:surface tag, -l:line tag
  //FIXME -a:ascii (default), -b:binary not yet supported, -c:csv ;
  //FXIME optional output file basename w/ -abc
  /* Example:
   * gmsh2fmr -t 1 -xy0 -t 2 -t 3 -xu 1e-3 -t 4 -x -z -f 1e6
   * 
   *   solution u fixed at zero in x,y for nodes in elements with physical id tag #1
   *   solution u fixed at zero in z for nodes in elements with physical id tag #2,3
   *   solution u set to 1e-3 in x,z for nodes in elements with physical id tag #4
   * */
  int verbosity=1;
  const char* bname=NULL;//FIXME Store this in Femera or Mesh instance?
  const char* iname=NULL; std::string file_ext=".msh";
  std::string pname,oriname;
  INT_PART part_0=0, part_n=0;
  Gmsh* M = new Gmsh();
  bool save_asc=false, save_bin=false, save_csv=false, save_abq=false;
  bool is_part=false;
  std::unordered_map<int,std::vector<FLOAT_PHYS>>
    mtrl_part, tcon_part, texp_part, plas_part;
  bool rotfile=false, allrand=false;// Random orientations
  FLOAT_MESH eps_find=1e-6;
  FLOAT_PHYS oriunit=1.0;
  std::vector<FLOAT_PHYS> orislist={};
  //int hown_meth = 2;// Algorithm to balance halo node owner
  // 0: first touch (lowest partition number)
  // 1: partition with fewest nodes, first touch tiebreaker (original optimize)
  // 2: partition with fewest nodes, even-odd tiebreaker (default)
  // 3: random
  //
  //FIXME Consider using C++ for parsing command line options.
  {//Scope these variables
    std::vector<int> tagslist={},nodelist={},dofslist={};
    std::vector<int>  parttags={}, axislist={},parttmp={};
    std::vector<FLOAT_PHYS> younlist={}, poislist={}, smodlist={},
      matclist={}, rdeglist={}, avallist={},
      tconlist={}, texplist={}, plaslist={};
#if 0
    std::vector<FLOAT_PHYS>  therlist={},
      younlist, poislist, smodlist, matclist, rdeglist;
    std::unordered_map<int,std::vector<int>> axislist;
#endif
    double uval=0.0, fval=0.0;
    int sval=1;
    bool fix0=false, load=false, disp=false, slic=false;
    bool hasmatp=false, hasmatc=false, rotrand=false, mtrldone=true;
    //
    opterr = 0; int c;
    while ((c = getopt (argc, argv,
      "abcqo:pv:wt:n:@:xyzT0u:f:S:M:X:Y:Z:E:N:G:A:K:J:C:B:O:R")) != -1){
      // x:  x requires an argument
      mtrldone=true;
      switch (c) {
        // Input filename(s)
        case 'v':{ verbosity = atoi(optarg);M->verbosity=verbosity; break; }
        case 'p':{ is_part = true; break; }
        case 'w':{ M->calc_band = true; break; }
        case 'o':{ M->hown_meth = atoi(optarg); break; }
        //case 'P':{ is_part = true; part_n=atoi(optarg); break;}// pstr = optarg;
        // Output format
        case 'a':{ save_asc=true; break; }
        case 'b':{ save_bin=true; break; }
        case 'c':{ save_csv=true; break; }
        case 'q':{ save_abq=true; break; }
        // Boundary Conditions
        case 'n':{ nodelist.push_back(atoi(optarg)); uval=0.0; fval=0.0; sval=1; break; }
        case 't':{ tagslist.push_back(atoi(optarg)); uval=0.0; fval=0.0; sval=1; break; }
        case '@':{ avallist.push_back(atof(optarg)); uval=0.0; fval=0.0; sval=1; break; }
        case 'x':{ dofslist.push_back(0); break; }
        case 'y':{ dofslist.push_back(1); break; }
        case 'z':{ dofslist.push_back(2); break; }
        case 'T':{ dofslist.push_back(3); break; }//FIXME Swap -t and -T?
        case '0':{ fix0=true; break; }
        case 'u':{ disp=true; uval=atof(optarg); break; }
        case 'f':{ load=true; fval=atof(optarg); break; }
        case 'S':{ slic=true; sval=atoi(optarg); break; }
#if 0
        // Physics
        case 'T':{ // Gmsh volume physical ID tag
          if(hasmatc|hasmatp){ volutmp.push_back(atoi(optarg));
          }else{
            mtrldone=false;volutags.push_back(atoi(optarg));
          } break; }
#endif
        case 'M':{ 
          if(hasmatc|hasmatp){ parttmp.push_back(atoi(optarg));
          }else{mtrldone=false;parttags.push_back(atoi(optarg));
          };break; }// Mesh partition number
        case 'C':{ matclist.push_back(atof(optarg)); hasmatc=true; mtrldone=false;break; }
        case 'E':{ younlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'N':{ poislist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'G':{ smodlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'A':{ texplist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'K':{ tconlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'J':{ plaslist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        //case 'H':{ therlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'X':{ axislist.push_back(0); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
        case 'Y':{ axislist.push_back(1); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
        case 'Z':{ axislist.push_back(2); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
        //case 'T':{ axislist.push_back(3); rdeglist.push_back(atof(optarg)); mtrldone=false; hasther=true;break; }
        case 'R':{ rotrand=true; mtrldone=false;break; }
        case 'B':{ rotfile=true; oriname=optarg; mtrldone=false; break; }
        case 'O':{ rotfile=true; oriname=optarg; mtrldone=false;
          oriunit = PI/180.; break;}
        //
        case '?':{
          if (optopt == 'P'){// specify number of partitions
            //FIXME This option does not yet work...
            fprintf (stderr, "ERROR Option -%c requires an integer argument.\n",
              optopt); return 1; }
          else if (isprint(optopt)){
            fprintf (stderr,
              "ERROR Unknown command line option `-%c'.\n",
              //"WARNING Ignoring unknown command line option `-%c'.\n",
              optopt); return 1;}//FIXME segfaults if continued
          else{
            fprintf(stderr,
              "ERROR Unknown command line option character `\\x%x'.\n",
              //"WARNING Ignoring unknown command line option character `\\x%x'.\n",
              optopt); return 1;}
          //return 1;
        }
        default:{ abort();}
      }
      if( hasmatp & hasmatc){
        fprintf (stderr,"ERROR Specify only material properties:\n");
        fprintf (stderr,"      -E<Young's modulus> -N<Poisson's ratio>\n");
        fprintf (stderr,"      -G<shear modulus>\n");
        fprintf (stderr,"      -A<Thermal expansion> -K<thermal conductivity>\n");
        fprintf (stderr,"      -J<Plasticity parameter>...\n");
        fprintf (stderr,"   OR material response matrix values:\n");
        fprintf (stderr,"      -C<c11>...\n");
        return 1; }
#if VERB_MAX > 11
        if(fix0){
          printf("Tags:");
          for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); }
          printf(", Nodes:");
          for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); }
          printf(", @:");
          for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); }
          printf(" DOFs:");
          for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); }
          printf(", Displacement/temperature fixed zero\n");
        }
        if(disp){
          printf("Tags:");
          for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); }
          printf(", Nodes:");
          for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); }
          printf(", @:");
          for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); }
          printf(" DOFs:");
          for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); }
          printf(", Displacement/temperature: %+9.2e\n",uval);
        }
        if(load){
          printf("Tags:");
          for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); }
          printf(", Nodes:");
          for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); }
          printf(", @:");
          for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); }
          printf(" DOFs:");
          for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); }
          printf(", Load: %+9.2e\n",fval);
        }
        if(slic){
#if 0
          printf("Tags:");
          for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); }
          printf(", Nodes:");
          for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); }
          printf(", @:");
          for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); }
          printf(" DOFs:");
#endif
          for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); }
          printf(", Slices: %i\n",sval);
          //for(size_t i=0;i<svallist.size();i++){ printf(" %i",svallist[i]); }
          //printf(", Slices: %i\n",sval);
        }
#endif
      if( fix0 | disp | load ){
        for(size_t f=avallist.size();f<dofslist.size();f++){
          for(size_t i=0;i<nodelist.size();i++){
            if(fix0){ M->bc0_nnf.push_back(Gmsh::tfitem(nodelist[i],dofslist[f])); }
            if(disp){ M->bcs_nvals.push_back(Gmsh::tfval(nodelist[i],dofslist[f],uval)); }
            if(load){ M->rhs_nvals.push_back(Gmsh::tfval(nodelist[i],dofslist[f],fval)); }
          }
          for(size_t i=0;i<tagslist.size();i++){
            if(fix0){ M->bc0_tnf.push_back(Gmsh::tfitem(tagslist[i],dofslist[f])); }
            if(disp){ M->bcs_tvals.push_back(Gmsh::tfval(tagslist[i],dofslist[f],uval)); }
            if(load){ M->rhs_tvals.push_back(Gmsh::tfval(tagslist[i],dofslist[f],fval)); }
          }
          for(size_t i=0;i<avallist.size();i++){
            if(fix0){ bc0_at.insert(
              ati((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i])); }
            if(disp){ bcs_at.insert(
              atv((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i],uval)); }
            if(load){ rhs_at.insert(
              atv((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i],fval)); }
          }
        }
      }
      if(slic){
        for(size_t f=0;f<dofslist.size();f++){
          M->part_slic[(INT_DOF)dofslist[f]] = sval;
          if( verbosity > 4 ){
            printf("DOF %i Slices: %i\n", dofslist[f], sval ); }
        }
      }
      if(fix0){ nodelist={};tagslist={};dofslist={};avallist={}; fix0=false; }
      if(disp){ nodelist={};tagslist={};dofslist={};avallist={}; disp=false; }
      if(load){ nodelist={};tagslist={};dofslist={};avallist={}; load=false; }
      if(slic){ nodelist={};tagslist={};dofslist={};avallist={}; slic=false; }
      //
      if( mtrldone & (parttags.size() > 0) ){
#if VERB_MAX>4
        if(verbosity>4){
        std::cout << ", ";
        std::cout << "Partitions:";
          for(int v : parttags ){ std::cout << " " << v; }
        std::cout << std::endl;
        //
        std::cout << "Young:";
          for(FLOAT_PHYS v : younlist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "Poiss:";
          for(FLOAT_PHYS v : poislist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "Shear:";
          for(FLOAT_PHYS v : smodlist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "Thermal expansion:";
          for(FLOAT_PHYS v : texplist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "Thermal conductivity:";
          for(FLOAT_PHYS v : tconlist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "Plasticity parameters:";
          for(FLOAT_PHYS v : plaslist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << " matC:";
          for(FLOAT_PHYS v : matclist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << " Rdeg:";
          for(FLOAT_PHYS v : rdeglist ){ std::cout << " " << v; }
        std::cout << std::endl;
        std::cout << "RAxes:";
          for(int v : axislist ){ std::cout << " " << v; }
        std::cout << std::endl;
        }
#endif
        {uint n= uint(3)-rdeglist.size();
        if(rotrand){ for(uint i=0; i<n; i++){
          rdeglist.push_back(FLOAT_PHYS(std::rand())/(FLOAT_PHYS(RAND_MAX)+1.0)
            * 2.0*PI );//FIXME These are overwritten later if allrand==true.
          if(n==3){allrand=true;}//FIXME Not quite the way to do this
          } } }
        if(rotfile){ rdeglist={0.0,0.0,0.0};}
        if(( smodlist.size()>0 ) & ( rdeglist.size()==0 ) ){
          rdeglist={0.0,0.0,0.0};}
        for(int p : parttags ){
          for(FLOAT_PHYS v : rdeglist ){ mtrl_part[p].push_back(v); }
          for(FLOAT_PHYS v : younlist ){ mtrl_part[p].push_back(v); }
          for(FLOAT_PHYS v : poislist ){ mtrl_part[p].push_back(v); }
          for(FLOAT_PHYS v : smodlist ){ mtrl_part[p].push_back(v); }
          for(FLOAT_PHYS v : plaslist ){ plas_part[p].push_back(v); }
          for(FLOAT_PHYS v : texplist ){ texp_part[p].push_back(v); }
          for(FLOAT_PHYS v : tconlist ){ tcon_part[p].push_back(v); }
        }
        mtrldone=true; hasmatc=false; hasmatp=false; rotrand=false;
        parttags=parttmp;
        younlist={}; poislist={}; smodlist={}; matclist={};
        tconlist={}; texplist={};
        rdeglist={}; axislist={};
        parttmp ={};// volutmp={};
      }// Done getting material property set
    }// end argument parsing loop
#if VERB_MAX>2
    if(verbosity>2){
      if(mtrl_part.size()>0){
        std::cout << "Partition Physics: "<< std::endl;;
        for (std::pair<int,std::vector<FLOAT_PHYS>> pr : mtrl_part){
          std::cout << "["<< pr.first <<"]";
          for(FLOAT_PHYS v : pr.second ){ std::cout <<" "<<v; }
          std::cout << std::endl;
        } }
#if 0
    if(mtrl_volu.size()>0){
      std::cout << "Tagged Volume Physics:" << std::endl;
      for (std::pair<int,std::vector<FLOAT_PHYS>> pr : mtrl_volu){
        std::cout << "["<< pr.first <<"]";
        for(FLOAT_PHYS v : pr.second ){ std::cout <<" "<<v; }
        std::cout << std::endl;
      } }
#endif
    }
#endif
    for (int i = optind; i < argc; i++){
      if(i<(argc-1)){
      fprintf (stderr, "WARNING Ignoring command line option: %s.\n", argv[i]);
      }else{
      bname = argv[i];
      }
    }
    if(rotfile){
      std::ifstream ofile(oriname);
#if VERB_MAX>1
      if(verbosity>1){
        std::cout << "Reading Bunge (deg) crystal orientations from " 
        << oriname << "..." <<'\n'; }
#endif
        FLOAT_PHYS o;
        while( ofile>>o ){ orislist.push_back( o * oriunit); }
      if(orislist.size()==0){
        std::cout << "ERROR No orientation data found in " <<oriname<< "."<<std::endl;
        return 1;
      }
    }
    // Done parsing command line options.
    //=======================================================
    if(!(save_asc | save_bin | save_csv)){ save_asc = true; }
    if(bname == NULL){
      std::cerr << "ERROR Mesh partition base filename not provided." << '\n';
      return 1;}
    else{
      if(is_part){
#if VERB_MAX>1
        if(verbosity>0){
      printf ("Looking for Gmsh partitions of %s...\n", bname); }
#endif
      bool fok=true; INT_PART part_i=1;
      while( fok ){
        std::stringstream ss;
        ss << bname << "_" << part_i << ".msh";
        pname = ss.str();
        // Check if can open file.
        FILE* pfile;
        pfile = fopen(pname.c_str(),"r");
        if (pfile==NULL){
          ss << "2";
          pname = ss.str();
          pfile = fopen(pname.c_str(),"r");
          if (pfile==NULL){ fok = false;
          }else{part_i++; iname=bname; file_ext=".msh2";
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "..." <<'\n'; }
#endif
          fclose (pfile); }
        }else{
          part_i++; iname=bname;
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "..." <<'\n'; }
#endif
          fclose (pfile);
          }
        }
      part_n = part_i-1;
      }else{
        std::stringstream ss;
        ss << bname ;
        pname = ss.str();
        // Check if can open file.
        FILE * pfile;
        pfile = fopen(pname.c_str(),"r");
        if (pfile==NULL){
          ss << ".msh" ;
          pname = ss.str();
          pfile = fopen(pname.c_str(),"r");
          if (pfile==NULL){
            ss << "2" ;
            pname = ss.str();
            pfile = fopen(pname.c_str(),"r");
            if (pfile==NULL){
              std::cout << "ERROR opening " << bname << " for reading." << '\n';
              return 1;
            } } }
          part_n=1; iname=pname.c_str();
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "." << '\n'; }
#endif
          if(pfile!=NULL){ fclose (pfile); }
      }
    }
    if(part_n>0){ part_0=0;
      if(part_n>1){ part_0=1;
#if VERB_MAX>1
        if(verbosity>0){
          printf ("Found %u mesh partitions.\n", part_n); } }
#endif
    }else{
      std::cerr << "ERROR No mesh partition files could be opened for reading."
        << '\n'; 
      return 1;
    }
  }//End scope of command line parsing variables
  //=================================================================
  // Read gmsh files.
  std::vector<Elem*> partlist(part_n+part_0);
//#pragma omp parallel for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    //Elem* E;// Gmsh* G;
    if(is_part){
    std::stringstream ss;
      ss << iname;  ss << "_" << part_i << ".msh" ;
      pname = ss.str();//FIXME Does not work in parallel
    }
#if VERB_MAX>3
    if(verbosity>3){
      std::cout << "Reading " << pname << "..." <<'\n'; }
#endif
    Elem* E=M->ReadMsh2( pname.c_str() );
//#pragma omp atomic write
    partlist[part_i]=E;
  }
  if(!is_part){ is_part=true;//part_0=1;//FIXME This determines first saved file.
    // Partition M[0] based on physical IDs or slice it...
    Elem* E0=partlist[0];
    uint Nc =uint(E0->elem_conn_n);//printf("**** %u ****",Nc);
    std::unordered_map<int,std::vector<int>> part_by;
    // Check if partition by slicing.
    const INT_PART slic_n = M->part_slic[0]*M->part_slic[1]*M->part_slic[2];
    if( slic_n > 1 ){
      part_by=M->elms_slid;
      if(verbosity>0){
        std::cout << "Partitioning " << pname << " by " << slic_n
          <<" ("<<M->part_slic[0]<<"x"<<M->part_slic[1]<<"x"<<M->part_slic[2]<<""
          <<") slices..." <<'\n'; }
    }else{
      part_by=M->elms_phid;
      if(verbosity>0){
          std::cout << "Partitioning " << pname << " by " << M->elms_phid.size()
            <<" Gmsh volume IDs..." <<'\n'; }
    }
#ifdef _OPENMP
    if(verbosity>0){
      printf("Partitioning %ix parallel...\n",omp_get_max_threads()); }
#endif
    //for(auto pr : part_by ){
#pragma omp parallel for schedule(static)
    for(uint p=0; p<part_by.size(); p++){
      auto pr=part_by.begin(); std::advance(pr, p);
      Elem* E = new Tet( E0->elem_p, pr->second.size());
#pragma omp critical
{
      if(verbosity>2){
        std::cout << "Making partition " << partlist.size()
          <<" with "<< pr->second.size() << " elements"
          <<"..."<<std::endl; }
      //part_n++;
      partlist.push_back(E);
}
      E->node_n=0;
      for(INT_MESH e=0;e<E->elem_n;e++){
        auto glel=pr->second[e];
        E->elem_glid[e]=glel;
        auto loe0=E0->elem_loid[glel];
        for(uint n=0;n<Nc;n++){
          auto glno = E0->node_glid[E0->elem_conn[Nc* loe0+n]];
          if( E->node_loid.count(glno)==0 ){
            E->node_loid[glno]=E->node_n; E->node_n++; }
          E->elem_conn[Nc* e+n] = E->node_loid[glno];
        }
      }
      uint d=uint(E->mesh_d);
      E->vert_n=E->node_n;
      E->node_coor.resize(d*E->vert_n);
      E->node_glid.resize(E->node_n);
      for( auto pr : E->node_loid ){
        int glid; INT_MESH l;
        std::tie(glid,l)=pr;
        E->node_glid[l]=glid;
        INT_MESH n0=E0->node_loid[glid];
        for(uint i=0;i<d;i++){
          E->node_coor[d* l+i ]=E0->node_coor[d* n0+i ]; }
      }
    }
  part_n = partlist.size();//-1;
  }// Done partitioning by gmsh slices or volume physical IDs.
  M->list_elem = partlist;
#if 0
  std::cout << "Made " << part_n <<":"<<M->list_elem.size()
    <<" partitions."<<std::endl;
#endif
#if VERB_MAX>1
  if(verbosity>0){
    printf("Applying boundary conditions...\n");
    if(verbosity>1){
    FLOAT_MESH loc,amt; INT_DOF f,g;
    for(auto tp : bc0_at){ std::tie(f,g,loc)=tp;
      printf("BC0 @DOF %u == %f: set DOF %u to zero.\n", uint(f),loc,uint(g)); }
    for(auto tp : bcs_at){ std::tie(f,g,loc,amt)=tp;
      printf("BCS @DOF %u == %f: set DOF %u = %f.\n", uint(f),loc,uint(g),amt); }
    for(auto tp : rhs_at){ std::tie(f,g,loc,amt)=tp;
      printf("RHS @DOF %u == %f: set DOF %u = %f.\n", uint(f),loc,uint(g),amt); }
    } }
#endif
  if( (bc0_at.size()+bcs_at.size()+rhs_at.size()) > 0 ){
    // Boundary conditions @
#pragma omp parallel for schedule(static)
    for(uint e=1; e<M->list_elem.size(); e++){//FIXME Merge with loops above?
      Elem* E=M->list_elem[e];
      int glid; INT_MESH loid;
      FLOAT_MESH loc,amt; INT_DOF f,g;
      uint d=uint(E->mesh_d);
      for( auto pr : E->node_loid ){ std::tie(glid,loid)=pr;
        for(auto tp : bc0_at){ std::tie(f,g,loc)=tp;
          if(std::abs(E->node_coor[d* loid+f ]-loc)<eps_find){
            E->bc0_nf.insert(Mesh::nfitem(loid,g)); } }
        for(auto tp : bcs_at){ std::tie(f,g,loc,amt)=tp;
          if(std::abs(E->node_coor[d* loid+f ]-loc)<eps_find){
            E->bcs_vals.insert(Mesh::nfval(loid,g,amt)); } }
        for(auto tp : rhs_at){ std::tie(f,g,loc,amt)=tp;
          if(std::abs(E->node_coor[d* loid+f ]-loc)<eps_find){
            E->rhs_vals.insert(Mesh::nfval(loid,g,amt)); } }
      }
    }
  }// end applying BC@
  //FIXME Destroy M->list_elem[0]?
  M->list_elem[0]=NULL; if(part_0==0){ part_0=1; part_n-=1; }//FIXME
  M->SyncIDs();//FIXME need to skip [0] when syncing
#if VERB_MAX>2
  if(verbosity>2){
  for(uint i=0; i<M->list_elem.size(); i++){
    std::cout << "Mesh Partition " << i ;
    if(M->list_elem[i]==NULL){std::cout << " is null.\n";
    }else{
      Elem* E=M->list_elem[i];
      std::cout << ": " << E->node_n << " Nodes ["
        << E->node_coor.size()<<"], "
        << E->elem_n << " Elems ["
        << E->elem_conn.size()<<"].";
      std::cout <<'\n' ;
#if VERB_MAX>3
    if(verbosity>3){
      printf("Nodes:");
      for(uint j=0;j<E->node_coor.size();j++){
        if(!(j%3)){printf("\n%3u(%3u):",j/3,E->node_glid[j/3]); }
        printf(" %9.2e",E->node_coor[j]);
      } printf("\n");
      printf("Elements:");
      INT_MESH ecn=E->elem_conn_n;
      for(INT_MESH j=0;j<E->elem_conn.size();j++){
        if(!(j%ecn)){printf("\n%3u(%3u):",j/ecn,E->elem_glid[j/ecn]); }
        printf(" %3u",E->elem_conn[j]);
      } printf("\n");
    INT_MESH n; INT_DOF f; FLOAT_SOLV v;
    std::cout << "RHS:" << '\n';
    for(auto t : E->rhs_vals){ std::tie(n,f,v)=t;
      std::cout << n << ":" << uint(f) << " " << v <<'\n'; }
    std::cout << "BCs:" << '\n';
    for(auto t : E->bcs_vals){ std::tie(n,f,v)=t;
      std::cout << n << ":" << uint(f) << " " << v <<'\n'; }
    std::cout << "BC0:" << '\n';
    for(auto t : E->bc0_nf  ){ std::tie(n,f)=t;
      std::cout << n << ":" << uint(f) <<'\n'; }
    }
#endif
    }//end if this Elem is defined
  }
  }
#endif
#if 0
  if(save_csv){
//#pragma omp parallel for schedule(static)
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      std::stringstream ss;
      ss << bname << "_" << part_i ;
      std::string pname = ss.str();
      std::cout << "Saving part " << pname << "..." <<'\n';
      M->list_elem[part_i]->SavePartCSV( pname.c_str() );
    }
  }
#endif
  if(save_asc | save_bin){
    if(verbosity==1){
      printf("Saving and appending physics to partitions...\n"); }
#pragma omp parallel for schedule(static)
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      Phys::vals props={100e9,0.3};//FIXME Should not have defaults here...
      Phys::vals dirs={0.0,0.0,0.0};// 
      Phys::vals prop={};
      auto m0=mtrl_part[0];
      std::stringstream ss;
      ss << bname;
      if(is_part){
        const INT_PART slic_n = M->part_slic[0]*M->part_slic[1]*M->part_slic[2];
        if( slic_n > 1 ){ ss << slic_n; }
        ss << "_" << part_i ;
      }
      ss << ".fmr";
      std::string pname = ss.str();
      if(verbosity>1){
        std::cout << "Saving part " << pname << "..." <<'\n'; }
      Phys* Y=new ElastIso3D(1.0,0.3);//FIXME
#pragma omp critical
{//FIXME why critical?
      if( mtrl_part.count( part_i ) >0 ){
        auto mp=mtrl_part[ part_i ];
        props.resize( mp.size() );
        for(uint i=0; i<mp.size(); i++){ props[i]=mp[i]; }
      }else if( m0.size() >0 ){
        props.resize( m0.size() );
        for(uint i=0; i<m0.size(); i++){ props[i]=m0[i]; }
        if(allrand &( props.size()>3 )){
          for(uint i=0; i<3; i++){
            props[i]=FLOAT_PHYS(std::rand())/(FLOAT_PHYS(RAND_MAX)+1.0)*2.0*PI;
            }
        }
        if(rotfile &(props.size()>3)){
          //FIXME Does not work unless -X0 -Z0 -X0 specified
          for(uint i=0; i<3; i++){
            props[i]=orislist[3* (part_i-part_0)+i ]; }
        }
      }
      if(props.size()>3){
        for(uint i=0;i<3;i++){ dirs[i]=props[i]; }
        prop.resize(props.size()-3);
        for(uint i=3;i<props.size();i++){ prop[i-3]=props[i]; }
        Y=new ElastOrtho3D(prop,dirs);
      }else{
        prop.resize(props.size());
        for(uint i=0;i<props.size();i++){ prop[i]=props[i]; }
        Y=new ElastIso3D(prop[0],prop[1]);
      }
      if(tcon_part[part_i].size()>0){
        if(verbosity>1){
          std::cout << "Setting partition thermal conductivities..." <<'\n'; }
        Y->ther_cond.resize(tcon_part[part_i].size());
        for(uint i=0; i<tcon_part[part_i].size(); i++){
          Y->ther_cond[i] = tcon_part[part_i][i]; }
      }else if(tcon_part[0].size()>0){
        if(verbosity>1){
          std::cout << "Setting model thermal conductivity..." <<'\n'; }
        Y->ther_cond.resize(tcon_part[0].size());
        for(uint i=0; i<tcon_part[0].size(); i++){
          Y->ther_cond[i] = tcon_part[0][i]; }
      }
      if(texp_part[part_i].size()>0){
        if(verbosity>1){
          std::cout << "Setting partition thermal expansions..." <<'\n'; }
        Y->ther_expa.resize(texp_part[part_i].size());
        for(uint i=0; i<texp_part[part_i].size(); i++){
          Y->ther_expa[i] = texp_part[part_i][i]; }
      }else if(texp_part[0].size()>0){
        if(verbosity>1){
          std::cout << "Setting model thermal expansion..." <<'\n'; }
        Y->ther_expa.resize(texp_part[0].size());
        for(uint i=0; i<texp_part[0].size(); i++){
          Y->ther_expa[i] = texp_part[0][i]; }
      }
      if(plas_part[part_i].size()>0){
        if(verbosity>1){
          std::cout << "Setting partition plasticities..." <<'\n'; }
        Y->plas_prop.resize(plas_part[part_i].size());
        for(uint i=0; i<plas_part[part_i].size(); i++){
          Y->plas_prop[i] = plas_part[part_i][i]; }
      }else if(plas_part[0].size()>0){
        if(verbosity>1){
          std::cout << "Setting model plasticity..." <<'\n'; }
        Y->plas_prop.resize(plas_part[0].size());
        for(uint i=0; i<plas_part[0].size(); i++){
          Y->plas_prop[i] = plas_part[0][i]; }
      }
}
      //Solv* S=new PCG(0, 0, 0.0);
      //Mesh::part t(M->list_elem[part_i],Y,S);
      Mesh::part t(M->list_elem[part_i],Y,new PCG(0, 0, 0.0));
      M->SavePartFMR( t, pname.c_str(), false );
      //if(verbosity>1){
      //  std::cout << "Appending physics to " << pname << "..." <<'\n'; }
      //Y->SavePartFMR( pname.c_str(), false );//FIXME Move to M->SavePartFMR()
    }//end saving parts loop
  }//end if ascii output
  if(save_abq){//FIXME Move to a method
    std::stringstream ss;
    ss << bname;;
    ss << ".inp";
    pname = ss.str();
    //pfile = fopen(pname.c_str(),"w");
    std::ofstream abqfile(pname);
    if(verbosity>0){
      std::cout << "Exporting Abaqus file "<< pname<<"..." <<'\n'; }
    // Create file with header
    abqfile << "*HEADING" <<'\n';
    abqfile << "Exported using gmsh2fmr" <<'\n';
    //
    // Append nodes
    abqfile << "*NODE" <<'\n';
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      Elem* E=M->list_elem[part_i];
      for(uint n=E->halo_remo_n; n<E->node_n; n++){
        abqfile << E->node_glid[n];
        for(uint i=0;i<E->mesh_d;i++){
          abqfile << "," << E->node_coor[E->mesh_d*n+i]; }
        abqfile << '\n';
      }
    }
    // Append Elements
    //std::string abq_el_str = "3D4";
    uint c=M->list_elem[part_0]->elem_conn_n;
    //if( c<1 ){ c=M->list_elem[1]->elem_conn_n; }
    //abqfile << "*ELEMENT, TYPE=C3D"<< c <<", ELSET=ALLTETS"<<'\n';
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      abqfile << "*ELEMENT, TYPE=C3D"<< c <<", ELSET=Volume" << part_i <<'\n';
      Elem* E=M->list_elem[part_i];
      uint Nc=E->elem_conn_n;
      for(uint e=0; e<E->elem_n; e++){
        abqfile << E->elem_glid[e];
        switch(c){
          case(10):{
            std::vector<uint> xn ={0,1,2,3, 4,5,6,7,9,8};// gmsh-> abq node number
            for(uint i=0; i<Nc; i++){
              abqfile <<"," << E->node_glid[ E->elem_conn[Nc* e+xn[i] ]]; }
            break;}
          default:{
            for(uint i=0; i<Nc; i++){
              abqfile <<"," << E->node_glid[ E->elem_conn[Nc* e+i ]]; } }
        }
        abqfile <<'\n';
      }
    }
    // Node sets for Sai's ScIFEN converter
    //for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
    //  Elem*to E=M->list_elem[part_i];
    //  abqfile << "*NODESET,NODESET=n_POLYCRYSTAL-"<< part_i;
    //  for(uint e=0; e<E->elem_n; e++){
    //    if( (e%40)==0){ abqfile<<'\n'; }else{ abqfile<<","; }
    //    //if(!(e%40)==0){ abqfile<<","; }
    //    abqfile << E->elem_glid[e];
    //  }
    //  abqfile <<'\n';
    //}
    // Element sets for Sai's ScIFEN converter
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      Elem* E=M->list_elem[part_i];
      //abqfile << "*ELSET,ELSET=Part_"<< part_i;
      abqfile << "*ELSET,ELSET=PhysicalVolume"<< part_i;
      for(uint e=0; e<E->elem_n; e++){
        if( (e%40)==0){ abqfile<<'\n'; }else{ abqfile<<","; }
        //if(!(e%40)==0){ abqfile<<","; }
        abqfile << E->elem_glid[e];
      }
      abqfile <<'\n';
    }
    // Define Materials
    abqfile << "*MATERIAL, TYPE=ISOTROPIC, NAME=MAT_0" <<'\n';
    abqfile << "*ELASTIC" <<'\n';
    auto mp=mtrl_part[0];//part_0];//FIXME
    if(mp.size()>1){// mp=mtrl_part[1];
      abqfile << mp[0] <<','<< mp[1]; }
    else{ mp=mtrl_part[1]; abqfile << mp[0] <<','<< mp[1]; }
    //if(mp.size()<2){ mp=mtrl_part[1]; }//FIXME
    //for(uint i=0;i<mp.size();i++){
    //  if(i<2){//FIXME Only iso
    //    abqfile << mp[i]; if(i<(mp.size()-1)){ abqfile  <<","; }
    //}
    abqfile << '\n';
    // Assign Materials to Elements
    abqfile << "*SOLID SECTION, MATERIAL=MAT_0, ELSET=ALLTETS" <<'\n';
    // Append BCs
    if((M->bc0_nf.size()+M->bcs_vals.size())>0){
      abqfile << "*BOUNDARY" <<'\n';
      int n; INT_DOF f; FLOAT_SOLV v;
      for(auto t : M->bc0_nf  ){ std::tie(n,f)=t;
        abqfile << n << ","<< uint(f+1) << "," << uint(f+1) <<'\n';
      }
      for(auto t : M->bcs_vals  ){ std::tie(n,f,v)=t;
        abqfile << n << "," << uint(f+1) <<","<< uint(f+1) <<"," << v <<'\n';
      }
    }
    // Start a load step
    abqfile << "*STEP, NAME=STEP-1, PERTURBATION" <<'\n';
    abqfile << "*STATIC" <<'\n';
    //FIXME Append load
    abqfile << "*NODE PRINT" <<'\n';
    abqfile << "COORD, U" <<'\n';
    abqfile << "*END STEP" <<'\n';
    //if(pfile!=NULL){ fclose (pfile); }
    abqfile.close();
  }// end Abaqus inp file export
  //if(save_bin){
  //  std::cout << "ERROR Binary save not yet implemented. " << '\n';
  //}
  //
  return 0;
}