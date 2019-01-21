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
  atiset bc0_at={};// ati(0,0,0.0), ati(1,1,0.0), ati(2,2,0.0) };
  atvset bcs_at={};// atv(0,0,1.0,0.001) };
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
  int verbosity=1;//,dots_mod=0;
  const char* bname=NULL;//FIXME Store this in Femera or Mesh instance?
  const char* iname=NULL; std::string file_ext=".msh";
  std::string pname,oriname;
  INT_MESH_PART part_0=0, part_n=0;
  Gmsh* M = new Gmsh();
  bool save_asc=false, save_bin=false, save_csv=false;
  bool is_part=false;
  std::unordered_map<int,std::vector<FLOAT_PHYS>> mtrl_part,mtrl_volu;
  bool rotfile=false, allrand=false;// Random orientations
  FLOAT_MESH eps_find=1e-6;
  FLOAT_PHYS oriunit=1.0;
  std::vector<FLOAT_PHYS> orislist={};
  //
  //FIXME Consider using C++ for parsing command line options.
  {//Scope these variables
    std::vector<int> tagslist={},nodelist={},dofslist={};
    //std::vector<int> volutags={},volutmp={};
    std::vector<int>  parttags={}, axislist={},parttmp={};
    std::vector<FLOAT_PHYS> younlist={}, poislist={}, smodlist={}, matclist={},
      rdeglist={}, avallist={};
    //  younlist, poislist, smodlist, matclist, rdeglist;
    //std::unordered_map<int,std::vector<int>> axislist;
    //int volutag=0, parttag=0;
    double uval=0.0, fval=0.0;
    bool fix0=false, load=false, disp=false;
    bool hasmatp=false, hasmatc=false, rotrand=false, mtrldone=true;
    //
    opterr = 0; int c;
    while ((c = getopt (argc, argv,
      "abcpv:t:n:@:xyz0u:f:M:X:Y:Z:E:N:G:C:B:O:R")) != -1){
      // x:  x requires an argument
      mtrldone=true;
      switch (c) {
        // Input filename(s)
        case 'v':{ verbosity = atoi(optarg);M->verbosity=verbosity; break; }
        case 'p':{ is_part = true; break; }
        //case 'P':{ is_part = true; part_n=atoi(optarg); break;}// pstr = optarg;
        // Output format
        case 'a':{ save_asc=true; break; }
        case 'b':{ save_bin=true; break; }
        case 'c':{ save_csv=true; break; }
        // Boundary Conditions
        case 'n':{ nodelist.push_back(atoi(optarg)); uval=0.0; fval=0.0; break; }
        case 't':{ tagslist.push_back(atoi(optarg)); uval=0.0; fval=0.0; break; }
        case '@':{ avallist.push_back(atof(optarg)); uval=0.0; fval=0.0; break; }
        case 'x':{ dofslist.push_back(0); break; }
        case 'y':{ dofslist.push_back(1); break; }
        case 'z':{ dofslist.push_back(2); break; }
        case '0':{ fix0=true; ;break; }
        case 'u':{ disp=true; uval=atof(optarg); break; }
        case 'f':{ load=true; fval=atof(optarg); break; }
        // Physics
        //case 'T':{ // Gmsh volume physical ID tag
        //  if(hasmatc|hasmatp){ volutmp.push_back(atoi(optarg));
        //  }else{
        //    mtrldone=false;volutags.push_back(atoi(optarg));
        //  };break; }
        //case 'M':{ 
        case 'M':{ 
          if(hasmatc|hasmatp){ parttmp.push_back(atoi(optarg));
          }else{mtrldone=false;parttags.push_back(atoi(optarg));
          };break; }// Mesh partition number
        case 'C':{ matclist.push_back(atof(optarg)); hasmatc=true; mtrldone=false;break; }
        case 'E':{ younlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'N':{ poislist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'G':{ smodlist.push_back(atof(optarg)); hasmatp=true; mtrldone=false;break; }
        case 'X':{ axislist.push_back(0); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
        case 'Y':{ axislist.push_back(1); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
        case 'Z':{ axislist.push_back(2); rdeglist.push_back(atof(optarg)); mtrldone=false;break; }
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
      };
      if( hasmatp & hasmatc){
        fprintf (stderr,"ERROR Specify only material properties:\n");
        fprintf (stderr,"      -E<Young's modulus> -N<Poisson's ratio> -G<shear modulus>...\n");
        fprintf (stderr,"   OR material response matrix values:\n");
        fprintf (stderr,"      -C<c11>...\n");
        return 1; };
#if VERB_MAX>3
      if(fix0){
        printf("Tags:");
        for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); };
        printf(", Nodes:");
        for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); };
        printf(", @:");
        for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); };
        printf(" DOFs:");
        for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); };
        printf(", Displacement fixed zero\n");
      };
      if(disp){
        printf("Tags:");
        for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); };
        printf(", Nodes:");
        for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); };
        printf(", @:");
        for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); };
        printf(" DOFs:");
        for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); };
        printf(", Displacement: %+9.2e\n",uval);
      };
      if(load){
        printf("Tags:");
        for(size_t i=0;i<tagslist.size();i++){ printf(" %i",tagslist[i]); };
        printf(", Nodes:");
        for(size_t i=0;i<nodelist.size();i++){ printf(" %i",nodelist[i]); };
        printf(", @:");
        for(size_t i=0;i<avallist.size();i++){ printf(" %f",avallist[i]); };
        printf(" DOFs:");
        for(size_t i=0;i<dofslist.size();i++){ printf(" %i",dofslist[i]); };
        printf(", Load: %+9.2e\n",fval);
      };
#endif
      if( fix0 | disp | load ){
        for(size_t f=avallist.size();f<dofslist.size();f++){
          for(size_t i=0;i<nodelist.size();i++){
            if(fix0){ M->bc0_nnf.push_back(Gmsh::tfitem(nodelist[i],dofslist[f])); };
            if(disp){ M->bcs_nvals.push_back(Gmsh::tfval(nodelist[i],dofslist[f],uval)); };
            if(load){ M->rhs_nvals.push_back(Gmsh::tfval(nodelist[i],dofslist[f],fval)); };
          };
          for(size_t i=0;i<tagslist.size();i++){
            if(fix0){ M->bc0_tnf.push_back(Gmsh::tfitem(tagslist[i],dofslist[f])); };
            if(disp){ M->bcs_tvals.push_back(Gmsh::tfval(tagslist[i],dofslist[f],uval)); };
            if(load){ M->rhs_tvals.push_back(Gmsh::tfval(tagslist[i],dofslist[f],fval)); };
          };
          for(size_t i=0;i<avallist.size();i++){
            if(fix0){ bc0_at.insert(
              ati((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i])); };
            if(disp){ bcs_at.insert(
              atv((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i],uval)); };
            if(load){ rhs_at.insert(
              atv((INT_DOF)dofslist[i],(INT_DOF)dofslist[f],avallist[i],fval)); };
          };
        };
      };
      if(fix0){ nodelist={};tagslist={};dofslist={};avallist={}; fix0=false; };
      if(disp){ nodelist={};tagslist={};dofslist={};avallist={}; disp=false; };
      if(load){ nodelist={};tagslist={};dofslist={};avallist={}; load=false; };
      //
      //if( mtrldone & ((volutags.size()+parttags.size()) > 0) ){
      if( mtrldone & (parttags.size() > 0) ){
#if VERB_MAX>4
        if(verbosity>4){
        //std::cout << "Volume tags:";
        //  for(int v : volutags ){ std::cout << " " << v; };
        std::cout << ", ";
        std::cout << "Partitions:";
          for(int v : parttags ){ std::cout << " " << v; };
        std::cout << std::endl;
        //
        std::cout << "Young:";
          for(FLOAT_PHYS v : younlist ){ std::cout << " " << v; };
        std::cout << std::endl;
        std::cout << "Poiss:";
          for(FLOAT_PHYS v : poislist ){ std::cout << " " << v; };
        std::cout << std::endl;
        std::cout << "Shear:";
          for(FLOAT_PHYS v : smodlist ){ std::cout << " " << v; };
        std::cout << std::endl;
        std::cout << " matC:";
          for(FLOAT_PHYS v : matclist ){ std::cout << " " << v; };
        std::cout << std::endl;
        std::cout << " Rdeg:";
          for(FLOAT_PHYS v : rdeglist ){ std::cout << " " << v; };
        std::cout << std::endl;
        std::cout << "RAxes:";
          for(int v : axislist ){ std::cout << " " << v; };
        std::cout << std::endl;
        };
#endif
        {uint n= uint(3)-rdeglist.size();
        if(rotrand){ for(uint i=0; i<n; i++){
          rdeglist.push_back(FLOAT_PHYS(std::rand())/(FLOAT_PHYS(RAND_MAX)+1.0)
            * 2.0*PI );//FIXME These are overwritten later if allrand==true.
          if(n==3){allrand=true;};//FIXME Not quite the way to do this
          }; }; }
        if(rotfile){ rdeglist={0.0,0.0,0.0};};
        for(int p : parttags ){
          for(FLOAT_PHYS v : rdeglist ){ mtrl_part[p].push_back(v); };
          for(FLOAT_PHYS v : younlist ){ mtrl_part[p].push_back(v); };
          for(FLOAT_PHYS v : poislist ){ mtrl_part[p].push_back(v); };
          for(FLOAT_PHYS v : smodlist ){ mtrl_part[p].push_back(v); };
        };
        //for(int p : volutags ){
        //  for(FLOAT_PHYS v : rdeglist ){ mtrl_volu[p].push_back(v); };
        //  for(FLOAT_PHYS v : younlist ){ mtrl_volu[p].push_back(v); };
        //  for(FLOAT_PHYS v : poislist ){ mtrl_volu[p].push_back(v); };
        //  for(FLOAT_PHYS v : smodlist ){ mtrl_volu[p].push_back(v); };
        //};
        //
        mtrldone=true; hasmatc=false; hasmatp=false; rotrand=false;
        parttags=parttmp;//volutags=volutmp;
        younlist={}; poislist={}; smodlist={}; matclist={};
        rdeglist={}; axislist={};
         parttmp={};// volutmp={};
      };// Done getting material property set
      //
    };// end argument parsing loop
#if VERB_MAX>2
    if(verbosity>2){
    if(mtrl_part.size()>0){
      std::cout << "Partition Physics: "<< std::endl;;
      for (std::pair<int,std::vector<FLOAT_PHYS>> pr : mtrl_part){
        std::cout << "["<< pr.first <<"]";
        for(FLOAT_PHYS v : pr.second ){ std::cout <<" "<<v; };
        std::cout << std::endl;
      }; };
    if(mtrl_volu.size()>0){
      std::cout << "Tagged Volume Physics:" << std::endl;
      for (std::pair<int,std::vector<FLOAT_PHYS>> pr : mtrl_volu){
        std::cout << "["<< pr.first <<"]";
        for(FLOAT_PHYS v : pr.second ){ std::cout <<" "<<v; };
        std::cout << std::endl;
      }; };
    };
#endif
    //if(!is_part){ std::cout << "ERROR Must specify -p or -P n" <<'\n'; return 1;};
    //FIXME Does not yet read unpartitioned meshes.
    for (int i = optind; i < argc; i++){
      if(i<(argc-1)){
      fprintf (stderr, "WARNING Ignoring command line option: %s.\n", argv[i]);
      }else{
      bname = argv[i];
      };
    };
    if(rotfile){
      //FILE* ofile;
      //ofile = fopen(oriname.c_str(),"r");
      std::ifstream ofile(oriname);
      //if (ofile==NULL){
      //  std::cout << "ERROR Could not open " <<oriname<< " for reading."<<std::endl;
      //  return 1;
      //}else{
#if VERB_MAX>1
      if(verbosity>1){
        std::cout << "Reading Bunge (deg) crystal orientations from " 
        << oriname << "..." <<'\n'; };
#endif
        FLOAT_PHYS o;
        while( ofile>>o ){ orislist.push_back( o * oriunit); };
        //fclose (ofile);
      //};
      if(orislist.size()==0){
        std::cout << "ERROR No orientation data found in " <<oriname<< "."<<std::endl;
        return 1;
      }
    };
    // Done parsing command line options.
    //=======================================================
    if(!(save_asc | save_bin | save_csv)){ save_asc = true; };
    if(bname == NULL){
      std::cerr << "ERROR Mesh partition base filename not provided." << '\n';
      return 1;}
    else{
      if(is_part){
#if VERB_MAX>1
        if(verbosity>0){
      printf ("Looking for Gmsh partitions of %s...\n", bname); };
#endif
      bool fok=true; INT_MESH_PART part_i=1;
      while( fok ){
      //for(int part_i=1; part_i<(part_n+1);part_i++){
        std::stringstream ss;
        ss << bname << "_" << part_i << ".msh" ;
        pname = ss.str();
        // Check if can open file.
        FILE* pfile;
        pfile = fopen(pname.c_str(),"r");
        if (pfile==NULL){// fclose(pfile);
          ss << "2";//bname << "_" << part_i << ".msh2" ;
          pname = ss.str();
          pfile = fopen(pname.c_str(),"r");
          if (pfile==NULL){ fok = false;//  fclose(pfile); 
          }else{part_i++; iname=bname; file_ext=".msh2";
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "..." <<'\n'; };
#endif
          fclose (pfile); }
        }else{// part_0=1;
          part_i++; iname=bname;
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "..." <<'\n'; };
#endif
          fclose (pfile);
          };
        };
      part_n = part_i-1;
      }else{
        std::stringstream ss;
        //ss << bname << ".msh" ;
        ss << bname ;
        pname = ss.str();
        // Check if can open file.
        FILE * pfile;
        pfile = fopen(pname.c_str(),"r");
        if (pfile==NULL){// fclose(pfile);
          ss << ".msh" ;
          pname = ss.str();
          pfile = fopen(pname.c_str(),"r");
          if (pfile==NULL){// fclose(pfile);
            ss << "2" ;
            pname = ss.str();
            pfile = fopen(pname.c_str(),"r");
            if (pfile==NULL){// fclose(pfile);
              std::cout << "ERROR opening " << bname << " for reading." << '\n';
              return 1;
            }}}
//            }else{ part_n=1;
//#if VERB_MAX>1
//            std::cout << "Found " << pname << "..." << '\n';
//#endif
//            iname = pname.c_str();
//            fclose (pfile); }
//          }else{ part_n=1;// pname=pname.c_str();// part_0=0;
//#if VERB_MAX>1
//            std::cout << "Found " << pname << "..." << '\n';
//#endif
//            iname = pname.c_str();
//            fclose (pfile); };
//        }else{//part_0=0;
          part_n=1; iname=pname.c_str();
#if VERB_MAX>1
        if(verbosity>1){
          std::cout << "Found " << pname << "..." << '\n'; };
#endif
          if(pfile!=NULL){ fclose (pfile); };
//          };
      };
    };
    if(part_n>0){ part_0=0;// is_part=false;
      if(part_n>1){ part_0=1;// is_part=true;
#if VERB_MAX>1
        if(verbosity>0){
      printf ("Found %u mesh partitions.\n", part_n);}; };
#endif
    }else{
      std::cerr << "ERROR No mesh partition files could be opened for reading."
        << '\n'; 
      return 1;
    };
  }//End scope of command line parsing variables
  //=================================================================
  // Read gmsh files.
  //INT_ORDER pord=1; 
  //int part_0=1;//FIXME unpartitioned meshes in list_elem[1]
  //FIXME unpartitioned mesh should be in list_elem[0]
  //if( part_n>1 ){ part_0=1; }else{ part_0=0; };
  //FIXME Does not work in parallel
  std::vector<Elem*> partlist(part_n+part_0);
  Elem* E;// Gmsh* G;
#pragma omp parallel for schedule(static)
  for(int part_i=part_0; part_i < (part_n+part_0); part_i++){
    if(is_part){
    std::stringstream ss;
      ss << iname;  ss << "_" << part_i << ".msh" ;
      pname = ss.str();
    };
#if VERB_MAX>3
        if(verbosity>3){
    std::cout << "Reading " << pname << "..." <<'\n'; };
#endif
    E=M->ReadMsh2( pname.c_str() );
#pragma omp atomic write
    partlist[part_i]=E;
  };
  if(!is_part){ is_part=true;//part_0=1;//FIXME This determines first saved file.
    // Partition M[0] based on physical IDs...
    //std::unordered_map<int,INT_MESH_PART> glel_part;
    auto E0=partlist[0];
    uint(cn)=uint(E0->elem_conn_n);//printf("**** %u ****",cn);
        if(verbosity>1){
    std::cout << "Partitioning " << pname << " by " << M->elms_phid.size()
      <<" Gmsh volume physical IDs..." <<'\n'; };
    for(auto pr : M->elms_phid){ part_n++;
      Elem* E = new Tet( E0->elem_p, pr.second.size());
      //printf("elem_glid[%u]\n",uint(E->elem_glid.size()));
      //E->elem_glid=pr.second;
      //INT_MESH node_i=0;
        if(verbosity>2){
      std::cout << "Making partition " << (part_n-1) <<"..."<<std::endl; };
      E->node_n=0;
      for(INT_MESH e=0;e<E->elem_n;e++){
        auto glel=pr.second[e];
        E->elem_glid[e]=glel;
        auto loe0=E0->elem_loid[glel];
        //printf("%u : %u\n",glel,loe0);
        //glel_part[pr.second[e]] = part_n;
        //uint(cn)=uint(E->elem_conn_n);
        for(uint n=0;n<cn;n++){//printf("%u ",e);
          auto glno = E0->node_glid[E0->elem_conn[cn* loe0+n]];
          //E->elem_conn[cn* e+n] = glno;
          if( E->node_loid.count(glno)==0 ){
            E->node_loid[glno]=E->node_n; E->node_n++; };
          E->elem_conn[cn* e+n] = E->node_loid[glno];
        };
      };
      //E->node_n=999;
      uint d=uint(E->mesh_d);
      E->vert_n=E->node_n;
      E->vert_coor.resize(d*E->vert_n);
      E->node_glid.resize(E->node_n);
      //for(INT_MESH i=0; i<node_n; i++){ node_glid[i]=; };
      for( auto pr : E->node_loid ){
        int glid; INT_MESH l;
        std::tie(glid,l)=pr;// printf("%u:%i\n",glid,l);
        E->node_glid[l]=glid;
        INT_MESH e0=E0->node_loid[glid];
        for(uint i=0;i<d;i++){
          E->vert_coor[d* l+i ]=E0->vert_coor[d* e0+i ]; };
      };
      partlist.push_back(E);
    };
  };// end partition by gmsh volume physical IDs.
  M->list_elem = partlist;
  //
#if VERB_MAX>1
  if(verbosity>0){
    printf("Applying boundary conditions...\n");
    if(verbosity>1){
    FLOAT_MESH loc,amt; INT_DOF f,g;
    for(auto tp : bc0_at){ std::tie(f,g,loc)=tp;
      printf("BC0 @DOF %u == %f: set DOF %u to zero.\n", uint(f),loc,uint(g)); };
    for(auto tp : bcs_at){ std::tie(f,g,loc,amt)=tp;
      printf("BCS @DOF %u == %f: set DOF %u = %f.\n", uint(f),loc,uint(g),amt); };
    for(auto tp : rhs_at){ std::tie(f,g,loc,amt)=tp;
      printf("RHS @DOF %u == %f: set DOF %u = %f.\n", uint(f),loc,uint(g),amt); };
    }; };
#endif
  //
  if( (bc0_at.size()+bcs_at.size()+rhs_at.size()) >0 ){
    // Boundary conditions @
    int glid; INT_MESH loid;
    FLOAT_MESH loc,amt; INT_DOF f,g;
    for(uint e=1; e<M->list_elem.size(); e++){
      Elem* E=M->list_elem[e];
      uint d=uint(E->mesh_d);
      for( auto pr : E->node_loid ){ std::tie(glid,loid)=pr;
        for(auto tp : bc0_at){ std::tie(f,g,loc)=tp;
          if(std::abs(E->vert_coor[d* loid+f ]-loc)<eps_find){
            E->bc0_nf.insert(Mesh::nfitem(loid,g)); }; };
        for(auto tp : bcs_at){ std::tie(f,g,loc,amt)=tp;
          if(std::abs(E->vert_coor[d* loid+f ]-loc)<eps_find){
            E->bcs_vals.insert(Mesh::nfval(loid,g,amt)); }; };
        for(auto tp : rhs_at){ std::tie(f,g,loc,amt)=tp;
          if(std::abs(E->vert_coor[d* loid+f ]-loc)<eps_find){
            E->rhs_vals.insert(Mesh::nfval(loid,g,amt)); }; };
      };
    };
  };// end applying BC@
  //
  M->list_elem[0]=NULL; if(part_0==0){part_0=1; part_n-=1;}//FIXME
  M->SyncIDs();//FIXME need to skip [0] when syncing
#if VERB_MAX>2
  if(verbosity>2){
  for(uint i=0; i<M->list_elem.size(); i++){
    std::cout << "Mesh Partition " << i ;
    if(M->list_elem[i]==NULL){std::cout << " is null.\n";
    }else{
      E=M->list_elem[i];
      std::cout << ": " << E->node_n << " Nodes ["
        << E->vert_coor.size()<<"], "
        << E->elem_n << " Elems ["
        << E->elem_conn.size()<<"].";
      std::cout <<'\n' ;
#if VERB_MAX>3
    if(verbosity>3){
      printf("Nodes:");
      for(uint j=0;j<E->vert_coor.size();j++){
        if(!(j%3)){printf("\n%3u(%3u):",j/3,E->node_glid[j/3]);};
        printf(" %9.2e",E->vert_coor[j]);
      };printf("\n");
      printf("Elements:");
      auto ecn=uint(E->elem_conn_n);
      for(uint j=0;j<E->elem_conn.size();j++){
        if(!(j%ecn)){printf("\n%3u(%3u):",j/ecn,E->elem_glid[j/ecn]);};
        printf(" %3u",E->elem_conn[j]);
      };printf("\n");
    INT_MESH n; INT_DOF f; FLOAT_SOLV v;
    std::cout << "RHS:" << '\n';
    for(auto t : E->rhs_vals){ std::tie(n,f,v)=t;
      std::cout << n << ":" << uint(f) << " " << v <<'\n'; };
    std::cout << "BCs:" << '\n';
    for(auto t : E->bcs_vals){ std::tie(n,f,v)=t;
      std::cout << n << ":" << uint(f) << " " << v <<'\n'; };
    std::cout << "BC0:" << '\n';
    for(auto t : E->bc0_nf  ){ std::tie(n,f)=t;
      std::cout << n << ":" << uint(f) <<'\n'; };
    };
#endif
    };//end if this Elem is defined
  };
  };
#endif
  if(save_csv){
#pragma omp parallel for schedule(static)
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      std::stringstream ss;
      ss << bname << "_" << part_i ;
      std::string pname = ss.str();
      std::cout << "Saving part " << pname << "..." <<'\n';
      M->list_elem[part_i]->SavePartCSV( pname.c_str() );
    };
  };
  if(save_asc | save_bin){
    if(verbosity==1){ 
      printf("Saving and appending physics to partitions...\n"); };
#pragma omp parallel for schedule(static)
    for(int part_i=part_0;part_i<(part_n+part_0);part_i++){
      Phys::vals props={100e9,0.3};//FIXME Should not have defaults here...
      Phys::vals dirs={0.0,0.0,0.0};
      Phys::vals prop={};
      auto m0=mtrl_part[0];
      std::stringstream ss;
      ss << bname;
      if(is_part){ ss << "_" << part_i ;};
      ss << ".fmr";
      std::string pname = ss.str();
      if(verbosity>1){
      std::cout << "Saving part " << pname << "..." <<'\n'; };
      //if(save_bin){
      //  M->list_elem[part_i]->SavePartFMR( pname.c_str(), true  ); };
      if(save_asc){
        M->list_elem[part_i]->SavePartFMR( pname.c_str(), false );
        Phys* Y;
        auto mp=mtrl_part[part_i];
        if(mp.size()>0){
          props.resize(mp.size());
          for(uint i=0;i<mp.size();i++){ props[i]=mp[i]; };
        }else if(m0.size()>0){
          props.resize(m0.size());
          for(uint i=0;i<m0.size();i++){ props[i]=m0[i]; };
          if(allrand &(props.size()>3)){
            for(uint i=0; i<3; i++){
              props[i]=FLOAT_PHYS(std::rand())/(FLOAT_PHYS(RAND_MAX)+1.)*2.*PI;
              };
          };
          if(rotfile &(props.size()>3)){
            for(uint i=0; i<3; i++){
              props[i]=orislist[3* (part_i-part_0)+i ];
              };
          };
        };
        if(props.size()>3){
          for(uint i=0;i<3;i++){ dirs[i]=props[i]; };
          prop.resize(props.size()-3);
          for(uint i=3;i<props.size();i++){ prop[i-3]=props[i]; };
          Y=new ElastOrtho3D(prop,dirs);
        }else{
          prop.resize(props.size());
          for(uint i=0;i<props.size();i++){ prop[i]=props[i]; };
          Y=new ElastIso3D(prop[0],prop[1]);
        };
        if(verbosity>1){
          std::cout << "Appending physics to " << pname << "..." <<'\n'; };
        Y->SavePartFMR( pname.c_str(), false );
      };
    };
  };
  //if(save_bin){
  //  std::cout << "ERROR Binary save not yet implemented. " << '\n';
  //};
  //
  return 0;
};
