#include "../Main/Plug.hpp"

#include "hdf5.h"
#ifdef FMR_HAS_MPI
#include "pcgnslib.h" // cgp_*
#else
#include "cgnslib.h"  // cg_*
#endif

#include <cstring>    // strcpy
#include <sys/stat.h> // stat, S_ISDIR

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

// Do NOT start names with cg or cgp (upper/lowercase)
//NOTE cg_free is only needed if certain routines are used.

namespace fmr {namespace detail {//TODO move to Femera namespace?
  static const std::map <Femera::Dcgn::Concurrency, std::string>
    concurrency_name ={
    {Femera::Dcgn::Concurrency::       Error,"concurrency access error"},
    {Femera::Dcgn::Concurrency::      Serial,"serial"},
    {Femera::Dcgn::Concurrency:: Independent,"independent"},
    {Femera::Dcgn::Concurrency::  Collective,"collective"}
  };
  static const std::map <Femera::Dcgn::File_format, std::string>
    format_cgns_name ={
    {Femera::Dcgn::File_format::    Dcgn_none,"unformatted"},
    {Femera::Dcgn::File_format::    Dcgn_ADF1,"ADF"},
    {Femera::Dcgn::File_format::    Dcgn_HDF5,"HDF5"},
    {Femera::Dcgn::File_format::    Dcgn_ADF2,"ADF2"},
    {Femera::Dcgn::File_format:: Dcgn_unknown,"unknown format"}
  };
} }//end fmr::detail:: namespace
namespace Femera {// header extension: needs cgnslib.h
  //NOTE Use mapname.at(key) to get value, NOT mapname[key].
  static const std::map<int,Dcgn::File_format> dcgn_format_from_cgns ={
    { CG_FILE_NONE, Dcgn::File_format:: Dcgn_none },
    { CG_FILE_ADF , Dcgn::File_format:: Dcgn_ADF1 },
    { CG_FILE_HDF5, Dcgn::File_format:: Dcgn_HDF5 },
    { CG_FILE_ADF2, Dcgn::File_format:: Dcgn_ADF2 }
  };
  static const std::map<fmr::data::Access,int> dcgn_cg_open_mode ={
    { fmr::data::Access::    New, CG_MODE_WRITE  },
    { fmr::data::Access::  Check, CG_MODE_CLOSED },
    { fmr::data::Access::  Error, CG_MODE_CLOSED },
    { fmr::data::Access::  Close, CG_MODE_CLOSED },
    { fmr::data::Access::   Read, CG_MODE_READ   },
    { fmr::data::Access::  Write, CG_MODE_WRITE  },
    { fmr::data::Access:: Modify, CG_MODE_MODIFY }
  };
  static const std::map<ElementType_t,fmr::Elem_form> elem_form_from_cgns ={
    {ElementTypeNull, fmr::Elem_form::None},
    {ElementTypeUserDefined, fmr::Elem_form::User},
    {NODE    , fmr::Elem_form::Node},
    //
    {BAR_2   , fmr::Elem_form::Line},
    {BAR_3   , fmr::Elem_form::Line},
    {TRI_3   , fmr::Elem_form::Tris},
    {TRI_6   , fmr::Elem_form::Tris},
    //
    {TETRA_4 , fmr::Elem_form::Tets},
    {TETRA_10, fmr::Elem_form::Tets},
    //TODO ...
    {BAR_4   , fmr::Elem_form::Line},
    {TRI_9   , fmr::Elem_form::Tris},
    {TRI_10  , fmr::Elem_form::Tris},
    //
    {TETRA_16, fmr::Elem_form::Tets},
    {TETRA_20, fmr::Elem_form::Tets},
    //
    {HEXA_64 , fmr::Elem_form::Cube}
  };
  /*
    {PYRA_5  , fmr::Elem_form::Pyra},
    {PYRA_14 , fmr::Elem_form::Pyra},
    {PENTA_6 , fmr::Elem_form::Prsm},
    {PENTA_15, fmr::Elem_form::Prsm},
    {PENTA_18, fmr::Elem_form::Prsm},
    {HEXA_8  , fmr::Elem_form::Cube},
    {HEXA_20 , fmr::Elem_form::Cube},
    {HEXA_27 , fmr::Elem_form::Cube},
     ElementTypeNull, ElementTypeUserDefined, NODE, BAR_2, BAR_3,
     TRI_3, TRI_6, QUAD_4, QUAD_8, QUAD_9,
     TETRA_4, TETRA_10, PYRA_5, PYRA_14,
     PENTA_6, PENTA_15, PENTA_18, HEXA_8, HEXA_20, HEXA_27,
     MIXED, PYRA_13, NGON_n, NFACE_n,
     BAR_4, TRI_9, TRI_10, QUAD_12, QUAD_16,
     TETRA_16, TETRA_20, PYRA_21, PYRA_29, PYRA_30,
     PENTA_24, PENTA_38, PENTA_40, HEXA_32, HEXA_56, HEXA_64
     */
}//end Femera namespace for header extension
namespace Femera {
  Dcgn::Dcgn (Proc* P, Data* D) noexcept:
    format (Dcgn::File_format::Dcgn_HDF5),
    comm   (Proc::Team_id( MPI_COMM_WORLD )) {// changed in init_task(..)
    this->proc=P; this->data=D;
    this->work_type = work_cast (Plug_type::Dcgn);
//    this->base_type = work_cast (Base_type::Data);
    this->task_name ="CGNS";
    this->verblevel = 2;
  }
#if 0
   //TODO set_concurrency (File_cgns)
  Dcgn::Concurrency Dcgn::set_concurrency (Dcgn::Concurrency conc){int err=0;
    if (this->get_concurrency() != conc){
      FMR_PRAGMA_OMP(omp single nowait){
      fmr::perf::timer_resume(&this->time);
        switch (conc){
          case Dcgn::Concurrency:: Independent :{
            err= cgp_pio_mode (CGP_INDEPENDENT );
            break;
          }
          case Dcgn::Concurrency:: Collective :{
            err= cgp_pio_mode (CGP_COLLECTIVE );
            break;
          }
          default :{}// No action needed.
        }
        fmr::perf::timer_pause (&this->time);
        if (err){
          this->proc->log->printf(
            "WARN""ING could not set CGNS concurrency to %s.\n",
            fmr::detail::concurrency_name[conc].c_str() );
          this->proc->log->printf ("WARN""ING CGNS concurrency is %s.\n",
            fmr::detail::concurrency_name[this->concurrency].c_str() );
        }else{
          this->concurrency = conc;
#ifdef FMR_DEBUG
          this->proc->log->printf ("DEBUG CGNS concurrency is %s\n",
            fmr::detail::concurrency_name[this->concurrency].c_str() );
#endif
    } } }
    return this->concurrency;
  }
#endif
  int Dcgn::prep (){int err=0;
#ifdef FMR_DEBUG
    std::printf("*** Dcgn::prep() start...\n");
#endif
#if 0
    this->version        = std::to_string( CGNS_DOTVERS );
    // painful to make CGNS_DOTVERS look nice
#else
    this->version = std::to_string( CGNS_VERSION / 1000 )
      + "." + std::to_string( CGNS_VERSION % 1000 );
#endif
#ifdef FMR_HAS_MPI
    this->version+=" (parallel)";//TODO Correct this.
#endif
    this->file_exts ={"cgn","cgns","adf","adf2"};// case-insensitive
    fmr::perf::timer_pause (&this->time);
    return err;
  }
  int Dcgn::init_task (int*, char** ){int err=0;
#ifdef FMR_DEBUG
    std::printf("*** Dcgn::init_task start [%i]...\n",err);
#endif
#ifdef FMR_HAS_MPI
    FMR_PRAGMA_OMP(omp master)
    if (this->proc!=nullptr){
      Proc* P = this->proc->task.first<Proc> (Plug_type::Pmpi);
      if (P!=nullptr){// Set the communicator
        MPI_Comm c;
        fmr::perf::timer_start (& this->time );
        err= MPI_Comm_dup (MPI_Comm(P->get_team_id()), &c);
        fmr::perf:: timer_pause (&this->time);
        this->comm = Proc::Team_id(c);
#if 0
        if(this->proc->log->detail > 1 ){
          this->proc->log->label_printf("CGNS mode",
            "Parallel access using %i MPI threads/team\n",//TODO
            this->proc->task.first<Proc>( Plug_type:: Pmpi )->get_proc_n() );
        }
#endif
    } }
#endif
    if (err){return 1; }
    return this->prep ();
  }
  int Dcgn::exit_task (int err ) {
    this->close ();
//TODO    FMR_PRAGMA_OMP(omp barrier)
    FMR_PRAGMA_OMP(omp master)
      if (this->comm != Proc::Team_id( MPI_COMM_WORLD )){
        MPI_Comm c=MPI_Comm(this->comm);
        err= MPI_Comm_free (&c);
    }
    return err;
  }
bool Dcgn::is_this_type (const std::string fname) {int err;
  int fmt = int(Dcgn::File_format::Dcgn_unknown);
  err= fmr::perf::time_activity<int> (&this->time,
    cg_is_cgns, fname.c_str(), &fmt);
  return err==0;
}
int Dcgn::make_mesh (const std::string, const fmr::Local_int) {
  return 1;
}
int Dcgn::make_part (const std::string, const fmr::Local_int) {
  return 1;
}
Data::File_info Dcgn::get_file_info (const std::string fname) {int err=0;
  //TODO Dcgn::get_file_info (..) is NOT thread safe. lock guard here?
  auto info = Dcgn::File_cgns (Data::Data_file (this,fname));
  info = this->file_info.count(fname) ? this->file_info.at(fname) : info;
  int fmt = int(Dcgn::File_format::Dcgn_unknown);
  err= fmr::perf::time_activity<int> (&this->time,
    cg_is_cgns, fname.c_str(), &fmt);//TODO Redundandant to is_this_type(..)?
  if (err) {
    info.state.has_error=true;
    this->file_info [fname] = info;
    return info;
  }
  info.data_file.first = this;
  info.state.can_read = true;
  info.format = (dcgn_format_from_cgns.count (fmt)>0)
    ? dcgn_format_from_cgns.at (fmt) : Dcgn::File_format::Dcgn_unknown;
  info.version = info.version.size()
    ? info.version
    : fmr::get_enum_string(fmr::detail::format_cgns_name, info.format);
    this->file_info [fname] = info;//TODO lock guard here?
  return info;
}
Dcgn::File_cgns Dcgn::close (const std::string fname){
  auto info = Dcgn::File_cgns (Data::Data_file (this,fname));
  info = this->file_info.count(fname) ? this->file_info.at(fname) : info;
  int err=0;
  fmr::perf::timer_resume (&this->time);
  switch (info.access){
    case fmr::data::Access::Read   :// Fall through.
    case fmr::data::Access::Write  :// Fall through.
    case fmr::data::Access::Modify :{err= cg_close (info.file_cgid); break;}
    default:{}// Do nothing.
  }
  fmr::perf::timer_pause (&this->time);
  if (err) {info.access = fmr::data::Access::Error; info.state.has_error = true;}
  else {info.access = fmr::data::Access::Close;}
  this->file_info[fname] = info;
  return info;
}
Dcgn::File_cgns Dcgn::open (const std::string fname,
  fmr::data::Access for_access, Data::Concurrency for_concurrency){
  auto info = Dcgn::File_cgns (Data::Data_file (this,fname));
  info = this->file_info.count(fname) ? this->file_info.at(fname) : info;
  int err=0; auto log = this->proc->log;
  switch (for_access){
    case fmr::data::Access::Read   :// Fall through.
    case fmr::data::Access::Write  :// Fall through.
    case fmr::data::Access::Modify :// Fall through.
    case fmr::data::Access::New    :{err = 0; break;}// valid open modes
    default:{err = 1;}// not a valid open mode
  }
  if (err) {
    info.state.has_error = true;
    this->file_info[fname] = info;
    return info;
  }
  fmr::perf::timer_resume (&this->time);
  if ((for_access == info.access) && (for_concurrency == info.concurrency)
    && !info.state.has_error){
#if 0
    log->fprintf (log->fmrerr, "NOTE %s is already open in %s mode.\n",
      fname.c_str(),
      fmr::get_enum_string (fmr::data::Access_name, info.access).c_str());
#else
    return info;// already open in correct modes
#endif
  }else{// not open in correct modes
    info.state.has_error = false;
    const auto mode_cgns = dcgn_cg_open_mode.at(for_access);
    switch (info.access){// Close file if already open in the wrong mode.
      case fmr::data::Access::Read   :// Fall through.
      case fmr::data::Access::Write  :// Fall through.
      case fmr::data::Access::Modify :{
        err= cg_close (info.file_cgid);
        if (!err) {info.access = fmr::data::Access::Close;}
        break;}
      default:{}// Do nothing.
    }
    if (err) {
      info.state.has_error = true;
      this->file_info[fname] = info;
      return info;
    }
    // Open file for_access mode.
    if (for_concurrency == Data::Concurrency::Collective) {
      FMR_PRAGMA_OMP(omp single nowait)
      {
        err= cgp_open(fname.c_str(), mode_cgns, &info.file_cgid );
      }
    }else{
      err= cg_open (fname.c_str(), mode_cgns, &info.file_cgid );
  } }
  fmr::perf::timer_pause (&this->time);
  if (err) {
    info.state.has_error = true;
    info.access = fmr::data::Access::Error;
    this->file_info[fname] = info;
    return info;
  }
  else {info.access = for_access; info.concurrency = for_concurrency; };
  fmr::perf::timer_resume (&this->time);
  if (for_access == fmr::data::Access::New) {
#if 1
    if (info.access == fmr::data::Access::New
      || for_access == fmr::data::Access::New){//TODO
      struct ::stat st;// defined in sys/stat.h
      bool does_exist = ::stat (fname.c_str(), &st) == 0;
      if (does_exist){// already exists, so cannot create a new one
        const std::string errmsg ="WARN"
          "ING Cannot create new CGNS file '%s' because it already exists.\n";
        switch (for_concurrency){
          case Data::Concurrency::Serial      :// Fall through.
          case Data::Concurrency::Independent :{
            log->fprintf (log->fmrerr, errmsg.c_str(), fname.c_str());
            break; }
          default: log->printf_err (errmsg.c_str(), fname.c_str());
        }
        info.access = fmr::data::Access::Error;
        info.state.has_error = true;
        return info;
    } }
#endif
    // File creation: open in CGNS WRITE mode (done above),
    // then close the file and reopen in CGNS MODIFY mode.
    for_access = fmr::data::Access::Modify;
    const auto mode_cgns = dcgn_cg_open_mode.at(for_access);
    if (info.concurrency == Data::Concurrency::Collective) {
      FMR_PRAGMA_OMP(omp single nowait)
      {
        err+= cgp_close (info.file_cgid);
        err+= cgp_open (fname.c_str(), mode_cgns, &info.file_cgid);
      }
    }else{
      err+= cg_close (info.file_cgid);
      err+= cg_open (fname.c_str(), mode_cgns, &info.file_cgid);
  } }
  fmr::perf::timer_pause (&this->time);
  if (err) {
    info.state.has_error = true;
    info.access = fmr::data::Access::Error;
    this->file_info[fname] = info;
    return info;
  }
  else {info.access = for_access;}
  info.state.is_default = false;
  switch (info.access) {
    case fmr::data::Access::Close :{
      info.state.can_read  = false;
      info.state.can_write = false;
      break;}
    case fmr::data::Access::Read :{
      info.state.can_read  = true;
      info.state.can_write = false;
      break;}
    case fmr::data::Access::Write :{
      info.state.can_read  = false;
      info.state.can_write = true;
      break;}
    case fmr::data::Access::Modify :{
      info.state.can_read  = true;
      info.state.can_write = true;
      break;}
    default:{}//do nothing
  }
  if (!info.state.was_checked) {
    // It should be ok to read the following information in write-only mode.
    //
    // Get CGNS file type: HDF or ADF.
    int fmt = int(Dcgn::File_format::Dcgn_unknown);
    err= fmr::perf::time_activity<int> (&this->time,
      cg_get_file_type, info.file_cgid, &fmt);
    if (err) {
      info.state.has_error = true;
      this->file_info[fname] = info;
      return info;
    }
    else {info.format = dcgn_format_from_cgns.at(fmt); }
    //
    // Get CGNS file version.
    fmr::perf::timer_resume (&this->time);
    Ver_cgns_float file_version_cgns = nanf("undefined");
    cg_version (info.file_cgid, & file_version_cgns);
    std::vector<char> buf(16,0);
    std::snprintf (&buf[0],15,"%.1f", double(file_version_cgns));
    info.version = std::string (& buf[0])
      +" "+fmr::get_enum_string (fmr::detail::format_cgns_name, info.format);
    //
    // Get floating-point precision.
    int prec=0; err= cg_precision (info.file_cgid, &prec);
    fmr::perf::timer_pause (&this->time);
    if (err) {
      info.state.has_error = true;
      this->file_info[fname] = info;
      return info;
    }
    else {info.precision = fmr::data::Precision (prec); }//TODO Check this.
    info.state.was_checked = true;
  }
#if 0
  if (log->detail >= this->verblevel) {
    log->label_fprintf (log->fmrout, (this->task_name +" open").c_str(),
      "%s (v%s)\n", fname.c_str(), info.version.c_str());
  }
#endif
  this->file_info[fname] = info;
  return info;
}
int Dcgn::add_cgns_here (const std::string fname, const fmr::Data_id id,
  const fmr::Data type) {int err=0;
  const auto cid = this->make_data_id (id, type);
#ifdef FMR_DEBUG
  const auto log = this->proc->log;
    log->label_fprintf(log->fmrerr,"**** Dcgn add!","%s\n",cid.c_str());
#endif
  this->data_path [cid] = Path_cgns();// Add to map.
#if 0
  auto path = &this->data_path.at (cid);//TODO works?
#else
  auto path = &this->data_path [cid];
#endif
  path->comm = this->comm;
  path->type = type;
  fmr::perf::timer_resume (&this->time);
  // Get tree depth.
  err= cg_where (&path->file_cgid, &path->base, &path->deep, nullptr, nullptr);
  //TODO handle err
  fmr::perf::timer_pause (&this->time);
  if (!err) {this->time.bytes += 3*sizeof(int);}
#ifdef FMR_DEBUG
  log->label_fprintf(log->fmrerr,"**** Dcgn deep","%u\n", path->deep);
#endif
  if (path->deep > 0) {
    // Resize arrays.
    const std::string null33 = std::string(33,'\0');
    path->inds.resize (path->deep);
    path->strs.resize (path->deep, null33);
    path->labs.resize (0);
    for (auto& str : path->strs) {// Set C-style char pointers.
      path->labs.push_back (&str.front());
  } }
  fmr::perf::timer_resume (&this->time);
  // Read tree directly into Femera:Data::Path_cgns struct.
  err= cg_where (&path->file_cgid, &path->base, &path->deep,
    path->labs.data(), path->inds.data());//TODO handle err
  fmr::perf::timer_pause (&this->time);
  if (!err) {this->time.bytes += (3 + path->deep) * sizeof(int)
    + path->deep * this->label_size * sizeof(char);
  }
  err= this->data->add_data_file (cid, this, fname);//TODO handle err
  return err;
}
int Dcgn::read_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals &vals) {
  int err=0;
#ifdef FMR_DEBUG
  const auto log = this->proc->log;
  log->label_fprintf (log->fmrerr,"**** Dcgn read","%s\n",id.c_str());
#endif
  if (vals.stored_state.was_read) {return 0;}
  Path_cgns* path = nullptr;// location where this data should be found
  if (this->data_path.count (id) > 0) {
    path = &this->data_path [id];
    if (path == nullptr) {return 1;}//TODO set read err in vals.
#ifdef FMR_DEBUG
    if (path->inds.size() == 1) {
      log->label_fprintf(log->fmrerr,"**** Dcgn read","%s: %s:%i\n",id.c_str(),
        path->strs[0].c_str(),path->inds[0]);
    }
    if (path->inds.size() >= 2) {
      log->label_fprintf(log->fmrerr,"**** Dcgn read","%s: %s:%i/%s:%i\n",
        id.c_str(),
        path->strs[0].c_str(),path->inds[0],
        path->strs[1].c_str(),path->inds[1]);
    }
#endif
  }
  const auto pn = path->inds.size();
  bool is_found = false;
  switch (vals.type) {
    case fmr::Data::Jacs_dets :
      //TODO Check if stored in jacs user-defined data CGNS node
      if (!is_found) {// read from this location
//        if (vals.data.size() < (sz+nj)) {vals.data.resize (sz+nj);}
        Data::Lock_here lock (this->liblock);
        err= cg_golist (path->file_cgid, path->base, path->deep,
          path->labs.data(), path->inds.data());
        fmr::perf::timer_resume (&this->time);
//TODO  err+ = cg_user_data_read(int Index, char *Name);//cg[p]_array_read
        fmr::perf::timer_pause (&this->time);
      }
      break;
    case fmr::Data::Node_coor :{
      if (pn < 1) {return 1;}
      bool isok = false;
      int array_n=0; cgsize_t range_min[3]; cgsize_t range_max[3];
      DataType_t mem_datatype = RealDouble;//CGNS: RealSingle or RealDouble
      switch (sizeof (vals.data[0])) {
        case  4 : mem_datatype = RealSingle; isok = true; break;
        case  8 : mem_datatype = RealDouble; isok = true; break;
        default : isok = false;
      }
      if (!isok) {return 1;}
      Data::Lock_here lock (this->liblock);//TODO break this up ?
      err= cg_golist (path->file_cgid, path->base, path->deep,
        path->labs.data(), path->inds.data());//TODO one level up from path?
      if (err !=0) {return err;}
      err= cg_ncoords (path->file_cgid, path->base, path->inds[pn-1],
        &array_n);
      if (err !=0) {return err;}
      const fmr::Dim_int coor_d = (array_n < 3) ? fmr::Dim_int (array_n) : 3;
      //TODO check geom_d/mesh_d/elem_d ?
      vals.layout = fmr::data::Layout::Block;
      const auto sz = vals.data.size ();
      const cgsize_t node_n = sz / coor_d;
      range_min[0] = 1; range_max[0] = node_n;// unstructured: only first val
      fmr::perf::timer_resume (&this->time);
      //char *coordname: CoordinateX, CoordinateY, CoordinateZ
      if (err==0 && coor_d>0) {
        err= cg_coord_read (path->file_cgid, path->base, path->inds [pn-1],
          "CoordinateX", mem_datatype, &range_min[0], &range_max[0],
          &vals.data [0]);
        if (err==0) {this->time.bytes+= node_n * sizeof (vals.data[0]);}
        else {vals.stored_state.has_error = true;}
      }
      if (err==0 && coor_d>1) {
        err= cg_coord_read (path->file_cgid, path->base, path->inds [pn-1],
          "CoordinateY", mem_datatype, &range_min[0], &range_max[0],
          &vals.data [node_n]);
        if (err==0){this->time.bytes+= node_n * sizeof (vals.data[0]);}
        else {vals.stored_state.has_error = true;}
      }
      if (err==0 && coor_d>2) {
        err= cg_coord_read (path->file_cgid, path->base, path->inds [pn-1],
          "CoordinateZ", mem_datatype, &range_min[0], &range_max[0],
          &vals.data [node_n*2]);
        if (err==0) {this->time.bytes+= node_n * sizeof (vals.data[0]);}
        else {vals.stored_state.has_error = true;}
      }
      fmr::perf::timer_pause (&this->time);
//        if (err !=0) {return err;}//TODO Remove?
      is_found = true;
      vals.stored_state.was_read = true;
      break;}
    default : err=1;
      //err= Data::read_geom_vals (id, vals);//TODO Remove this call?
  }
  vals.stored_state.has_error |= (err!=0);
//  err= is_found ? 0 : 1;//TODO Remove?
  if (err >0) {return err;}
#if 0
  if (log->detail >= this->verblevel) {
    const std::string label = this->task_name+" read";
    log->label_fprintf (log->fmrout, label.c_str(), "%lu %s geom vals.\n",
      vals.data.size(), id.c_str());
  }
#else
  err= Data::read_geom_vals (id, vals);//TODO only prints msg?
#endif
  return err;
}
int Dcgn::read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals) {
  int err=0;
  const auto log = this->proc->log;
  if (vals.stored_state.was_read) {return 0;}
  Path_cgns* path = nullptr;
  if (this->data_path.count (id) > 0) {
//    path = &this->data_path.at(id);
    path = &this->data_path [id];
    if (path == nullptr) {return 1;}//TODO set read err in vals.
#ifdef FMR_DEBUG
    if (path->inds.size() ==0) {
      log->label_fprintf(log->fmrerr,"**** Dcgn","%s\n",id.c_str());
    }
    if (path->inds.size() ==1) {
      log->label_fprintf(log->fmrerr,"**** Dcgn","%s: %s:%i\n",id.c_str(),
        path->strs[0].c_str(),path->inds[0]);
    }
    if (path->inds.size() >= 2) {
      log->label_fprintf(log->fmrerr,"**** Dcgn","%s: %s:%i/%s:%i\n",id.c_str(),
        path->strs[0].c_str(),path->inds[0],
        path->strs[1].c_str(),path->inds[1]);
    }
#endif
  }
  const auto pn = path->inds.size();
  switch (vals.type) {
    case (fmr::Data::Elem_conn) : {
      cgsize_t conn_sz = 0;
      if (pn < 2) {err = 1;}
      else{
        Data::Lock_here lock (this->liblock);
        err= cg_golist (path->file_cgid, path->base, path->deep,
          path->labs.data(), path->inds.data());
        fmr::perf::timer_resume (&this->time);
        err+= cg_ElementDataSize (path->file_cgid, path->base,
          path->inds[pn-2], path->inds[pn-1], &conn_sz);
        fmr::perf::timer_pause (&this->time);
      }
      vals.stored_state.has_error |= (err != 0);
      if (err!=0) {return err;}
      if (conn_sz <= 0) {err = 1;}
      else{
        auto buf = std::valarray<cgsize_t> (conn_sz);
        {
          Data::Lock_here lock (this->liblock);
          err= cg_golist (path->file_cgid, path->base, path->deep,
            path->labs.data(), path->inds.data());
          fmr::perf::timer_resume (&this->time);
          err+= cg_elements_read (path->file_cgid, path->base,
            path->inds[pn-2], path->inds[pn-1], &buf[0], nullptr);
            // last arg: cgsize_t *ParentData
          fmr::perf::timer_pause (&this->time);
        }
        vals.stored_state.has_error |= (err !=0);
        if (err!=0) {return err;}
        this->time.bytes += conn_sz * sizeof (cgsize_t);
        if (vals.data.size() < size_t(conn_sz)) {vals.data.resize (conn_sz);}
        for (cgsize_t i=0; i<conn_sz; i++) {
          vals.data[i] = fmr::Local_int (buf[i]);//TODO XS sims only.
        }
        vals.stored_state.was_read = (err == 0);
      }
      break;}
    default : {
      const auto label = "WARN""ING "+this->task_name;
      log->label_fprintf(log->fmrerr, label.c_str(),
        "read_local_vals (%s,..) datatype not handled\n", id.c_str());
  } }//end switch (vals.type)
  vals.stored_state.has_error |= err > 0;
  Data::read_local_vals (id, vals);//TODO Remove this call? only prints msg?
  return err;
}
Data::File_info Dcgn::scan_file_data (const std::string fname) {
  auto log = this->proc->log;
  auto info = Dcgn::File_cgns (Data::Data_file (this,fname));
  info = this->file_info.count(fname) ? this->file_info.at(fname) : info;
  if (!info.state.was_read) {
    bool do_open = true;
    switch (info.access) {// Check if already open.
      case fmr::data::Access::Read   :{}//valid open modes.
      case fmr::data::Access::Write  :{}
      case fmr::data::Access::Modify :{do_open = false; break;}
      default: {do_open = true;}
    }
    if (info.state.has_error) {do_open=true;}// Try to reopen.
    if (do_open) {
#if 0
      //TODO if output files same as input, assume write back to input files
      const auto inp_file_mode
        = this->get_inp_file_names().size()==this->get_out_file_names().size()
        ? fmr::data::Access::Modify : fmr::data::Access::Read;
#else
      const auto inp_file_mode = fmr::data::Access::Read;
#endif
      info = this->open (fname, inp_file_mode, Data::Concurrency::Independent);
    }
    if (info.state.has_error) {return info;}
    //Read CGNS base names
    int err=0;//TODO use this to throw exception
    // Material properties are in DataArray_t nodes,
    // and can be in Base_t, Zone_t or Family_t nodes.
    int base_n=0, geom_d=0, phys_d=0;
    // Try reading basenames from the CGNS file.
    err= fmr::perf::time_activity<int> (&this->time,
      cg_nbases, info.file_cgid,& base_n);
    if (err) {info.state.has_error=true; return info;}
    else {this->time.bytes+= sizeof(base_n);}
#if 0
    if (info.current_access != Data::Access::Read ){//TODO
      if (base_n < 1){// Add a new base.
        //TODO Check if the cg_*_write routines are thread safe.
        int base_id=1;
        char cname[this->label_size+1]; ::strcpy (cname,"new_model_1");
        fmr::perf::timer_resume (&this->time);
        err= cg_base_write (info.file_cgid, cname, geom_d, phys_d,& base_id);
        if (!err) {this->time.count += this->label_size;}
        fmr::perf::timer_pause (&this->time);
    } }
#endif
    //TODO_FMR_PRAGMA_OMP(omp critical)// Check use with OpenMP but not MPI.
    for (int base_i=1; base_i <= base_n; base_i++) {
      int zone_n=0, gset_n=0, conn_n=0;
      fmr::Sim_time sim_time = fmr::Sim_time::Unknown;
      char cname[this->label_size+1];
      err= fmr::perf::time_activity<int> (&this->time,
        cg_base_read, info.file_cgid, base_i, cname, &geom_d, &phys_d);
      if (err) {
        log->label_fprintf (log->fmrerr,"ERR""OR Dcgn",
          "could not read base %i in file %s.\n", base_i, fname.c_str());
      }else{
        this->time.bytes += this->label_size + sizeof(geom_d) + sizeof(phys_d);
      }
      const std::string data_id = std::string (cname);
      this->sims_names.insert (std::string (data_id));
      //
      ::SimulationType_t sim_type_cg = ::SimulationTypeNull;
      err= fmr::perf::time_activity<int> (&this->time,
        cg_simulation_type_read, info.file_cgid, base_i, &sim_type_cg);
      if (!err) {this->time.bytes += sizeof(sim_type_cg);
        switch (sim_type_cg) {
          case ::SimulationTypeNull: sim_time= fmr::Sim_time::None    ; break;
          case ::NonTimeAccurate   : sim_time= fmr::Sim_time::Implicit; break;
          case ::TimeAccurate      : sim_time= fmr::Sim_time::Explicit; break;
          case   CG_UserDefined    : sim_time= fmr::Sim_time::Plugin  ; break;
          default                  : {}// Do nothing.
        }
#ifdef FMR_DEBUG
        log->label_fprintf (log->fmrout,
          "CGNS*sim type","%s\n",
          fmr::get_enum_string (fmr::Sim_time_name, sim_time).c_str());
#endif
      }
      err= fmr::perf::time_activity<int> (&this->time,
        cg_nzones, info.file_cgid, base_i,& zone_n);
      if (err) {
        zone_n = 0;
      }else{
        this->time.bytes+= sizeof(zone_n);
      }
      err= fmr::perf::time_activity<int> (&this->time,
        cg_n1to1_global, info.file_cgid, base_i, & conn_n);// n1to1_global
      if (err) {
        zone_n = 0;
      }else{
        this->time.bytes+= sizeof(conn_n);
      }
      int family_n=0;//TODO Include all child family nodes? Check each zone?
      err= fmr::perf::time_activity<int> (&this->time,
        cg_nfamilies, info.file_cgid, base_i, & family_n);
      if (!err){
        gset_n = family_n;
        this->time.bytes+= sizeof(family_n);
      }
      //TODO The following can exist at CGNS base (Sims) level.
      //TODO   err = cg_biter_read(cgid, base_i, char *BaseIterName,
      //             int *Nsteps); // iter_vals
      //TODO   err = cg_gravity_read(int fn, int B, float *GravityVector);
      //
      // Use cg_goto (cgid, base_i,"end") to check for sim-wide info:
      //TODO   err = cg_convergence_read(int *niterations,
      //             char **NormDefinitions);// convergence history
      //TODO   err = cg_nintegrals(int *nintegrals);// integral data
      //TODO   err = cg_nuser_data(int *nuserdata);// user data
      //TODO   err = cg_user_data_read(int Index, char *Name);
      //TODO ier = cg_unitsfull_read(MassUnits_t mass, LengthUnits_t,...);
      //
      // Scan the first part hierarchy level...
      // Element types are in sections.
      // However, synchronization information (halo nodes, rind) is stored at
      // the Part (CGNS Zone) level.
      //NOTE This only needs to be read now for Partition::Join
      //TODO Move to get_info(part_id,..)
      //
      fmr::Local_int mesh_n=0, grid_n=0;
      fmr::Global_int node_n=0, node_sysn=0, elem_sysn=0;
      int sect_0 = 0;
      for(int zone_i=1; zone_i <= zone_n; zone_i++) {
        ::ZoneType_t zone_type;// grid:structured or mesh:unstructured
        err= fmr::perf::time_activity<int> (&this->time,
          cg_zone_type, info.file_cgid, base_i, zone_i,& zone_type);
        if (!err) {this->time.bytes+= sizeof(zone_type);}
#if 1
        char zonename [this->label_size+1];//TODO Cache names now?
        auto zonesize = std::valarray<cgsize_t>(9);
        err= fmr::perf::time_activity<int> (&this->time,
          cg_zone_read, info.file_cgid, base_i, zone_i, zonename,& zonesize[0]);
        if (!err) {this->time.bytes+= this->label_size;}
        switch (zone_type) {
          case ::Structured   : break;//TODO
          case ::Unstructured :
            if (!err) {this->time.bytes+= 3*sizeof(zonesize[0]);}
            node_n     = zonesize[0];
            node_sysn += node_n;// counts halo nodes more than once
            break;
          default : err+=1;
        }
#endif
        //TODO Check for material properties?
        //
        int sect_n=0;// Count grids & meshes in this zone.
        err= fmr::perf::time_activity<int> (&this->time,
          cg_nsections, info.file_cgid, base_i, zone_i,& sect_n);
//        if (err) {return err;}
        switch (zone_type) {
          case ::Structured   : grid_n += sect_n; break;
          case ::Unstructured : mesh_n += sect_n; break;
          default : err+=1;
        }
#if 1
        if (mesh_n>0) {
          this->new_enum_vals (data_id, fmr::Data::Elem_form, mesh_n);
          this->new_local_vals (data_id, fmr::Data::Elem_n, mesh_n);
          this->new_local_vals (data_id, fmr::Data::Node_n, mesh_n);
        }
#endif
        // Scan meshes (CGNS sections) //TODO for Partition::Join only?
        // https://cgns.github.io/CGNS_docs_current/sids/gridflow.html#Elements
        err= cg_goto (info.file_cgid, base_i, "Zone_t",zone_i, NULL);
//        if (err) {return err;}
        const auto part_id = data_id + ":Part_" + std::to_string(zone_i);
        if (!err) {// Register location of >XS sim data here.
          for (auto t : {fmr::Data::Node_coor}) {
//            const auto id = this->make_data_id (part_id, t);
            err= this->add_cgns_here (fname, part_id, t);
#ifdef FMR_DEBUG
              log->label_fprintf(log->fmrerr,"**** Dcgn add!","%s\n",
                this->make_data_id (part_id, t).c_str());
#endif
        } }
        for (int sect_i=1; sect_i<=sect_n; sect_i++) {
          char sectname [this->label_size+1];//TODO Cache names now?
          ElementType_t eltype; cgsize_t start=0, end=0;
          int nbndry=0, parent_flag=0;
          err= fmr::perf::time_activity<int> (&this->time,
            cg_section_read, info.file_cgid, base_i, zone_i, sect_i,
            sectname,& eltype,& start,& end,& nbndry,& parent_flag);
          if (err==0 && elem_form_from_cgns.count (eltype) > 0) {
            const auto form = elem_form_from_cgns.at (eltype);
            // Only scan meshes with recognized element types.
            const auto elem_n = fmr::Local_int (end - start +1);
            const auto mesh_i = sect_0 + sect_i -1;
            const auto fid = this->make_data_id (data_id, fmr::Data::Elem_form);
            const auto eid = this->make_data_id (data_id, fmr::Data::Elem_n);
            const auto nid = this->make_data_id (data_id, fmr::Data::Node_n);
            this->data->enum_vals[fid].data[mesh_i] = fmr::enum2val (form);
            this->data->local_vals[eid].data[mesh_i] = fmr::Local_int (elem_n);
            this->data->local_vals[nid].data[mesh_i] = fmr::Local_int (node_n);
            const auto mesh_id = data_id + ":Mesh_" + std::to_string(mesh_i);
            if (elem_n > 0) {//TODO only adding meshes needed for now.
              err= cg_goto (info.file_cgid, base_i, "Zone_t",zone_i, NULL);
              for (auto t : {fmr::Data::Node_coor}) {
//                const auto id = this->make_data_id (mesh_id, t);
                this->add_cgns_here (fname, mesh_id, t);// Register XS sim data.
                this->add_cgns_here (fname, part_id, t);// bigger sim data.
#ifdef FMR_DEBUG
                log->label_fprintf(log->fmrerr,"**** Dcgn add!","%s\n",
                  this->make_data_id (mesh_id, t).c_str());
#endif
              }
              err= cg_goto (info.file_cgid, base_i,
                "Zone_t",zone_i, "Elements_t",sect_i, NULL);
              // Log location of this here, and in the general Data handler.
              if (err==0) {for (auto t : {fmr::Data::Elem_conn}) {
//                const auto id = this->make_data_id (mesh_id, t);
                this->add_cgns_here (fname, mesh_id, t);
#ifdef FMR_DEBUG
                log->label_fprintf(log->fmrerr,"**** Dcgn add!","%s\n",
                  this->make_data_id (mesh_id, t).c_str());
#endif
              } }
              int user_n=0;
              err = cg_nuser_data (& user_n);
              if (sect_i < user_n) {
                //TODO Move to zone level and loop through all nodes to match
                //     name "fmr::Data::Jacs_dets" using
                //     cg_user_data_read(sect_i, char *Name);
                err= cg_goto (info.file_cgid, base_i,
                  "Zone_t",zone_i, "UserDefinedData_t",sect_i, NULL);
              if (err==0) {for (auto t : {fmr::Data::Jacs_dets}) {
                const auto id = this->make_data_id (mesh_id, t);
                this->add_cgns_here (fname, mesh_id, t);
              } }else{
                const auto lab = "WARN""ING "+this->task_name;
                log->label_fprintf (log->fmrerr, lab.c_str(),
                  "cg_goto (cgid, %i, Zone_t,%i, UserDefinedData_t,%i, NULL)"
                  " in %s returned %i.\n", base_i, zone_i, sect_i,
                  data_id.c_str(), err);
              } }
              if (fmr::elem_form_d [fmr::enum2val (form)] == geom_d) {
                // Count all elements of the same dimension as enclosing space.
                elem_sysn += elem_n;
#ifdef FMR_DEBUG
              const auto str = fmr::get_enum_string (fmr::elem_form_name, form);
              log->label_fprintf (log->fmrerr, "**** Dcgn scan", "%s %lu %s\n",
                data_id.c_str(), elem_sysn, str.c_str());
#endif
        } } } }//end section loop
        if (true) {sect_0 += sect_n;}//TODO Partition::Join only
      }//end zone loop
#ifdef FMR_DEBUG
      log->label_fprintf (log->fmrerr, "*** Dcgn scan",
        "%s %lu elem_sysn, %lu node_sysn (halo duped)\n",
        data_id.c_str(), elem_sysn, node_sysn);
#endif
      // Set cached data vals.
      {//-------------------------------------------------------------------v
        const auto id = this->make_data_id (data_id, fmr::Data::Node_sysn);
        const bool is_found = this->data->global_vals.count(id) > 0;
        if (!is_found) {this->data->global_vals[id]
          = fmr::Global_int_vals (fmr::Data::Node_sysn, 1, node_sysn);
        }else if (this->data->global_vals[id].data.size() < 1) {
          this->data->global_vals[id].data.resize (1, node_sysn);
        }else{
          this->data->global_vals[id].data[0]+= node_sysn;
        }
      }//-------------------------------------------------------------------^
      {//-------------------------------------------------------------------v
        const auto id = this->make_data_id (data_id, fmr::Data::Elem_sysn);
        const bool is_found = this->data->global_vals.count(id) > 0;
        if (!is_found) {this->data->global_vals[id]
          = fmr::Global_int_vals (fmr::Data::Elem_sysn, 1, elem_sysn);
        }else if (this->data->global_vals[id].data.size() < 1) {
          this->data->global_vals[id].data.resize (1, elem_sysn);
        }else{
          this->data->global_vals[id].data[0]+= elem_sysn;
        }
      }//-------------------------------------------------------------------^
      const auto geomid = this->data->make_data_id (data_id,
        fmr::Data::Geom_info);
      const auto physid = this->data->make_data_id (data_id,
        fmr::Data::Phys_info);
      for (Data* D : std::vector<Data*> ({this->data})) {//TODO
        const bool is_geomid_found = D->local_vals.count(geomid) > 0;
        if (!is_geomid_found) {
          D->local_vals[geomid] = fmr::Local_int_vals (fmr::Data::Geom_info);
        }
        const bool is_physid_found = D->local_vals.count(physid) > 0;
        if (!is_physid_found) {
          D->local_vals[physid] = fmr::Local_int_vals (fmr::Data::Phys_info);
        }
        auto gvals =& D->local_vals[geomid].data[0];
        auto gisok =& D->local_vals[geomid].isok[0];
        auto yvals =& D->local_vals[physid].data[0];
        auto yisok =& D->local_vals[physid].isok[0];
        const fmr::Local_int gd = fmr::Local_int (geom_d);
        const fmr::Local_int yd = fmr::Local_int (phys_d);
        if (yd > yvals[enum2val(fmr::Phys_info::Phys_d)]) {
          yisok[enum2val(fmr::Phys_info::Phys_d)] = true;
          yvals[enum2val(fmr::Phys_info::Phys_d)] = yd;
        }
        bool do_set_sim_time = false;
        switch (sim_time) {
          case fmr::Sim_time::None     :// Fall through...
          case fmr::Sim_time::Plugin   :// Fall through...
          case fmr::Sim_time::Explicit :// Fall through...
          case fmr::Sim_time::Implicit : do_set_sim_time=true; break;
          default : {}// No action needed.
        }
        if (do_set_sim_time) {
          yvals[enum2val(fmr::Phys_info::Sim_time)]
            = fmr::Local_int(enum2val (sim_time));
          yisok[enum2val(fmr::Phys_info::Sim_time)] = true;
        }
        if (gd > gvals[enum2val(fmr::Geom_info::Geom_d)]) {
          gvals[enum2val(fmr::Geom_info::Geom_d)] = gd;
          gisok[enum2val(fmr::Geom_info::Geom_d)] = true;
        }
        gvals[enum2val(fmr::Geom_info::Gset_n)] += fmr::Local_int(gset_n);
        gisok[enum2val(fmr::Geom_info::Gset_n)] |= gset_n > 0;
        //
        gvals[enum2val(fmr::Geom_info::Part_n)]+= fmr::Local_int(zone_n);
        gisok[enum2val(fmr::Geom_info::Part_n)] |= zone_n > 0;
        //
        gvals[enum2val(fmr::Geom_info::Part_halo_n)] += fmr::Local_int(conn_n);
        gisok[enum2val(fmr::Geom_info::Part_halo_n)] |= conn_n > 0;
        //
        gvals[enum2val(fmr::Geom_info::Mesh_n)] += fmr::Local_int(mesh_n);
        gisok[enum2val(fmr::Geom_info::Mesh_n)] |= conn_n > 0;
        //
        gvals[enum2val(fmr::Geom_info::Grid_n)] += fmr::Local_int(grid_n);
        gisok[enum2val(fmr::Geom_info::Grid_n)] |= conn_n > 0;
    } }
    info.state.was_read = true;
  }
  this->file_info[fname] = info;
  return info;
}
#if 0
        //TODO Check families for materials //TODO do this somewhere else
      if (!err) {// check for mtrl props
        int mtrl_n=0, prop_n=0;
        err= cg_goto (info.file_cgid, base_i, NULL);
        if (!err) {
          err=  fmr::perf::time_activity<int> (&this->time,
            cg_narrays, & prop_n);
          if (!err) {mtrl_n+= (prop_n>0) ? 1 : 0;
            this->time.bytes+= sizeof(prop_n); }
          prop_n=0;
      } }
#endif
#if 0
          // Assume material props are assigned to CGNS families with names?
          //TODO The name of the family identifies the material.
          for (int family_i=1; family_i<=family_n; family_i++){
            int famname_n=0;
            err= fmr::perf::time_activity<int> (&this->time,
              cg_nfamily_names, info.file_cgid, base_i, family_i, &famname_n);
            if (!err){
              this->time.bytes+= sizeof(famname_n);
                if (!err) {// check for mtrl props
                  err= cg_goto (info.file_cgid, base_i, "Family_t", family_i,
                    NULL);
                  if (!err) {
                    err=  fmr::perf::time_activity<int> (&this->time,
                      cg_narrays, & prop_n);
                    //TODO Check name==fmr::Material, instead.
                    if (!err) {
                      mtrl_n+= (prop_n>0) ? 1 : 0;
                      this->time.bytes+= sizeof(prop_n); }
                    prop_n=0;
        } } } }
#endif
#if 0
        //TODO Move to get_info(part_id,..)
        // This is the next part hierarchy level...
        // Element types are in sections.
        // However, synchronization information (halo nodes, rind) is stored at
        // the Part (CGNS Zone) level.
          for(int zone_i=1; zone_i <= zone_n; zone_i++){
            ::ZoneType_t zone_type;// grid:structured or mesh:unstructured
            err= fmr::perf::time_activity<int> (&this->time,
              cg_zone_type, info.file_cgid, base_i, zone_i,& zone_type );
            if(!err){ this->time.bytes+= sizeof(zone_type); }
            auto ix = enum2val(fmr::Geom_info::Grid_n);
            bool has_structure = true;
            switch (zone_type){
              case ::Structured :{
                ix = enum2val(fmr::Geom_info::Grid_n); break;}
              case ::Unstructured :{
                ix = enum2val(fmr::Geom_info::Mesh_n); break;}
              default: {
                has_structure = false;
            } }
            if (has_structure){//TODO descend zone (Part) hierarchy.
            int sect_n;// count grids/meshes
            //NOTE Only those with materials/physics or bocos need to be loaded.
            err= fmr::perf::time_activity<int> (&this->time,
              cg_nsections, info.file_cgid, base_i, zone_i,& sect_n);
            if(!err){
              this->time.bytes+= sizeof(sect_n);
              gvals[ix]+= fmr::Local_int(sect_n);
              //TODO get eltype now?
#if 0
              if (!err) {//TODO check for mtrl props
                err= cg_goto (info.file_cgid, base_i, "Zone_t", zone_i, NULL);
                if (!err) {
                  err=  fmr::perf::time_activity<int> (&this->time,
                    cg_narrays, & prop_n);
                  if (!err) {mtrl_n+= (prop_n>0) ? 1 : 0;
                    this->time.bytes+= sizeof(prop_n); }
                  prop_n=0;
              } }
#endif
#ifdef FMR_DEBUG
              if (log->verbosity >= this->verblevel) {
                std::string sects="";
                err= cg_goto (info.file_cgid, base_i, "Zone_t",zone_i, NULL);
                for (int sect_i=1; sect_i<=sect_n; sect_i++){
                  char sectname [this->label_size+1];//TODO cache names now?
                  ElementType_t eltype; cgsize_t start,end;
                  int nbndry, parent_flag;
                  err= fmr::perf::time_activity<int> (&this->time,
                    cg_section_read, info.file_cgid, base_i, zone_i, sect_i,
                    sectname,& eltype,& start,& end,& nbndry,& parent_flag);
                  if (!err){
                    this->time.bytes+=this->label_size;
                    sects += ", "+std::string (sectname);
                } }
                log->label_printf ("sections ****",
                  "%s\n", sects.c_str());
              }
#endif
        } } }
#endif
int Dcgn::close (){int err=0;
  fmr::perf::timer_resume (&this->time);
  for (auto fi : this->file_info){// Close all open files.
    auto fname = fi.first;
    auto info  = fi.second;
    switch (info.access) {
      case fmr::data::Access::Read   :{}//Fall through.
      case fmr::data::Access::Write  :{}//Fall through.
      case fmr::data::Access::Modify :{this->close (fname); break;}
      default :{}// Do nothing.
  } }
  fmr::perf::timer_pause (&this->time);
  return err;
}
std::deque<std::string> Dcgn:: get_sims_names (){//TODO Remove?
  std::deque<std::string> model_names={};
  for (auto item : this->sims_names) {model_names.push_back (item); }
  return model_names;
}
}//end Femera namespace

#undef FMR_DEBUG
