#include "../Main/Plug.hpp"

#include "gmsh.h"
#include "../../external/gmsh/Common/CreateFile.h" // GetKnownFileFormats (..)

#include <sys/stat.h> // stat

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace fmr {namespace detail {//TODO move to Femera namespace?
  static const std::map <int, std::string> format_gmsh_name ={
    { 1,"msh" },{ 2,"unv"     },{10,"auto" },{16,"vtk" },{19,"vrml"   },
    {21,"mail"},{26,"pos stat"},{27,"stl"  },{28,"p3d" },{30,"mesh"   },
    {31,"bdf" },{32,"cgns"    },{33,"med"  },{34,"diff"},{38,"ir3"    },
    {39,"inp" },{40,"ply2"    },{41,"celum"},{42,"su2" },{47,"tochnog"},
    {49,"neu" },{50,"matlab"  }
  };
  static const std::map <int, fmr::Elem_info> elem_info_gmsh ={
    { 0, fmr::Elem_info (fmr::Elem_form::Unknown, fmr::math::Poly::Unknown, 0)},
    { 1, fmr::Elem_info (fmr::Elem_form::Line, fmr::math::Poly::Full, 1)},
      // 2-node line.
    { 2, fmr::Elem_info (fmr::Elem_form::Tris, fmr::math::Poly::Full, 1)},
      //  3-node triangle.
    { 3, fmr::Elem_info (fmr::Elem_form::Quad, fmr::math::Poly::Full, 1)},
      //  4-node quadrangle.
    { 4, fmr::Elem_info (fmr::Elem_form::Tets, fmr::math::Poly::Full, 1)},
      //  4-node tetrahedron.
    { 5, fmr::Elem_info (fmr::Elem_form::Cube, fmr::math::Poly::Full, 1)},
      //  8-node hexahedron.
    { 6, fmr::Elem_info (fmr::Elem_form::Prsm, fmr::math::Poly::Prism, 1)},
      //  6-node prism.
    { 7, fmr::Elem_info (fmr::Elem_form::Prmd, fmr::math::Poly::Pyramid, 1)},
      //  5-node pyramid.
    { 8, fmr::Elem_info (fmr::Elem_form::Line, fmr::math::Poly::Full, 2)},
      //  3-node second order line (2 vertices, 1 edge).
    { 9, fmr::Elem_info (fmr::Elem_form::Tris, fmr::math::Poly::Full, 2)},
      //  6-node second order triangle (3 vertices, 3 edges).
    {10, fmr::Elem_info (fmr::Elem_form::Quad, fmr::math::Poly::Serendipity, 2)},
      //  9-node second order quadrangle (4 vertices, 4 edges, 1 face).
    {11, fmr::Elem_info (fmr::Elem_form::Tets, fmr::math::Poly::Full, 2)},
      //  10-node second order tetrahedron (4 vertices, 6 edges).
    {12, fmr::Elem_info (fmr::Elem_form::Cube, fmr::math::Poly::Full, 2)},
      //  27-node second order hexahedron (8 vertices, 12 edges, 6 faces, 1 volume).
    {13, fmr::Elem_info (fmr::Elem_form::Prsm, fmr::math::Poly::Prism, 2)},
      //  18-node second order prism (6 vertices, 9 edges, 3 quadrangular faces).
    {14, fmr::Elem_info (fmr::Elem_form::Prmd, fmr::math::Poly::Pyramid, 2)},
      //  14-node second order pyramid (5 vertices, 8 edges, 1 quadrangular face).
    {15, fmr::Elem_info (fmr::Elem_form::Node, fmr::math::Poly::Full, 0)},
      //  1-node point.
    {16, fmr::Elem_info (fmr::Elem_form::Quad, fmr::math::Poly::Full, 2)},
      //  8-node second order quadrangle (4 vertices, 4 edges).
    {17, fmr::Elem_info (fmr::Elem_form::Cube, fmr::math::Poly::Serendipity, 2)},
      //  20-node second order hexahedron (8 vertices, 12 edges).
    {18, fmr::Elem_info (fmr::Elem_form::Prsm, fmr::math::Poly::Prism, 2)},
      //  15-node second order prism (6 vertices, 9 edges).
    {19, fmr::Elem_info (fmr::Elem_form::Prmd, fmr::math::Poly::Pyramid, 2)},
      //  13-node second order pyramid (5 vertices, 8 edges).
    {20, fmr::Elem_info (fmr::Elem_form::Tris, fmr::math::Poly::Tripoly, 3)},
      //  9-node third order incomplete triangle (3 vertices, 6 edges)
    {21, fmr::Elem_info (fmr::Elem_form::Tris, fmr::math::Poly::Full, 3)},
      //  10-node third order triangle (3 vertices, 6 edges, 1 face)
    {26, fmr::Elem_info (fmr::Elem_form::Line, fmr::math::Poly::Full, 3)},
      // 4-node third order edge (2 vertices, 2 edge)
    {29, fmr::Elem_info (fmr::Elem_form::Tets, fmr::math::Poly::Tripoly, 3)},
      //  20-node third order tetrahedron (4 vertices, 12 edges, 4 faces)
    {92, fmr::Elem_info (fmr::Elem_form::Cube, fmr::math::Poly::Full, 3)}
      //  64-node third order hexahedron (8 vertices, 24 edges, 24 on faces, 8 in volume)
    //TODO See external/gmsh/Common/GmshDefines.h
  };
  //
} }
namespace Femera {
  Dmsh:: Dmsh (Proc* P,Data* D) noexcept {this->proc=P; this->data=D;
    this-> work_type = work_cast (Plug_type::Dmsh);
//    this-> base_type = work_cast (Base_type::Data);
    this-> task_name ="Gmsh";
    this-> verblevel = 2;
    this-> version   = GMSH_API_VERSION ;
    this-> file_exts ={};// filled in Dmsh::prep ()
    this-> task.add (this);
  }
  int Dmsh::prep () {int err=0;
    Dmsh::Optval optval = Dmsh::Optval(0);
#if 0
    try {gmsh::option:: setNumber("General.AbortOnError",optval); }
    catch(int e){
      //int aoe=0.0; gmsh::option:: getNumber("General.AbortOnError",aoe);
      this->proc->log->
        label_printf("Gmsh","Set General.AbortOnError to 0 threw %i.\n",e);
    }
#endif
#if 0
      optval = 1.0;
      // Mesh only entities that have no existing mesh
      gmsh::option::setNumber ("Mesh.MeshOnlyEmpty",optval);//TODO crashes
#endif
#ifdef FMR_HAS_OMP
    FMR_PRAGMA_OMP(omp master) {
//    FMR_PRAGMA_OMP(omp single nowait) {//TODO omp single nowait ?
      auto P = this->proc->task.first<Proc> (Plug_type::Pomp);
      if (P) {
        Dmsh::Optval n = Dmsh::Optval(P->get_proc_n());
        gmsh::option::setNumber ("General.NumThreads",n);
        if (this->proc->log->detail > this->verblevel){
          this->proc->log->label_printf ("Gmsh uses",
            "up to %g OpenMP threads each.\n",n);
    } } }
#endif
    FMR_PRAGMA_OMP(omp master) {//TODO omp single nowait ?
      const bool only_mesh_formats = false;//TODO only write formats?
      const std::string f = ::GetKnownFileFormats (only_mesh_formats);
      // Dunno why .geo is not in the list.
      fmr::detail::string_split (f+", geo",',', &this->file_exts);
      for (uint i=0; i<this->file_exts.size(); i++){
        std::string str = this->file_exts[i];
        if (str.length()>1){ if (str.substr(0,1) ==" "){
          str = str.substr (1,str.length()-1);
        } }
        this->file_exts[i] = str;
    } }
    optval = Dmsh::Optval(0);
    gmsh::option::getNumber ("Mesh.Binary",optval);
    this->encode = (optval > Dmsh::Optval(0))
      ? Data::Encode::Binary : Data::Encode::ASCII;
    //
    gmsh::option::getNumber ("Mesh.Format",optval);
    this->format = int(optval);
    //
    return err;
  }
  int Dmsh::init_task (int*,char**) {int err=0;// init w/out cli args.
    fmr::perf::timer_start (& this->time);
    const bool read_gmsh_config = false;
    if (this->is_omp_parallel ()) {
      FMR_PRAGMA_OMP(omp master) {
        // unknown opt error:gmsh::initialize (argc[0], argv, read_gmsh_config);
        gmsh::initialize (0, nullptr, read_gmsh_config);// init w/out cli args.
    } }else{
      gmsh::initialize (0, nullptr, read_gmsh_config);// init w/out cli args.
    }
    this->is_init = true;
    fmr::perf::timer_pause (& this->time);
    this->prep ();
    return err;
  }
  int Dmsh::exit_task (int err){
    fmr::perf::timer_pause (& this->time);
    fmr::perf::timer_resume(& this->time);
    if (this->is_init){
      if (this->is_omp_parallel ()){
        FMR_PRAGMA_OMP(omp master) {
          gmsh::finalize ();
        } }else{
        gmsh::finalize ();
    } }
    return err;
  }
std::deque<std::string> Dmsh::get_sims_names (){//TODO Remove?
  std::deque<std::string> model_names={};
  for (auto item : this->sims_names) {model_names.push_back (item); }
  return model_names;
}
int Dmsh::make_mesh (const std::string model, const fmr::Local_int ix) {
  int err=0;
  const auto log = this->proc->log;
  const auto warnlabel = "WARN""ING "+this->task_name;
  fmr::perf::timer_resume (& this->time);
  try {gmsh::model::setCurrent (model);}
  catch (int e) {err = e;
    log->label_fprintf (log->fmrerr, warnlabel.c_str(),
      "can not find %s.\n", model.c_str());
  }
  //TODO handle mesh index # to generate
  int geom_d = 0;
  if (!err) {
  geom_d = gmsh::model::getDimension ();
    if (geom_d < 1 || geom_d > 3) {err= 1;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "can not make %iD mesh for %s:%u.\n", geom_d, model.c_str(), ix);
  } }
  if (!err) {
//  FMR_PRAGMA_OMP(omp critical) {//TODO thread safety?
    try {gmsh::model::mesh::generate (geom_d);}
    catch (int e) {err= e;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "generate %iD mesh of %s:%u returned %i.\n",
        geom_d, model.c_str(), ix, err);
  } }// }
  if (!err) {
    Dmsh::Optval optval=Dmsh::Optval(0);
    gmsh::option::getNumber ("Mesh.NbPartitions", optval);//0: unpartitioned
    if (optval > 1) {//TODO or >0 ?
      try {gmsh::model::mesh::partition (int (optval));}
      catch (int e) {err= e;
        log->label_fprintf (log->fmrerr, warnlabel.c_str(),
          "partition (%i) %s:%u returned %i.\n",
          int (optval), model.c_str(), ix, err);
        gmsh::option::setNumber ("Mesh.NbPartitions", 0.0);
  } } }
  fmr::perf::timer_pause (& this->time);
#ifdef FMR_DEBUG
  log->label_fprintf (log->fmrout, "**** Gmsh",
    "make_mesh %s:%u return %i...\n", model.c_str(), ix, err);
#endif
  if (err>0) {return err;}
  return this->scan_model (model);;
}
bool Dmsh::is_omp_parallel (){ bool is_omp=false;
  Proc* P = this->proc->task.first<Proc> (Base_type::Pomp);
  if (P) {is_omp = P->is_in_parallel ();}
  return is_omp;
}
Data::File_info Dmsh::get_file_info (const std::string fname) {
  //NOTE cannot read any info without opening and reading the whole file.
  auto info = Dmsh::File_gmsh (Data::Data_file (this,fname));
  info = this->file_info.count (fname) ? this->file_info.at(fname) : info;
  info.state.can_read = true;
  this->file_info [fname] = info;
  return info;
}
Dmsh::File_gmsh Dmsh::open (Dmsh::File_gmsh info,
  fmr::data::Access for_access, Data::Concurrency for_cncr) {
  int err=0;//TODO This loads file data. Do not reload later.
  auto log = this->proc->log;
//  FMR_PRAGMA_OMP(omp critical) {//TODO use is_omp_parallel, like below.
  const auto fname = info.data_file.second;
  fmr::perf::timer_resume (& this->time);
  try {gmsh::open (fname);}
  catch (int e) {
    fmr::perf::timer_pause (& this->time);
    info.access = fmr::data::Access::Error;
    info.state.has_error = true;
    log->printf_err ("WARN""ING Gmsh could not open %s\n",
      fname.c_str());
    this->file_info[fname] = info;
    return info;
  }
#ifdef FMR_DEBUG
  log->printf_err ("*** Gmsh opened %s\n", fname.c_str());
#endif
  fmr::perf::timer_pause (& this->time);
  if (this->verblevel <= log->timing) {
    struct ::stat stat_buf;
    err = ::stat (fname.c_str(), &stat_buf);
    if (!err) {this->time.bytes += stat_buf.st_size;}
  }
  Dmsh::Optval optval=Dmsh::Optval(0);
  gmsh::option::getNumber ("Mesh.Binary", optval);
  info.encode = (optval > Dmsh::Optval(0))
    ? Data::Encode::Binary : Data::Encode::ASCII;
  std::string encstr = (optval > Dmsh::Optval(0)) ? "binary" : "ASCII";
  //
  gmsh::option::getNumber ("Mesh.Format", optval);
  info.format = int(optval);
  if (fmr::detail::format_gmsh_name.count (info.format) > 0) {
    info.version = fmr::detail::format_gmsh_name.at (info.format)+" ";
    if (info.format == 1) {// .msh format
      std::string strval="";
      gmsh::option::getString ("Mesh.MshFileVersion", strval);
      info.version += strval+" ";
  } }
  info.version += encstr;
  //
  info.access = for_access;
  info.concurrency = for_cncr;
  info.state.was_read   = true;
  info.state.is_default = false;
  switch (info.access) {
    case fmr::data::Access::Close :
      info.state.can_read  = false;
      info.state.can_write = false;
      break;
    case fmr::data::Access::Read :
      info.state.can_read  = true;
      info.state.can_write = false;
      break;
    case fmr::data::Access::Write :
      info.state.can_read  = false;
      info.state.can_write = true;
      break;
    case fmr::data::Access::Modify :
      info.state.can_read  = true;
      info.state.can_write = true;
      break;
    default :{}//do nothing
  }
  this->file_info[fname] = info;
  return info;
}
int Dmsh::read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals) {
  int err=0;
  auto log = this->proc->log;
#ifdef FMR_DEBUG
  const auto label = this->task_name +" read";
  log->label_fprintf (log->fmrerr, label.c_str(), "%s...\n", id.c_str());
#endif
  switch (vals.type) {
    case (fmr::Data::Elem_conn) :
      if (this->elem_gmsh_info.count(id) > 0) {
        const auto info = this->elem_gmsh_info.at(id);
        const auto n = info.tags.size();
        if (n>1) {//TODO Not handled yet
          log->printf_err (
            "ERROR Dmsh::read_local_vals (..) multiple tag parts not done yet.");
          return 1;
        }
        if (n>0) {
          std::vector<std::size_t> elem={}, conn={};
          std::size_t task_n = 1;//TODO for automatic partitioning by node #
          for (size_t i=0; i<n; i++) {
            gmsh::model::mesh::getElementsByType (info.type, elem, conn,
              info.tags[i], info.task, task_n);
          }
          const auto esz = conn.size();
          if (esz > 0) {
            if (vals.data.size() < esz) {vals.data.resize(esz);}
            for (size_t i=0; i<esz; i++) {
              vals.data[i] = fmr::Local_int(conn[i]);//TODO XS only
            }//TODO global_id == local_id is XS only
            vals.stored_state.was_read = true;
            //TODO cache elem tags?.
        } } }
      break;
    default : {}//TODO Print warning.
  }
  err= Data::read_local_vals (id, vals);//TODO Remove this call?
  return err;
}
Data::File_info Dmsh::scan_file_data (const std::string fname) {
  auto info = Dmsh::File_gmsh (Data::Data_file (this, fname));
  info = this->file_info.count (fname) ? this->file_info.at (fname) : info;
//  info.data_file.second = fname;//TODO redundant?
  if (!info.state.was_read) {
    bool do_open = true;
    switch (info.access) {// Check if already open.
      case fmr::data::Access::Read   :// Fall through valid open modes...
      case fmr::data::Access::Write  :// Fall through...
      case fmr::data::Access::Modify : do_open = false; break;
      default : do_open = true;
    }
    if (info.state.has_error) {do_open=true;}// Try to reopen.
    if (do_open) {
      const auto inp_file_mode = fmr::data::Access::Read;
      info = this->open (info, inp_file_mode, Data::Concurrency::Independent);
    }
    if (info.state.has_error) {
      this->file_info[fname] = info;
      return info;
    }
    // Read Gmsh model name
    std::string data_id(""); gmsh::model::getCurrent (data_id);
    this->sims_names.insert (std::string (data_id));
    info.state.has_error = this->scan_model (data_id) > 0;
  }//end if !read
  fmr::perf::timer_pause (&this->time);
  info.state.was_checked = true;
  this->file_info[fname] = info;
  return info;
  }
  int Dmsh::scan_model (const std::string data_id) {int err=0;
    auto log = this->proc->log;
    fmr::perf::timer_resume (&this->time);
    gmsh::model::setCurrent (data_id);//TODO catch error
    const auto gid = this->make_data_id (data_id,
      fmr::Tree_type::Sims,{}, fmr::Data::Geom_info);
//    std::valarray<bool> item_isok (false,fmr::Data::Geom_info::end);
    Dmsh::Optval optval=Dmsh::Optval(0);
    //
    bool is_found = this->data->local_vals.count(gid) > 0;
    if (!is_found) {
      this->data->local_vals[gid] = fmr::Local_int_vals (fmr::Data::Geom_info);
    }
#if 0
    const auto physid = make_data_id (data_id,
      fmr::Tree_type::Sims,{}, fmr::Data::Phys_info);
    const bool is_found = data->local_vals.count (physid) > 0;
    if (!is_found) {data->local_vals[physid]
      = fmr::Local_int_vals (fmr::Data::Phys_info, 0);
    }
    auto pvals =& data->local_vals[physid].data[0];
#endif
    // this->chck();
    auto gd = gmsh::model::getDimension();// return<0: File has no Gmsh model.
    if (gd<1 || gd>3) {err= 1;}
    if(!err){
      const auto geom_d = fmr::Local_int(gd);
#if 1
      gmsh::option::getNumber ("Mesh.NbNodes", optval);
      const auto node_n = fmr::Global_int(optval);
      const auto nid = this->make_data_id (data_id, fmr::Data::Node_sysn);
      {//---------------------------------------------------------------------v
        is_found = this->data->global_vals.count(nid) > 0;
        if (!is_found) {this->data->global_vals[nid]
          = fmr::Global_int_vals (fmr::Data::Node_sysn, 1, node_n);
        }else if (this->data->global_vals[nid].data.size() < 1) {
          this->data->global_vals[nid].data.resize (1, node_n);
        }else{
          this->data->global_vals[nid].data[0]+= node_n;
        }
      }//---------------------------------------------------------------------^
#ifdef FMR_DEBUG
      log->label_fprintf (log->fmrerr,"*** Dmsh","%s[0] has %u nodes.\n",
        nid.c_str(), node_n);
#endif
#endif
      fmr::Global_int elem_sysn =0;
      std::string fname ="";
      gmsh::option::getString ("General.FileName", fname);
      std::vector<int> mesh_elem_gmsh={};// int dims=-1, tags=-1;
      gmsh::model::mesh::getElementTypes (mesh_elem_gmsh);//, dims, tags);
      const auto mesh_n = fmr::Local_int(mesh_elem_gmsh.size());
#ifdef FMR_DEBUG
      if (mesh_n > 1) {
        log->label_fprintf (log->fmrerr,"Gmsh scan",
          "scan_model %s (Gmsh types %i,%i)\n",
          data_id.c_str(), mesh_elem_gmsh[0], mesh_elem_gmsh[1]);
      }
      if (mesh_n > 2) {
        log->label_fprintf (log->fmrerr,"Gmsh scan",
          "scan_model %s (Gmsh type  %i)\n",
          data_id.c_str(), mesh_elem_gmsh[2]);
      }
#endif
      if (mesh_n > 0) {//TODO XS sims only
        const auto eid = this->make_data_id (data_id, fmr::Data::Elem_n);
        {//TODO Refactor into a function. -----------------------------------v
          is_found = this->data->local_vals.count(eid) > 0;
          if (!is_found) {this->data->local_vals[eid]
            = fmr::Local_int_vals (fmr::Data::Elem_n, mesh_n);
          }else if (this->data->local_vals[eid].data.size() < mesh_n) {
            //TODO Resize if != mesh_n ?
            this->data->local_vals[eid].data.resize (mesh_n);//TODO clears it
          }
        }//------------------------------------------------------------------^
        auto elems =& this->data->local_vals[eid].data[0];
#ifdef FMR_DEBUG
        log->label_fprintf (log->fmrerr,"*** Dmsh","%s size is %u.\n",
          eid.c_str(), this->data->global_vals[eid].data.size());
#endif
        const auto fid = this->make_data_id (data_id, fmr::Data::Elem_form);
        {//------------------------------------------------------------------v
          is_found = this->data->enum_vals.count(fid) > 0;
          if (!is_found) {this->data->enum_vals[fid]
            = fmr::Enum_int_vals (fmr::Data::Elem_form, mesh_n);
          }else if (this->data->enum_vals[fid].data.size() < mesh_n) {
            this->data->enum_vals[fid].data.resize (mesh_n);
          }
        }//------------------------------------------------------------------^
        auto forms =& this->data->enum_vals [fid].data[0];
        for (fmr::Local_int mesh_i=0; mesh_i<mesh_n; mesh_i++) {
          const auto  elem_gmsh = mesh_elem_gmsh [mesh_i];
          auto             form = fmr::Elem_form::Unknown;
          fmr::Local_int conn_n = 0;
          fmr::Dim_int   elem_d = 0;
          if (fmr::detail::elem_info_gmsh.count (elem_gmsh) > 0) {
            const auto elinfo = fmr::detail::elem_info_gmsh.at (elem_gmsh);
            form = elinfo.form;
            elem_d = fmr::elem_form_d [fmr::enum2val (form)];
            conn_n = fmr::math::poly_terms (elem_d, elinfo.poly, elinfo.pord);
          }else{
            log->printf_err("WARNING Gmsh element type %i not supported.\n",
              elem_gmsh);
          }
          fmr::Local_int elem_n = 0;
          switch (form) {
            //TODO What about 0D & 1D element meshes?
            case (fmr::Elem_form::Tris) :
              gmsh::option::getNumber ("Mesh.NbTriangles", optval);
              elem_n = fmr::Local_int(optval);
              break;
            case (fmr::Elem_form::Quad) :
              gmsh::option::getNumber ("Mesh.NbQuadrangles", optval);
              elem_n = fmr::Local_int(optval);
              break;
#if 0
            case (TODO) :// 3-plane inersect
              gmsh::option::getNumber ("Mesh.NbTrihedra", optval);
              elem_n = fmr::Local_int(optval);
              break;
#endif
            case (fmr::Elem_form::Tets) :
              gmsh::option::getNumber ("Mesh.NbTetrahedra", optval);
              elem_n = fmr::Local_int(optval);
              break;
            case (fmr::Elem_form::Prmd) :
              gmsh::option::getNumber ("Mesh.NbPyramids", optval);
              elem_n = fmr::Local_int(optval);
              break;
            case (fmr::Elem_form::Prsm) :
              gmsh::option::getNumber ("Mesh.NbPrisms", optval);
              elem_n = fmr::Local_int(optval);
              break;
            case (fmr::Elem_form::Cube) :
              gmsh::option::getNumber ("Mesh.NbHexahedra", optval);
              elem_n = fmr::Local_int(optval);
              break;
            default : {}//TODO
          }
          elem_sysn += (elem_d == geom_d) ? elem_n : 0;
          //
          elems [mesh_i] = elem_n;
          forms [mesh_i] = fmr::enum2val(form);
          //
          const auto id = data_id +":Mesh_"+std::to_string (mesh_i);
          const auto cid = this->make_data_id(id, fmr::Data::Elem_conn);
          this->elem_gmsh_info [cid] = Elem_gmsh_info ({-1},0,elem_gmsh);
          err= this->data->add_data_file (cid, this, fname);//TODO handle err
          err= this->new_local_vals (id, fmr::Data::Elem_conn, elem_n * conn_n);
        }//end mesh_i loop
      }//end if mesh_n > 0
      if (elem_sysn > 0) {
        const auto id = this->make_data_id (data_id, fmr::Data::Elem_sysn);
        {//-------------------------------------------------------------------
          is_found = this->data->global_vals.count(id) > 0;
          if (!is_found) {this->data->global_vals[id]
            = fmr::Global_int_vals (fmr::Data::Elem_sysn, 1, elem_sysn);
          }else if (this->data->global_vals[id].data.size() < 1) {
            this->data->global_vals[id].data.resize (1, elem_sysn);
          }else{
            this->data->global_vals[id].data[0]+= elem_sysn;
          }
        }//-------------------------------------------------------------------
        //TODO gmsh::model::mesh::getNodesByElementType (..) to get node_n,
        //     later because the function call (may?) copy node data.
      }
      //
      // If no meshes exist yet, handle as an abstract geometry (CAD) file.
      const fmr::Local_int gcad_n = (mesh_n==0) ? 1 : 0;
      //
      gmsh::option::getNumber ("Mesh.NbPartitions", optval);//0: unpartitioned
      auto part_n = fmr::Local_int(optval);
      // Each Gmsh physical group is a Femera geometry set (gset).
      gmsh::vectorpair dim_tag ={};// vector of (dim, tag) integer pairs
      const int tag_d = -1;// <0 returns tags for all geometric dimensions
      gmsh::model::getPhysicalGroups (dim_tag, tag_d);
      // number of geometry sets = # physical tags.
      //
//      auto phys_d_ix = enum2val(fmr::Phys_info::Phys_d);//TODO ResultSet?
//      auto mtrl_n_ix = enum2val(fmr::Geom_info::Mtrl_n);//TODO Remove.
      const auto geom_d_ix = enum2val(fmr::Geom_info::Geom_d);
      const auto gset_n_ix = enum2val(fmr::Geom_info::Gset_n);
      const auto part_n_ix = enum2val(fmr::Geom_info::Part_n);
      const auto mesh_n_ix = enum2val(fmr::Geom_info::Mesh_n);
      const auto gcad_n_ix = enum2val(fmr::Geom_info::Gcad_n);
      auto vals =& this->data->local_vals[gid].data[0];
      if (geom_d > vals[geom_d_ix]) {vals[geom_d_ix] = geom_d;}
      vals[gset_n_ix] = fmr::Local_int (dim_tag.size());//TODO +=
      vals[part_n_ix] = part_n;//TODO +=
      vals[mesh_n_ix] = mesh_n;//TODO +=
      vals[gcad_n_ix] = gcad_n;//TODO +=
      auto isok =& this->data->local_vals[gid].isok[0];
      isok[geom_d_ix] |= vals[geom_d_ix] > 0;
      isok[gset_n_ix] |= dim_tag.size() > 0;
      isok[part_n_ix] |= part_n > 0;
      isok[mesh_n_ix] |= mesh_n > 0;
      isok[gcad_n_ix] |= gcad_n > 0;
#if 0
      auto log = this->proc->log;
      //TODO Remove; just checking element jacobians...
      if (log->verbosity >= this->verblevel) {
        const int dim = 3;
        std::vector<int> elementTypes={};
        gmsh::model::mesh::getElementTypes (elementTypes, dim);
        const auto elem_n = elementTypes.size();//TODO NOT elem_n?
        if (elem_n < 1 ) {
          log->label_printf ("Gmsh elem","ERROR: %u elem\n",elem_n);
        }else{
          //const std::string integrationType ="Gauss1";
          std::vector<double> intp={};// elem coor
          std::vector<double> intw={};
          gmsh::model::mesh::getIntegrationPoints (elementTypes[0],
            "Gauss1",intp,intw);
          if (intw.size() < 1){
            log->label_printf ("Gmsh elem ptwt","ERROR: %u intpts\n",intw.size());
          }else{
            for (uint i=0; i<intw.size(); i++) {
              log->label_printf ("Gmsh elem ptwt","%1.2f,%1.2f,%1.2f: %1.2f\n",
                intp[3*i],intp[3*i+1],intp[3*i+2], intw[i]);
            }
            std::vector<double> jacs={};
            std::vector<double> dets={};
            std::vector<double> points={};// global coor
            gmsh::model::mesh::getJacobians (elementTypes[0],
              intp,jacs,dets,points);
            const auto jacs_n =dets.size();
            if (jacs_n > 0 ) {//(jacs_n == 5 )
              for (uint j=0; j<jacs_n; j++) {
                log->label_fprintf (log->fmrout,"Gmsh elem jacs",
                  "%1.2f,%1.2f,%1.2f|%1.2f,%1.2f,%1.2f|%1.2f,%1.2f,%1.2f:%1.2f\n",
                  jacs[9*j+0],jacs[9*j+1],jacs[9*j+2],
                  jacs[3*j+9],jacs[9*j+4],jacs[9*j+5],
                  jacs[3*j+9],jacs[9*j+7],jacs[9*j+8], dets[j]);
      } } } } }
#endif
#if 0
      // Get material associations//TODO Do this somewhere else?
      // Assume materials are assigned to Gmsh physical tags with names.
      // The name of the tag identifies the material.
      // TODO Allow associating material type with physical ID number.
      bool has_1d_elem_tags = false;
      const size_t n=dim_tag.size();
      for (size_t i=0; i<n; i++){
#if 0
        std::string physname = "";
        gmsh::model::getPhysicalName (dim_tag[i].first, dim_tag[i].second,
          physname);//TODO cache name
        if (physname.size()>0){ vals[mtrl_n_ix] +=1; }
        //TODO index and cache material names now?
#endif
        has_1d_elem_tags = dim_tag[i].first == 1;
      }
      // Include a 1D bar mesh if it is tagged.//TODO Is this ok?
      if (has_1d_elem_tags){ vals[mesh_n_ix]+= 1; }
#endif
    }//end if !err
  fmr::perf::timer_pause (&this->time);
  return err;
  }
int Dmsh::close () {
  fmr::perf::timer_resume(&this->time);
#if 0
  if (this->is_omp_parallel ()){
      FMR_PRAGMA_OMP(omp critical) {
      //TODO still hangs, probably because
      //     incomplete parallel for loop does not call this on all threads.
      gmsh::model::remove();
    }
  }else{
    gmsh::model::remove();
  }
#else
    gmsh::model::remove ();//TODO check if data is still in use.
#endif
  fmr::perf::timer_pause (&this->time);
  return 0;
}
bool Dmsh::is_this_type (std::string fname){int err=0;//, fmt;
  std::string::size_type idx = fname.rfind('.');
  if (idx != std::string::npos){
    std::string file_ext = fname.substr(idx+1);
    for (std::string ext : this->file_exts){
      if (file_ext == ext){
#if 0
        this->proc->log->fprintf(stderr,"Gmsh knows ext: %s\n",fname.c_str());
#endif
        return true;
  } } }
  if(!err){
#if 0
  try {gmsh::open (fname); }//TODO    err= this->open ();// Try to read it
  catch (int e) {err=1;}
#endif
  }
  return err==0;
}
}// end Femera namespace

#undef FMR_DEBUG
