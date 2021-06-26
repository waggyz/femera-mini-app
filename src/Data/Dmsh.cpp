#include "../Main/Plug.hpp"

#include "gmsh.h"
#include "../../external/gmsh/Common/CreateFile.h" // GetKnownFileFormats (..)

#include <sys/stat.h> // stat
#include <unistd.h> //TODO Remove usleep?

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
      //TODO liblock instead of omp master?
      auto P = this->proc->task.first<Proc> (Plug_type::Pomp);
      if (P) {
        const auto n = P->get_proc_n();
        this-> max_open_n = n;//TODO for 1 sim/omp (XS)?
        //this-> max_open_n = this->data->get_redo_n () / n + 1;//TODO
        gmsh::option::setNumber ("General.NumThreads",Dmsh::Optval(n));
        if (this->proc->log->detail > this->verblevel){
          this->proc->log->label_printf ("Gmsh uses",
            "up to %i OpenMP thread%s each.\n",n,(n==1)?"":"s");
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
  int Dmsh::set_color (const std::string name, const int c[3]) {int err=0;
    try {gmsh::option::setColor(name, c[0], c[1], c[2]);}
    catch (Dmsh::Thrown e) {err=-1;
      this->label_gmsh_err ("WARNING","gmsh::option::setColor (...)", e);
    }
    return err;
  }
  int Dmsh::init_task (int* argc, char** argv) {int err=0;
    fmr::perf::timer_start (& this->time);
    fmr::perf::timer_resume (&this->time);
    const auto log = this->proc->log;
    FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
      int argc2=argc[0];// Copy getopt variables.
      auto opterr2=opterr; auto optopt2=optopt;
      auto optind2=optind; auto optarg2=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv,
        "d::v::t::hi:o:m:n:s:x:")) != -1){
        // x:  requires an argument
        // x:: optional argument (Gnu compiler)
        switch (optchar){
          case 's':{this->proc->opt_add (optchar);
            this->display_string = std::string(optarg); break; }
          case '?':{break; }
          default :{break; }//TODO
      } }
      const bool read_gmsh_config = false;
      // unknown opt error: gmsh::initialize (argc[0], argv, read_gmsh_config);
      //TODO Maybe can fix by handling all non-Gmsh args above?
      gmsh::initialize (0, nullptr, read_gmsh_config);// init w/out args.
      gmsh::option::setNumber ("General.Verbosity", Dmsh::Optval(0));
      if (this->display_string.size() > 0) {
        gmsh::option::setString ("General.Display", this->display_string);
        gmsh::option::setNumber ("General.NoPopup", 1);
#if 0//def FMR_DEBUG
        log->label_fprintf (log->fmrout,"**** Dmsh Xwin",
          "init display %s\n", this->display_string.c_str());
#endif
        gmsh::option::setNumber("General.Trackball", 0);
        gmsh::option::setNumber("General.RotationX", 0);
        gmsh::option::setNumber("General.RotationY", 0);
        gmsh::option::setNumber("General.RotationZ", 0);
        gmsh::option::setNumber("General.Orthographic", 0);
#if 0
        gmsh::option::setNumber("General.Axes", 0);
        gmsh::option::setNumber("General.SmallAxes", 0);
#endif
        auto mw = Dmsh::Optval(0);
        gmsh::option::getNumber("General.MenuWidth", mw);
        gmsh::option::setNumber("General.GraphicsWidth" , 640 + mw);
        gmsh::option::setNumber("General.GraphicsHeight", 480);
        //
        const int white[3] = {255, 255, 255};
        const int black[3] = {0, 0, 0};
        this->set_color("General.Background", white);
        this->set_color("General.Foreground", black);
        this->set_color("General.Text", black);
        //
        if (!gmsh::fltk::isAvailable ()) {
          try {gmsh::fltk::initialize ();}
          catch (Dmsh::Thrown e) {err= -1;
            this->label_gmsh_err ("WARNING","gmsh::fltk::initialize ()", e);
          }
          catch (...) {err = -1;
            log->label_fprintf (log->fmrerr, "WARNING Dmsh",
              "Could not open Xwindows display %s for visualization.\n",
              this->display_string.c_str());
        } }
        this->is_xwin_open = (err==0);
      }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
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
//TODO        FMR_PRAGMA_OMP(omp barrier)
        FMR_PRAGMA_OMP(omp master) {
          gmsh::finalize ();
        } }else{
        gmsh::finalize ();
    } }
    return err;
  }
int Dmsh::label_gmsh_err (std::string label, const std::string from,
  Dmsh::Thrown e){
  label += " " + this->task_name;
  this->proc->log->label_fprintf (this->proc->log->fmrerr, label.c_str(),
    "%s threw \"%s\".\n", from.c_str(), std::string(e).c_str());
  return 0;
}
std::deque<std::string> Dmsh::get_sims_names (){//TODO Remove?
  std::deque<std::string> model_names={};
  for (auto item : this->sims_names) {model_names.push_back (item.first); }
  return model_names;
}
int Dmsh::make_mesh (const std::string model, const fmr::Local_int) {
  int err=0;
  const auto log = this->proc->log;
  const auto warnlabel = "WARN""ING "+this->task_name;
  //TODO handle mesh index # to generate?
  int geom_d=-1,bbox_d=-1;//, elem_d=0;//TODO check max existing elem_d?
  if (!err) { Data::Lock_here lock (this->liblock);
#if 0
    try {gmsh::model::setCurrent (this->sims_names.at(model));}
#else
    try {gmsh::model::setCurrent (model);}
#endif
    catch (Dmsh::Thrown e) {err= -1;
      const auto from = "gmsh::model::setCurrent ("+model+")";
      this->label_gmsh_err ("WARNING",from.c_str(), e);
    }
    catch (std::out_of_range e) {err= -1;
      const auto from = "this->sims_names.at("+model+") in make_mesh (..)";
      this->label_gmsh_err ("WARNING",from.c_str(), e.what());
    }
    catch (...) {err = -1;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "can not find %s.\n", model.c_str());
    }
    geom_d = gmsh::model::getDimension ();//TODO this->get_dim_val(Gcad_d)
    const double un=std::nan("unset");
    double xmin=un, ymin=un, zmin=un, xmax=un, ymax=un, zmax=un;
    gmsh::model::getBoundingBox (-1,-1, xmin,ymin,zmin, xmax,ymax,zmax);
    bbox_d// Get geom_d from model bounding box.//TODO this->get_dim_val(Geom_d)
      = ((xmax-xmin) > 0.0 ? 1:0)
      + ((ymax-ymin) > 0.0 ? 1:0)
      + ((zmax-zmin) > 0.0 ? 1:0);
    //TODO get geom_d, gcad_d, mesh_d from cached vals.
#if 0
    if ((geom_d == 2) && (bbox_d == 3)) {//TODO stl file defines one volume
      gmsh::model::geo::addSurfaceLoop (std::vector<int>({1}));
      gmsh::model::geo::addVolume (std::vector<int>({1}));
      geom_d = gmsh::model::getDimension ();
    }
#endif
    if ((geom_d < 1) || (geom_d > bbox_d)) {err= 1;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "can not make %iD mesh in %iD space for %s.\n",
        geom_d, bbox_d, model.c_str());
  } }
  if ((!err) && (geom_d <= bbox_d)) { Data::Lock_here lock (this->liblock);
    fmr::perf::timer_resume (& this->time);
#if 0
    try {gmsh::model::setCurrent (this->sims_names.at(model));}
#else
    try {gmsh::model::setCurrent (model);}
#endif
    catch (Dmsh::Thrown e) {
      const auto from = "gmsh::model::setCurrent ("+model+")";
      this->label_gmsh_err ("WARNING",from.c_str(), e);
    }
    catch (std::out_of_range e) {
      const auto from = "this->sims_names.at("+model+") in make_mesh (..)";
      this->label_gmsh_err ("WARNING",from.c_str(), e.what());
    }
    try {gmsh::model::mesh::generate (geom_d);}
    catch (int e) {err= e;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "generate %iD mesh of %s returned %i.\n", geom_d, model.c_str(), err);
    }
    catch (Dmsh::Thrown e) {err= 1;
      const auto from
        = "gmsh::model::mesh::generate ("+std::to_string(geom_d)+")";
      this->label_gmsh_err ("WARNING", from.c_str(), e);
    }
    catch (...) {err= 1;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "generate %iD mesh of %s returned %i.\n", geom_d, model.c_str(), err);
    }
    fmr::perf::timer_pause (& this->time);
  }
#ifdef FMR_DEBUG
  log->label_fprintf (log->fmrout, "**** Gmsh",
    "make_mesh %s %iD return %i...\n", model.c_str(), geom_d, err);
#endif
  if (err>0) {return err;}
  return this->scan_model (model);
}

int Dmsh::make_part (const std::string model, const fmr::Local_int,
  const fmr::Local_int part_n) {
  int err=0;
  const auto log = this->proc->log;
  const auto warnlabel = "WARN""ING "+this->task_name;
  //TODO check if meshed
  //TODO handle part index # to generate
  if (!err) { Data::Lock_here lock (this->liblock);
#if 0
    try {gmsh::model::setCurrent (this->sims_names.at(model));}
#else
    try {gmsh::model::setCurrent (model);}
#endif
      catch (Dmsh::Thrown e) {err= 1;
        const auto from = "gmsh::model::setCurrent ("+model+"))";
        this->label_gmsh_err ("WARNING",from.c_str(), e);
      }
    catch (std::out_of_range e) {err= 1;
      const auto from = "this->sims_names.at("+model+") in make_part (..)";
      this->label_gmsh_err ("WARNING",from.c_str(), e.what());
    }
    catch (...) {err= 1;
      log->label_fprintf (log->fmrerr, warnlabel.c_str(),
        "can not find %s.\n", model.c_str());
    }
    auto optval=Dmsh::Optval(0);
    //gmsh::option::getNumber ("Mesh.NbPartitions", optval);//0: unpartitioned
    if (optval > 1) {
      fmr::perf::timer_resume (& this->time);
      try {gmsh::model::mesh::partition (int (part_n));}
      catch (Dmsh::Thrown e) {err= 1;
        const auto from
          = "gmsh::model::mesh::partition (int ("+std::to_string(part_n)+"))";
        this->label_gmsh_err ("WARNING",from.c_str(), e);
      }
      catch (...) {err= 1;
        log->label_fprintf (log->fmrerr, warnlabel.c_str(),
          "partition (%i) %s returned %i.\n",
          int (optval), model.c_str(), err);
        gmsh::option::setNumber ("Mesh.NbPartitions", Dmsh::Optval(0));
      }
      fmr::perf::timer_pause (& this->time);
    } }
#ifdef FMR_DEBUG
  log->label_fprintf (log->fmrout, "**** Gmsh",
    "make_part %s return %i...\n", model.c_str(), err);
#endif
  if (err>0) {return err;}
  return this->scan_model (model);
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
  const auto fname = info.data_file.second;
  {// liblock scope
    Data::Lock_here lock(this->liblock);
    fmr::perf::timer_resume (& this->time);
    //
    try {gmsh::open (fname);}
    catch (Dmsh::Thrown e) {err= 1;
      const auto from = "gmsh::open ("+fname+")";
      this->label_gmsh_err ("WARNING", from.c_str(), e);
    }
    catch (...) {err= 1;
      fmr::perf::timer_pause (& this->time);
      log->printf_err ("WARN""ING Gmsh could not open %s.\n", fname.c_str());
      // Do not return from within lock scope.
    }
    if (err>0) {
      info.access = fmr::data::Access::Error;
      info.state.has_error = true;
      this->file_info[fname] = info;
    }
#if 0
    if (!info.state.has_error) {
      this->open_n++;
      std::string data_id("");
      gmsh::model::getCurrent (data_id);
      this->add_sims_name (data_id);
    }
#endif
    this->open_n += (info.state.has_error) ? 0 : 1;
  }
  if (info.state.has_error) {return info;}
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
  gmsh::option::getNumber ("Mesh.Binary", optval);//TODO this is for write,
  info.encode = (optval > Dmsh::Optval(0))        // not current file encoding
    ? Data::Encode::Binary : Data::Encode::ASCII;
  std::string encstr = (optval > Dmsh::Optval(0)) ? "binary" : "ASCII";
  //
  gmsh::option::getNumber ("Mesh.Format", optval);
  info.format = int(optval);//TODO this is for write, not current file.
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
  int Dmsh::read_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals &vals) {
    int err=0;
    auto log = this->proc->log;
    if (vals.stored_state.was_read) {return 0;}
#ifdef FMR_DEBUG
    const auto label = "**** "+this->task_name+" read";
    log->label_fprintf (log->fmrerr, label.c_str(), "%s...\n", id.c_str());
#endif
    switch (vals.type) {
      case (fmr::Data::Jacs_dets) :
        if (this->elem_gmsh_info.count(id) >0) {
          const auto info = this->elem_gmsh_info.at(id);
          const auto n = info.tags.size();
          if (n>1) {//TODO Not handled yet
            log->label_fprintf (log->fmrerr, "WARN""ING Dmsh",
              "read_local_vals (%s,..) multi-tag parts not supported.\n",
              id.c_str());
            return 1;
          }
          fmr::Dim_int elem_d =0;
          if (fmr::detail::elem_info_gmsh.count (info.type) >0) {
            const auto form
              = fmr::detail::elem_info_gmsh.at (info.type).elem_form;
            elem_d = fmr::elem_form_d [enum2val(form)];
          }
          if (n>0 && elem_d>0) {
            const std::vector<double> intp (elem_d, 0.5);//TODO only tets & tris
            std::vector<double> jacs={}, dets={}, pts={};
            std::size_t task_n = 1;//TODO automatic partitioning by elem #
            { Data::Lock_here lock(this->liblock);
              for (size_t i=0; i<n; i++) {
#if 0
                try{ gmsh::model::setCurrent (this->sims_names.at(info.model));}
#else
                try{ gmsh::model::setCurrent (info.model);}
#endif
                catch (Dmsh::Thrown e) {
                  const auto from = "gmsh::model::setCurrent ("+info.model+")";
                  this->label_gmsh_err ("WARNING", from.c_str(), e);
                }
                catch (std::out_of_range e) {
                  const auto from = "this->sims_names.at("+info.model
                    +") in read_geom_values (..)";
                  this->label_gmsh_err ("WARNING",from.c_str(), e.what());
                }
                gmsh::model::mesh::getJacobians (info.type, intp,
                  jacs, dets, pts, info.tags[i], info.task, task_n);
            } }
            const auto nj = dets.size();
            const auto sz = jacs.size();
            if (nj > 0) {
              const auto ea = sz / nj;// 1,4,9 for 1D, 2D, 3D
              fmr::Local_int badj_n=0; double vol = 0.0;
              if (vals.data.size() < (sz+nj)) {vals.data.resize (sz+nj);}
              for (size_t i=0; i<nj; i++) {// interleave jacs & dets
                const auto ji = (ea+1)*i;
                for (size_t j=0; j<ea; j++) {//TODO memcpy
                  vals.data [ji +j] = fmr::Geom_float (jacs [ea*i +j]);
                }
                const auto det = dets[i];//TODO set Jacs_type?
                vals.data [ji +ea] = fmr::Geom_float (det);
                vol += det;
                badj_n += (fmr::math::inv3 (& vals.data [ji],// Invert jac.
                  vals.data [ji +ea]) == 0) ? 0 : 1;
#ifdef FMR_DEBUG
                using d=double;
                const auto J =& vals.data[ji];
                const std::string label = this->task_name +" jacs invt";
                log->label_fprintf (log->fmrout, label.c_str(),
                  "%lu:r0 %+8.3e %+8.3e %+8.3e\n", i, d(J[0]),d(J[1]),d(J[2]));
                log->label_fprintf (log->fmrout, label.c_str(),
                  "%lu:r1 %+8.3e %+8.3e %+8.3e\n", i, d(J[3]),d(J[4]),d(J[5]));
                log->label_fprintf (log->fmrout, label.c_str(),
                  "%lu:r2 %+8.3e %+8.3e %+8.3e: %+8.3\n",
                  i, d(J[6]),d(J[7]),d(J[8]),d(J[9]));
#endif
              }
#ifdef FMR_DEBUG
              if (this->verblevel <= log->detail) {
                const std::string label = this->task_name +" jacs invt";
                log->label_fprintf (log->fmrout, label.c_str(),
                  "%u jacobians in %s...\n", nj, id.c_str());
              }
#endif
              fmr::math::perf_inv3 (&this->time,
                fmr::Local_int (nj),& vals.data [0]);//TODO Global_int
              vals.stored_state.was_read = true;
              if (badj_n > 0) {
                const auto lab = "WARN""ING "+this->task_name;
                std::string meas ="";
                switch (elem_d) {
                  case 1 : meas ="length"; break;
                  case 2 : meas ="area"; break;
                  case 3 : meas ="volume"; break;
                  default : meas = std::to_string(elem_d)+"D volume"; break;
                }
                log->label_fprintf (log->fmrerr, lab.c_str(),
                  "%u/%lu nonpositive element determinants in %s (%s %g)\n",
                  badj_n, nj, id.c_str(), meas.c_str(), vol);
              }
#ifdef FMR_DEBUG
              const auto lab = "**** "+this->task_name+" read";
              log->label_fprintf (log->fmrerr, lab.c_str(),
                "%lu %s geom vals, %g volume\n", sz+nj, id.c_str(), vol);
#endif
          } } }
        break;
      default : {}//TODO Print warning.
    }
    err= Data::read_geom_vals (id, vals);//TODO Remove this call?
    return err;
  }
  int Dmsh::read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals) {
    int err=0;
    auto log = this->proc->log;
    if (vals.stored_state.was_read) {return 0;}
#ifdef FMR_DEBUG
    const auto label = "**** "+this->task_name+" read";
    log->label_fprintf (log->fmrerr, label.c_str(), "%s...\n", id.c_str());
#endif
    switch (vals.type) {
      case (fmr::Data::Elem_conn) :
        if (this->elem_gmsh_info.count(id) > 0) {
          const auto info = this->elem_gmsh_info.at(id);
          const auto n = info.tags.size();
          if (n>1) {//TODO Not handled yet
            log->label_fprintf (log->fmrerr, "WARN""ING Dmsh",
              "read_local_vals (%s,..) multi-tag parts not supported.\n",
              id.c_str());
            return 1;
          }
          if (n>0) {
            std::vector<std::size_t> elem={}, conn={};
            std::size_t task_n = 1;//TODO for automatic partitioning by elem #
              for (size_t i=0; i<n; i++) {
                Data::Lock_here lock(this->liblock);
#if 0
                try{gmsh::model::setCurrent (this->sims_names.at(info.model));}
#else
                try{gmsh::model::setCurrent (info.model);}
#endif
                catch (Dmsh::Thrown e) {
                  const auto from = "gmsh::model::setCurrent ("+info.model+")";
                  this->label_gmsh_err ("WARNING", from.c_str(), e);
                }
                catch (std::out_of_range e) {
                  const auto from = "this->sims_names.at("+info.model
                    +") in read_local_vals (..)";
                  this->label_gmsh_err ("WARNING",from.c_str(), e.what());
                }
                gmsh::model::mesh::getElementsByType (info.type, elem, conn,
                  info.tags[i], info.task, task_n);
            }
            const auto esz = conn.size();
            if (esz > 0) {
              if (vals.data.size() < esz) {vals.data.resize(esz);}
              for (size_t i=0; i<esz; i++) {
                vals.data[i] = fmr::Local_int(conn[i] - 1);//TODO Check this -1.
              }//TODO XS only: local_id = global_id - 1
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
    //NOTE: Must be called from within a thread locked scope.
    auto info = Dmsh::File_gmsh (Data::Data_file (this, fname));
    info = this->file_info.count (fname) ? this->file_info.at (fname) : info;
  //  info.data_file.second = fname;//TODO redundant?
    if (!info.state.was_read) {
      bool do_open = (this->open_n < this->max_open_n);
      switch (info.access) {// Check if already open.
        case fmr::data::Access::Read   : do_open = false; break;
        case fmr::data::Access::Write  : do_open = false; break;//TODO true?
        case fmr::data::Access::Modify : do_open = false; break;
        default :{}// do nothing
      }
      if (info.state.has_error) {do_open=true;}// Try to reopen.
      std::string data_id("");
      if (do_open) {
        info = this->open (info, fmr::data::Access::Read,
          Data::Concurrency::Independent);
      }
      if (info.state.has_error) {
        this->file_info [fname] = info;
      }else{
        gmsh::model::getCurrent (data_id);// Read Gmsh model name
    }
    if (info.state.has_error) {return info;}
    const auto names = this->add_sims_name (data_id);
    if (names[0] != data_id) {
      try {gmsh::model::remove ();}
      catch (Dmsh::Thrown e) {info.state.has_error= true;
        const auto from = "gmsh::model::remove () current: "+data_id;
        this->label_gmsh_err ("WARNING", from.c_str(), e);
      }
      const int dlevel=0;//TODO should be MPI level
      const fmr::Local_int thrd_n  = this->proc->get_stat()[dlevel].thrd_n;
      const fmr::Local_int thrd_id = this->proc->get_stat()[dlevel].thrd_id;
      fmr::Local_int frame_id = thrd_id;
      for (const auto name : names) {
        if (name != data_id) {
          // Data::Lock_here lock(this->liblock);//TODO when pulled out
          gmsh::model::add (name);// should make current, but doesn't?
          gmsh::model::setCurrent (name);
          gmsh::onelab::setNumber ("frame", {Dmsh::Optval(frame_id+1)});
          gmsh::merge (fname);//TODO open(fname, model_name)
          this->sims_names[name] = name;
          this->data->sims_names[name] = name;//TODO make protected
          frame_id+= thrd_n;
        }
        info.state.has_error |= this->scan_model (name) > 0;
    } } }//end if !read
    fmr::perf::timer_pause (&this->time);
    info.state.was_checked = true;
    this->file_info [fname] = info;
    return info;
    }
  int Dmsh::scan_model (const std::string data_id) {int err=0;
    Data::Lock_here lock(this->liblock);
    auto log = this->proc->log;
#ifdef FMR_DEBUG
    log->label_fprintf (log->fmrerr, "**** Gmsh scan",
      "model %s\n",data_id.c_str());
#endif
    fmr::perf::timer_resume (&this->time);
#if 0
    try {gmsh::model::setCurrent (this->sims_names.at(data_id));}
#else
    try {gmsh::model::setCurrent (data_id);}
#endif
    catch (std::out_of_range e) {err= 1;
      const auto from = "this->sims_names.at("+data_id
        +") in scan_model (..)";
      this->label_gmsh_err ("WARNING",from.c_str(), e.what());
    }
    catch (Dmsh::Thrown e) {err= 1;
      const auto from = "gmsh::model::setCurrent ("+data_id+")";
      this->label_gmsh_err ("WARNING", from.c_str(), e);
    }
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
    auto gd = gmsh::model::getDimension ();// return<0: File has no Gmsh model.
    if (gd<1 || gd>3) {err= 1;}
    if (!err) {
      fmr::Dim_int mesh_d = 0;
      const double un=std::nan("unset");
      double xmin=un, ymin=un, zmin=un, xmax=un, ymax=un, zmax=un;
      gmsh::model::getBoundingBox (-1,-1, xmin,ymin,zmin, xmax,ymax,zmax);
      const uint ugd// this is to avoid spurious conversion from int warning
        = ((xmax-xmin) > 0.0 ? 1:0)
        + ((ymax-ymin) > 0.0 ? 1:0)
        + ((zmax-zmin) > 0.0 ? 1:0);
      const auto geom_d = fmr::Dim_int (ugd);// spatial dim is bounding box dim
      gmsh::option::getNumber ("Mesh.NbNodes", optval);
      const auto node_n = fmr::Global_int (optval);
      const auto nid = this->make_data_id (data_id, fmr::Data::Node_sysn);
      {//---------------------------------------------------------------------v
        is_found = this->data->global_vals.count(nid) > 0;
        if (!is_found) {this->data->global_vals[nid]
          = fmr::Global_int_vals (fmr::Data::Node_sysn, 1, node_n);
        }else if (this->data->global_vals[nid].data.size() < 1) {
          this->data->global_vals[nid].data.resize (1, node_n);
        }else{
          this->data->global_vals[nid].data[0] += node_n;
        }
      }//---------------------------------------------------------------------^
#ifdef FMR_DEBUG
      log->label_fprintf (log->fmrerr,"**** Dmsh scan","%s[0] has %u nodes.\n",
        nid.c_str(), node_n);
#endif
      fmr::Global_int elem_sysn =0;
      std::string fname ="";
      gmsh::option::getString ("General.FileName", fname);
      std::vector<int> mesh_elem_gmsh={};// int dims=-1, tags=-1;
      gmsh::model::mesh::getElementTypes (mesh_elem_gmsh);//, dims, tags);
      const auto mesh_n = fmr::Local_int (mesh_elem_gmsh.size());//TODO XS sims
#ifdef FMR_DEBUG
      std::string s="";for (auto i : mesh_elem_gmsh){s+=" "+std::to_string(i);}
      log->label_fprintf (log->fmrerr,"**** Gmsh scan",
        "scan_model %s (Gmsh types%s)\n", data_id.c_str(), s.c_str());
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
        log->label_fprintf (log->fmrerr,"**** Dmsh","%s size is %u.\n",
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
#if 0
        const auto did = this->make_data_id (data_id, fmr::Data::Elem_d);
        {//------------------------------------------------------------------v
          is_found = this->data->dim_vals.count(did) > 0;
          if (!is_found) {this->data->dim_vals[did]
            = fmr::Dim_int_vals (fmr::Data::Elem_d, mesh_n);
          }else if (this->data->dim_vals[did].data.size() < mesh_n) {
            this->data->dim_vals[did].data.resize (mesh_n);
          }
        }//------------------------------------------------------------------^
        auto edims =& this->data->dim_vals [did].data[0];
#endif
        for (fmr::Local_int mesh_i=0; mesh_i<mesh_n; mesh_i++) {
          const auto  elem_gmsh = mesh_elem_gmsh [mesh_i];
          auto             form = fmr::Elem_form::Unknown;
          fmr::Dim_int   elem_d = 0;
          if (fmr::detail::elem_info_gmsh.count (elem_gmsh) > 0) {
            const auto elinfo = fmr::detail::elem_info_gmsh.at (elem_gmsh);
            form = elinfo.elem_form;
            elem_d = fmr::elem_form_d [fmr::enum2val (form)];
            mesh_d = (elem_d > mesh_d) ? elem_d : mesh_d;
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
//          edims [mesh_i] = elem_d;
          forms [mesh_i] = fmr::enum2val(form);
          //
          const auto mesh_id = data_id +":Mesh_"+std::to_string (mesh_i);
          // Register element data available here.
          for (auto t : {fmr::Data::Elem_conn, fmr::Data::Jacs_dets}) {
            const auto id = this->make_data_id(mesh_id, t);
            this->elem_gmsh_info [id] = Elem_gmsh_info(data_id,{-1},0,elem_gmsh);
            err= this->data->add_data_file (id, this, fname);//TODO handle err
          }
#if 0
          fmr::Local_int conn_n = 0;
          err= this->new_local_vals (mesh_id, fmr::Data::Elem_conn,
            elem_n * conn_n);//TODO Why is mem allocated here for this?
#endif
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
#if 0
      const fmr::Local_int gcad_n = (mesh_n==0) ?  1 : 0;
      const fmr::Dim_int   gcad_d = (mesh_n==0) ? fmr::Dim_int(gd) : 0;
#else
      const fmr::Dim_int gcad_d = (fmr::Dim_int(gd) > mesh_d)
        ? fmr::Dim_int(gd) : mesh_d;
      const fmr::Local_int gcad_n = (gcad_d > 0) ? 1 : 0;
#endif
      //TODO Check gcad_d.
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
      const auto gcad_d_ix = enum2val(fmr::Geom_info::Gcad_d);
      const auto mesh_d_ix = enum2val(fmr::Geom_info::Mesh_d);
      const auto gset_n_ix = enum2val(fmr::Geom_info::Gset_n);
      const auto part_n_ix = enum2val(fmr::Geom_info::Part_n);
      const auto mesh_n_ix = enum2val(fmr::Geom_info::Mesh_n);
      const auto gcad_n_ix = enum2val(fmr::Geom_info::Gcad_n);
      auto vals =& this->data->local_vals[gid].data[0];
      if (geom_d > vals[geom_d_ix]) {vals[geom_d_ix] = geom_d;}
      if (gcad_d > vals[gcad_d_ix]) {vals[gcad_d_ix] = gcad_d;}
      if (mesh_d > vals[mesh_d_ix]) {vals[mesh_d_ix] = mesh_d;}
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
      //TODO move; checking element jacobians...
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
int Dmsh::close (const std::string model) {int err=0;
  const auto log = this->proc->log;
  fmr::perf::timer_pause (&this->time);
  fmr::perf::timer_resume(&this->time);
  if (this->sims_names.count(model)>0) {
    Data::Lock_here lock (this->liblock);
    //TODO Check if data is still in use.
#if 0
    try {gmsh::model::setCurrent (this->sims_names.at(model));}
#else
    try {gmsh::model::setCurrent (model);}
#endif
    catch (std::out_of_range e) {err= 1;
      const auto from = "this->sims_names.at("+model+") in close (..)";
      this->label_gmsh_err ("WARNING",from.c_str(), e.what());
    }
    catch (Dmsh::Thrown e) {err= 1;
      const auto from = "gmsh::model::setCurrent ("
        +this->sims_names.at(model)+") in close (..)";
      this->label_gmsh_err ("WARNING", from.c_str(), e);
    }
    catch (...) {err= 1;
      const auto warnlabel = "WARN""ING "+this->task_name;
      log->label_fprintf (log->fmrerr,
        warnlabel.c_str(), "could not find %s to close it.\n", model.c_str());
    }
    //TODO handle model already closed. Check model list?
    // Or handle in Data::close(model): returns err only if all return err.
    if (err<=0) {
      std::string current_model="";
      gmsh::model::getCurrent (current_model);
      err = (current_model == model) ? err : 1;
    }
#if 1
#if 0
    if (gmsh::fltk::isAvailable () != 0) {
      gmsh::fltk::lock ();//TODO how to use this?
#endif
    if ((err<=0) && this->is_xwin_open) {err= 0;//TODO Move to Post/View.
      const auto start = fmr::perf::get_now_ns();
      if (err==0) {
        try {gmsh::graphics::draw ();}
        catch (Dmsh::Thrown e) {err= -1;
          const auto from = "gmsh::graphics::draw ()";
          this->label_gmsh_err ("WARNING", from, e);
        }
        catch (...) {err= -1;}
      }
      if (err==0) {
        //int v = -1;
        //std::vector<int> vs; gmsh::view::getTags (vs);
        const auto v = gmsh::view::add ("fmr:view:1", 1);//TODO store new view.
        if (v > 0) {
          fmr::Local_int frame = 0;//TODO Store sim number instead of parsing.
          const std::string delim = ".";
          const auto dlen = delim.length ();
          const auto nlen = model.length ();
          const auto pos  = model.find (delim);
          if (nlen > pos+dlen) {
            const std::string tok = model.substr (pos+dlen, model.length()-pos);
            frame = std::stoi (tok);
          }
          fmr::perf::timer_resume (&this->proc->time);
          const auto wall
            = 1e-9 * double (fmr::perf::timer_total_elapsed (this->proc->time));
          //
          const auto sims = (frame-1)/30;
          const auto sims_mn = std::floor (sims / 60);
          const auto sims_sc = std::floor (sims - 60*sims_mn);
          const auto wall_mn = std::floor (wall / 60);
          const auto wall_sc = std::floor (wall - 60*wall_mn);
          //
          auto fstr = std::string("");
          auto wstr = std::string("");
          std::vector<char> buf(31 +1,0); int c=0;
          c = std::snprintf (&buf[0], 31," Sim time %2g:%02g",sims_mn, sims_sc);
          if (c>0) {fstr = std::string (& buf[0]);}
          c = std::snprintf (&buf[0], 31,"Wall time %2g:%02g",wall_mn, wall_sc);
          if (c>0) {wstr = std::string (& buf[0]);}
          //
          gmsh::view::addListDataString (v, {10.0,15.0}, {fstr});
          gmsh::view::addListDataString (v, {10.0,30.0}, {wstr});
        } else {
          log->label_fprintf (log->fmrerr, "WARNING Dmsh",
            "Could not add view %s.\n","fmr:view:1");
        }
        //TODO Only OpenMP master thread does FLTK operations. Pending ops are
        // queued by Gmsh until the master thread makes a model current. Thus,
        // the write and remove () ops on other threads should be deferred until
        // after the master thread can render it.
        // But, this works fine when there is only 1 OpenMP thread / MPI rank.
#if 0
        try {gmsh::fltk::awake ("update");}
        catch (Dmsh::Thrown e) {err= -1;
          const auto from = "gmsh::fltk::awake (\"update\")";
          this->label_gmsh_err ("WARNING", from, e);
        }
        catch (...) {err= -1;}
#else
        FMR_PRAGMA_OMP(omp master) {
          try {gmsh::fltk::update ();}
          catch (Dmsh::Thrown e) {err= -1;
            const auto from = "gmsh::fltk::update ()";
            this->label_gmsh_err ("WARNING", from, e);
          }
          catch (...) {err= -1;}
#endif
        if (err==0) {
          auto fname = model;
          fname += ".png";
          try {gmsh::write (fname);}
          catch (Dmsh::Thrown e) {err= -1;
            const auto from = "gmsh::write ("+fname+")";
            this->label_gmsh_err ("WARNING", from.c_str(), e);
          }
          catch (...) {err= -1;}
      } } }
      if (this->verblevel <= log->timing) {//TODO Remove?
        log->proc_printf ("%i,\"%s\",\"%s\",%lu,%lu\n",
          this->proc->get_proc_id(), model.c_str(), "view",
          start, fmr::perf::get_now_ns());
    } }
    if (err<=0) {err=0;
      try {gmsh::model::remove ();}
      catch (Dmsh::Thrown e) {err= 1;
        const auto from = "gmsh::model::remove () current: "+model;
        this->label_gmsh_err ("WARNING", from.c_str(), e);
      }
      catch (...) {err= 1;}
      if (err<=0) {this->open_n--;}
    } else {err= 0;}
#endif
#if 0
//    gmsh::fltk::unlock ();
  } else {//TODO Below not needed if gmsh::fltk::lock () does not work.
    Data::Lock_here lock (this->liblock);
    try {gmsh::model::setCurrent (model);}
    catch (Dmsh::Thrown e) {err= 1;
      const auto from = "gmsh::model::setCurrent ("+model+")";
      this->label_gmsh_err ("WARNING", from.c_str(), e);
    }
    catch (...) {err= 1;
#ifdef FMR_DEBUG
      const auto warnlabel = "WARN""ING "+this->task_name;
      this->proc->log->label_fprintf (this->proc->log->fmrerr, warnlabel.c_str(),
        "could not find %s to close it.\n", model.c_str());
#endif
    }
    if (err<=0) {
      try {gmsh::model::remove ();}
      catch (Dmsh::Thrown e) {err= 1;
        const auto from = "gmsh::model::remove () current: "+model;
        this->label_gmsh_err ("WARNING", from.c_str(), e);
      }
      catch (...) {err= 1;}
      if (err<=0) {this->open_n--;}
    } else {err= 0;}//TODO
  }
#endif
  }// end if handled by Gmsh
  fmr::perf::timer_pause (&this->time);
  return err;
}
int Dmsh::close () {//TODO Remove? (not thread safe) or close all?
#if 0
  Data::Lock_here lock (this->liblock);
  fmr::perf::timer_resume(&this->time);
  gmsh::model::remove ();//TODO check if data is still in use.
  fmr::perf::timer_pause (&this->time);
  return 0;
#else
  return 1;
#endif
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
