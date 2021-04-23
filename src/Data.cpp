#include "base.h"

#include <unistd.h>   // getopt, optarg
#include <sys/stat.h> // stat, S_ISDIR
#include <limits>     // std::numeric_limits<int>::max()

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
Data:: Data () {meter_unit="W";}
Data:: Data (Proc* P) {
  this-> proc=P; this->data=this;
  this-> work_type = work_cast (Base_type::Data);
  this-> task_name ="Data";
  this-> verblevel = 2;
}
int Data::print_sims_file_info (const std::string sim_name) {
  auto log = this->proc->log;
  if (log->detail >= this->verblevel) {
    std::string namelist ="";
    if (this->sims_data_file.count (sim_name)) {
      const auto data_names = this->sims_data_file.at (sim_name);
      const auto n = data_names.size ();
      if (n) {for (size_t i=0; i<n; i++) {
        Data* D; std::string name; std::tie(D,name)=data_names.at(i);
        namelist += name;
        if (i+1 < n) {namelist+=", ";}
      } }
    else {namelist = "No data files found.";}
    log->label_printf ("Sim file",("for "+sim_name+": "+namelist+"\n").c_str());
  } }
  return 0;
}
int Data::prep () {
  if (this->task.get<Data>(0) != this) {this->task.add (this);}
  return 0;
}
int Data::add_inp_file (const std::string name) {
  chk_file_names.push_back (name);
  return 0;
}
int Data::set_out_file (const std::string name) {
  out_file_names.clear ();
  out_file_names.push_back (name);
  return 0;
}
Data* Data::get_task_for_file (const std::string fname) {
  const auto n = this->task.count();
  for (int i=0; i < n; i++) {
    Data* D=this->task.get<Data>(i); if (D) {
      if (D->work_type != this->work_type) {
        if (D->is_this_type (fname)) {
          return D;
  } } } }
  return nullptr;
}
std::vector<Data*> Data::get_tasks_for_file (const std::string fname) {
  std::vector<Data*> v={};
  const auto n = this->task.count();
  for (int i=0; i < n; i++) {
    Data* D=this->task.get<Data>(i); if (D) {
      if (D->work_type != this->work_type) {
        if (D->is_this_type (fname)) {
          v.push_back (D);
  } } } }
  return v;
}
bool Data::is_this_type (const std::string) {
  return false;
}
Data::File_info Data::get_file_info (const std::string fname) {
  auto info = Data::File_info();
  info.data_file.second = fname;
  return info;
}
Data::File_info Data::scan_file_data (const std::string fname) {
  auto info = Data::File_info();
  info.data_file.second = fname;
  return info;
}
#if 1
Data::File_info Data::get_file_info (Data::Data_file data_file) {
  Data::File_info new_info = Data::File_info ();//TODO get one batch.
  Data* D; std::string fname; std::tie (D,fname) = data_file;
  if (!D) {
    new_info.state.has_error = true;
    D = this->get_task_for_file (fname);
    data_file.first = D;
  }
  new_info.data_file = data_file;
  auto log = this->proc->log;
  if (D == nullptr) {// Unknown file format.
    new_info.state.has_error = true;
    log->fprintf (log->fmrerr,"WARN""ING Unrecognized file type: %s\n",
      fname.c_str());
    return new_info;
  }
  new_info.state.has_error = false;
  new_info.access = fmr::data::Access::Check;
  auto info= D->get_file_info (new_info.data_file.second);
  if (info.state.has_error) {
    log->fprintf (log->fmrerr,"WARN""ING %s file may be corrupt: %s\n",
      D->task_name.c_str(), fname.c_str());
    return info;
  }
  return info;
}
#endif
Data::File_info Data::scan_file_data (Data::Data_file df) {
  auto info = this->get_file_info (df);
  if (info.state.has_error) {return info;}
  Data* D; std::string fname; std::tie (D,fname) = info.data_file;
//  if (!D) {D = this->get_task_for_file (fname); }//TODO Remove?
  if (D) {
    const auto log = this->proc->log;
    info = D->scan_file_data (fname);// Call derived class handler.
    if (this->verblevel <= log->detail) {
      const std::string label = this->task_name+" scan file";
      const auto ver = info.version;
      log->label_fprintf (log->fmrout, label.c_str(),"%s %s (%s%s)\n",
        fmr::get_enum_string (fmr::data::Access_name, info.access).c_str(),
        fname.c_str(), D->task_name.c_str(), ver.size()?(" "+ver).c_str():"");
  } }
  return info;
}
int Data::chck () {
  return 0;
}
int Data::make_mesh (const std::string model, const fmr::Local_int ix) {
  int err=0;
  const auto log = this->proc->log;
#ifdef FMR_DEBUG
  log->label_fprintf (log->fmrout,"**** Data","meshing %s...\n",model.c_str());
#endif
  bool did_mesh = false;
  const auto n = this->task.count ();
  if (n>0) {for (int i=0; i<n; i++) {
    Data* D = this->task.get<Data>(i);
    if (D != nullptr && D != this) {
      fmr::perf::timer_resume (&this->time);
      err= D->make_mesh (model, ix);
      fmr::perf::timer_pause (&this->time);
      if (err <= 0) {
        did_mesh = true;
        if (log->timing >= D->verblevel) {
          const auto s = fmr::perf::format_time_units (this->time.last_busy_ns);
          const auto label = this->task_name+" "+D->task_name+" mesh";
          log->label_fprintf (log->fmrout, label.c_str(),
            "sim_%u: %s meshed in %s.\n",
            ix, model.c_str(), s.c_str());
        }
        return err;// early return: no need to try other drivers
  } } } }
  if (!did_mesh) {
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "did not mesh sim_%u: %s.\n", ix,model.c_str());
    return 1;
  }
  return err;
}
int Data::make_part (const std::string model, const fmr::Local_int ix) {
  int err=0;
  const auto log = this->proc->log;
#ifdef FMR_DEBUG
  log->label_fprintf (log->fmrout,"**** Data","partitioning %s:%u...\n",
    model.c_str(),ix);
#endif
  bool did_part = false;
  const auto n = this->task.count ();
  if (n>0) {for (int i=0; i<n; i++) {
    Data* D = this->task.get<Data>(i);
    if (D != nullptr && D != this) {
      fmr::perf::timer_resume (&this->time);
      err= D->make_part (model, ix);
      fmr::perf::timer_pause (&this->time);
      if (err <= 0) {
        did_part = true;
        if (log->timing >= D->verblevel) {
          const auto s = fmr::perf::format_time_units (this->time.last_busy_ns);
          const auto label = this->task_name+" "+D->task_name+" part";
          log->label_fprintf (log->fmrout, label.c_str(),
            "sim_%u: %s partitioned in %s.\n",
            ix, model.c_str(), s.c_str());
        }
        return err;// early return: no need to try other drivers
  } } } }
  if (!did_part) {
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "did not partition sim_%u: %s.\n", ix, model.c_str());
    return 1;
  }
  return err;
}
int Data::close () {int err=0;
  const auto n = this->task.count ();
  for (int i=0; i < n; i++) {
    Data* D=this->task.get<Data>(i); if (D) {
      if (D->work_type != this->work_type) {
        err+= D->close ();
  } } }
  return err;
}
fmr::Data_id Data::make_data_id (const fmr::Data_id base_path,
  const fmr::Tree_type tree_type, const fmr::Tree_path branch,
  const fmr::Data data_type) {
  fmr::Data_id id = base_path;
#if 0
  if (tree_type == fmr::Tree_type::Join_part) {
    id += "/Join";
  }else{
#else
  if (tree_type != fmr::Tree_type::Join_part) {
#endif
    if (tree_type != fmr::Tree_type::None) {
      fmr::Data_id path_sep ="";
      switch (tree_type) {
        case fmr::Tree_type::Mtrl  : {path_sep ="/Mtrl"; break;}
        case fmr::Tree_type::Sims  : {path_sep ="/Sims"; break;}
        case fmr::Tree_type::Gset  : {path_sep ="/Gset"; break;}
        case fmr::Tree_type::Part  : {path_sep ="/Part"; break;}
        case fmr::Tree_type::Mesh  : {path_sep ="/Mesh"; break;}
        case fmr::Tree_type::Grid  : {path_sep ="/Grid"; break;}
        case fmr::Tree_type::Error : {path_sep ="/*ERR""OR*"; break;}
        default : {path_sep="/unknown_tree_type:";}
      }
      const auto n = branch.size();
      if (n>0) {for (size_t i=0; i<n; i++) {
        id += path_sep +":"+ std::to_string (branch[i]);
        path_sep = "";
  } } } }
  if (data_type != fmr::Data::None) {
    id += ":" + fmr::vals_name [fmr::enum2val (data_type)];
  }
  return id;
}
fmr::Data_id Data::make_data_id (const fmr::Data_id path,//above called by this
  const fmr::Data data_type) {
  return Data::make_data_id (path, fmr::Tree_type::None,{}, data_type);
}
#if 0
fmr::Data_id Data::get_id (){fmr::Data_id id = "";//TODO needed?
  return id;
}
#endif
fmr::Dim_int Data::get_hier_max (){
  return data_hier_max;
}
int Data::add_data_file (const fmr::Data_id id, Data* D,
  const std::string fname) {
  if (this->sims_data_file.count (id) > 0) {
    this->sims_data_file[id].push_back (Data_file (D,fname));
  }else{
    this->sims_data_file[id] = std::vector<Data_file>({Data_file(D,fname)});
  }
  return 0;
}
int Data::get_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals& vals) {
  int err= 0;
  const auto vals_id = this->make_data_id (id, vals.type);
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    const std::string valname = fmr::get_enum_string (fmr::vals_name,vals.type);
    log->label_fprintf (log->fmrerr, "**** Data",
      "%u %s geom vals\n",
      vals.data.size(), vals_id.c_str());
#endif
  // Check if data is cached here in a homogeneous data array.
  bool is_found = this->geom_vals.count (vals_id) > 0;
  if (is_found) {// The data might be cached already.
    if (!vals.stored_state.was_read) {// The data has been cached homogeneous.
      vals = this->geom_vals.at (vals_id);
      if (vals.stored_state.was_read) {
        return vals.stored_state.has_error ? 1 : 0;
  } } }
  // The data has not been cached, but maybe we know how to get it...
  is_found = false;// Change to true if this data can be read by a data handler.
  if (this->sims_data_file.count (vals_id) > 0) {
    const auto data_files = this->sims_data_file.at (vals_id);
#ifdef FMR_DEBUG
    log->label_fprintf (log->fmrerr, "**** Data",
      "%u %s geom vals found %lu handler...\n",
      vals.data.size(), vals_id.c_str(), data_files.size());
#endif
    for (auto df : data_files) {auto D = df.first;
      if (D) {
        err= D->read_geom_vals (vals_id, vals);
        is_found = is_found | ((err==0) & vals.stored_state.was_read);
  } } }
//  this->read_geom_vals (vals_id, vals);//TODO Remove?
  if (is_found) {return vals.stored_state.has_error ? 1 : 0;}
  err= 1;
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    const std::string valname = fmr::get_enum_string (fmr::vals_name,vals.type);
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "%u %s:%s geom vals not found.\n",
      vals.data.size(), id.c_str(), valname.c_str());
#endif
  return err;
}
int Data::get_dim_vals (const fmr::Data_id id, fmr::Dim_int_vals& vals) {
  int err= 0;
  const auto vals_id = this->make_data_id (id, vals.type);
  bool is_found = this->dim_vals.count (vals_id) > 0;
  if (is_found) {
    vals = this->dim_vals.at (vals_id);
    return err;
  }
  const auto names = this->get_sims_names();
  fmr::Local_int name_i=0;
  for (auto name : names) {
    fmr::Data_id cache_id = this->make_data_id (name, fmr::Data::Geom_info);
    if (this->local_vals.count (cache_id) > 0) {
      if (name_i < vals.data.size()) {
        fmr::Geom_info key = fmr::Geom_info(0);
        is_found = true;
        switch (vals.type) {
          case fmr::Data::Geom_d : key = fmr::Geom_info::Geom_d; break;
          default : is_found = false;
        }
        if (is_found) {
          const auto cached = this->local_vals.at (cache_id);
          vals.data [name_i] = fmr::Dim_int(cached.data [enum2val (key)]);
    } } }
    name_i++;
  }
  if (!is_found) {
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,vals.type);
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "%u %s:%s dim vals not found.\n",
      vals.data.size(), id.c_str(), namestr.c_str());
#endif
    return 1;
  }
  return err;
}
int Data::get_enum_vals (const fmr::Data_id id, fmr::Enum_int_vals& vals) {
  int err= 0;
  const auto vals_id = this->make_data_id (id, vals.type);
  bool is_found = this->enum_vals.count (vals_id) > 0;
  if (is_found) {
    vals = this->enum_vals.at (vals_id);
    return err;
  }
  const auto names = this->get_sims_names();
  fmr::Local_int name_i=0;
  for (auto name : names) {
    fmr::Data_id cache_id = this->make_data_id (name, fmr::Data::Geom_info);
    if (this->enum_vals.count (cache_id) > 0) {
      if (name_i < vals.data.size()) {
        fmr::Geom_info key = fmr::Geom_info(0);
        is_found = true;
        switch (vals.type) {
//          case fmr::Data::Elem_type : key = fmr::Geom_info::Geom_d; break;
          default : is_found = false;
        }
        if (is_found) {
          const auto cached = this->enum_vals.at (cache_id);
          vals.data [name_i] = cached.data [enum2val (key)];
    } } }
    name_i++;
  }
  if (!is_found) {
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    const std::string name = fmr::get_enum_string (fmr::vals_name, vals.type);
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "%lu %s:%s enum vals not found.\n",
      vals.data.size(), id.c_str(), name.c_str());
#endif
    return 1;
  }
  return err;
}
int Data::read_geom_vals (const fmr::Data_id id, fmr::Geom_float_vals &vals) {
  int err= 0;
  const auto log = this->proc->log;
  if (log->detail >= this->verblevel) {
    const std::string label = this->task_name+" read";
    log->label_fprintf (log->fmrerr, label.c_str(), "%lu %s geom vals.\n",
      vals.data.size(), id.c_str());
  }
  err= vals.stored_state.has_error | !vals.stored_state.was_read ? 1:0;
  return err;
}
int Data::read_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals) {
  int err= 0;
  const auto log = this->proc->log;
  if (log->detail >= this->verblevel) {
    const std::string label = this->task_name+" read";
    log->label_fprintf (log->fmrerr, label.c_str(), "%lu %s local vals.\n",
      vals.data.size(), id.c_str());
  }
  err= vals.stored_state.has_error | !vals.stored_state.was_read ? 1:0;
  return err;
}
int Data::get_local_vals (const fmr::Data_id id, fmr::Local_int_vals &vals) {
  int err= 0;
  const auto vals_id = this->make_data_id (id, vals.type);
  // Check if data is in a homogeneous data array. ----------------------------
  bool is_found = this->local_vals.count (vals_id) > 0;
  if (is_found) {// The data might be cached already.
    if (vals.stored_state.was_read) {// The data has been cached homogeneous.
      vals = this->local_vals.at (vals_id);
      return vals.stored_state.has_error ? 1 : 0;
  } }
  // The data has not been cached, but maybe we know how to get it...
  is_found = false;// Change to true if this data can be read by a data handler.
  if (this->sims_data_file.count (vals_id) > 0) {
    const auto data_files = this->sims_data_file.at (vals_id);
    for (auto df : data_files) {auto D = df.first;
      if (D) {
        err= D->read_local_vals (vals_id, vals);
        is_found = is_found | ((err==0) & vals.stored_state.was_read);
  } } }
  if (is_found) {return vals.stored_state.has_error ? 1 : 0;}
#ifdef FMR_DEBUG
  else{
    log->label_fprintf (log->fmrerr, "**** Data",
      "no file handler found to read %s local vals...\n", vals_id.c_str());
  }
#endif
  // Check if data is cached in a heterogeneous data structure. ---------------
  // TODO refactor as homogeneous data?
  const auto names = this->get_sims_names();
  fmr::Local_int name_i=0;
  for (auto name : names) {//TODO not an efficient search
    fmr::Data_id cache_id = this->make_data_id (name, fmr::Data::Geom_info);
    if (this->local_vals.count (cache_id) > 0) {
      if (name_i < vals.data.size()) {
        fmr::Geom_info key = fmr::Geom_info(0);
        is_found = true;
        switch (vals.type) {
          case fmr::Data::Gset_n : key = fmr::Geom_info::Gset_n; break;
          case fmr::Data::Part_n : key = fmr::Geom_info::Part_n; break;
          case fmr::Data::Mesh_n : key = fmr::Geom_info::Mesh_n; break;
          case fmr::Data::Grid_n : key = fmr::Geom_info::Grid_n; break;
          case fmr::Data::Gcad_n : key = fmr::Geom_info::Gcad_n; break;
          default : is_found = false;
        }
        if (is_found) {
          const auto cached = this->local_vals.at (cache_id);
          vals.data [name_i] = cached.data [enum2val (key)];
    } } }
    name_i++;
  }
  if (!is_found) {
#ifdef FMR_DEBUG
    const auto log = this->proc->log;
    const std::string name = fmr::get_enum_string (fmr::vals_name, vals.type);
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "%u %s:%s local vals not found.\n",
      vals.data.size(), id.c_str(), name.c_str());
#endif
    return 1;
  }
#if 0
  //for (int i=1; 1<this->task.count();
#if 0
  switch (vals.type) {//TODO is switch needed?
    case fmr::Data::Geom_info :{
      if (is_found) {
        vals.data = this->local_vals[id].data; }//TODO copies data?
      break;}
    case fmr::Data::Phys_info :{
      if (is_found) {
        vals.data = this->local_vals[id].data; }//TODO copies data?
      break;}
    default: err= 1;
  }
#else
    if (is_found) {vals.data = this->local_vals[id].data; }//TODO copies?
    else {err= 1;}
#endif
  if (err){
    vals.stored_state.was_checked = true;
    vals.stored_state.has_error   = true;
  }else{
#if 0
    const bool w
      = (info.access == fmr::data::Access::Write
      || info.access == fmr::data::Access::Modify);
    vals.stored_state.can_write   = w;//TODO
#endif
    vals.stored_state.can_read    = true;
    vals.stored_state.was_read    = true;
    vals.stored_state.was_checked = true;
    vals.memory_state.can_write   = true;
    vals.memory_state.can_read    = true;
    vals.memory_state.was_read    = true;
  }
#endif
  return vals.stored_state.has_error ? 1 : 0;;
}
int Data::get_global_vals (const fmr::Data_id id, fmr::Global_int_vals& vals) {
  int err= 0;
  const auto vals_id = this->make_data_id (id, vals.type);
  bool is_found = this->global_vals.count (vals_id) > 0;
  if (is_found) {
    vals = this->global_vals.at (vals_id);
    return err;
  }
  const auto names = this->get_sims_names();
  fmr::Local_int name_i=0;
  for (auto name : names) {
    fmr::Data_id cache_id = this->make_data_id (name, vals.type);
    if (this->global_vals.count (cache_id) > 0) {
      if (name_i < vals.data.size()) {
        is_found = true;
        const auto cached = this->global_vals.at (cache_id);
        vals.data [name_i] = cached.data [0];
    } }
    name_i++;
  }
  if (!is_found) {
#ifdef FMR_DEBUG
    auto log = this->proc->log;
    const std::string namestr = fmr::get_enum_string (fmr::vals_name,vals.type);
    log->label_fprintf (log->fmrerr, "WARN""ING Data",
      "%u %s:%s global vals not found.\n",
      vals.data.size(), id.c_str(), namestr.c_str());
#endif
    return 1;
  }
  return err;
}
int Data::new_enum_vals (const fmr::Data_id data_id,
  const fmr::Data type, const size_t nvals) {int err=0;
  const auto id = this->make_data_id (data_id, type);
  const bool is_found = this->data->enum_vals.count(id) > 0;
  if (!is_found) {this->data->enum_vals[id]
    = fmr::Enum_int_vals (type, nvals);
  }else if (this->data->enum_vals[id].data.size() < nvals) {
    const auto tmp = this->data->enum_vals[id].data;
    this->data->enum_vals[id].data.resize (nvals);// clears data
    const auto n = tmp.size();
    if (n>0) {
      //TODO replace loop below with memcpy
      FMR_PRAGMA_OMP_SIMD
      for (size_t i=0; i<n; i++) {data->enum_vals[id].data[i] = tmp[i];}
  } }
  return err;
}
int Data::new_local_vals (const fmr::Data_id data_id,// cache here in Data.
  const fmr::Data type, const size_t nvals) {int err=0;
  const auto id = this->make_data_id (data_id, type);
  const bool is_found = this->data->local_vals.count(id) > 0;
  if (!is_found) {this->data->local_vals[id]= fmr::Local_int_vals (type, nvals);
  }else{
    if (this->data->local_vals[id].data.size() < nvals) {
    const auto tmp = this->data->local_vals[id].data;
    this->data->local_vals[id].data.resize (nvals);// clears data
    const auto n = tmp.size();
    if (n>0) {
      //TODO replace loop below with memcpy
      FMR_PRAGMA_OMP_SIMD
      for (size_t i=0; i<n; i++) {data->local_vals[id].data[i] = tmp[i];}
  } } }
  return err;
}
#if 0
int Data::new_geom_vals (const fmr::Data_id data_id,// cache here in Data.
  const fmr::Data type, const size_t nvals) {int err=0;
  const auto id = this->make_data_id (data_id, type);
  const bool is_found = this->data->geom_vals.count(id) > 0;
  if (!is_found) {this->data->geom_vals[id]= fmr::Geom_float_vals (type, nvals);
  }else{
    if (this->data->geom_vals[id].data.size() < nvals) {
    const auto tmp = this->data->geom_vals[id].data;
    this->data->geom_vals[id].data.resize (nvals);// clears data
    const auto n = tmp.size();
    if (n>0) {
      //TODO replace loop below with memcpy
      FMR_PRAGMA_OMP_SIMD
      for (size_t i=0; i<n; i++) {data->geom_vals[id].data[i] = tmp[i];}
  } } }
  return err;
}
#endif
#if 0
int Data::get_global_vals (std::string name, fmr::Tree_path part_tree_id,
  std::vector<fmr::Global_int_vals*> V){int err=0;
  const bool w
    = (this->current_access == fmr::data::Access::Write
    || this->current_access == fmr::data::Access::Modify);
  const size_t n = V.size();
  for (size_t i=0; i<n; i++) {
    bool item_err= false;
    switch (V[i]->type) {
      case fmr::Data::Part_size :{ printf("*** Woo64!\n"); break;}
      default: item_err=true; err= 1;
    }
    if (item_err){
      V[i]->stored_state.was_checked = true;
      V[i]->stored_state.has_error   = true;
    }else{
      V[i]->stored_state.can_write   = w;
      V[i]->stored_state.can_read    = true;
      V[i]->stored_state.was_read    = true;
      V[i]->stored_state.was_checked = true;
      V[i]->memory_state.can_write   = true;
      V[i]->memory_state.can_read    = true;
      V[i]->memory_state.was_read    = true;
  } }
  return err;
}
#endif
std::deque<std::string> Data::get_sims_names () {
  std::deque<std::string> model_names={};
  for (auto name : this->sims_names) {model_names.push_back (name);}
  if (this->work_type == work_cast (Base_type::Data)) {
    for (int i =0; i < this->task.count(); i++) {
      auto D = this->task.get<Data>(i);
       if (D) {if (D != this) {
        auto names = D->get_sims_names();
        while (!names.empty()){
          if (this->sims_names.insert (names.back()).second == false) {
            model_names.push_back (names.back());
          }
          names.pop_back();
  } } } } }
  return model_names;
}
fmr::Local_int Data::get_sims_n (){
  return fmr::Local_int(this->sims_names.size());
}
std::deque<std::string> Data::get_inp_file_names (){
  return this->inp_file_names;
}
std::deque<std::string> Data::get_out_file_names (){
  return this->out_file_names;
}
Work_type Data::get_file_type (const std::string fname) {
  for (int i=0; i < this->task.count(); i++) {
    Data* D=this->task.get<Data>(i);
    if (D->is_this_type (fname)) {return D->work_type;}
  }
  return work_cast(Base_type::None);
}
int Data::clear (){
  for (int i=0; i < this->task.count(); i++){
    //TODO check for open model?
    Data* D=this->task.get<Data>(i);
    if (D) {
      D->sims_names  ={};
      D->chk_file_names ={};
      D->inp_file_names ={};
      D->out_file_names ={};
  } }
#ifdef FMR_DEBUG
  this->proc->log->printf ("Data::clear (): %i models\n",this->get_sims_n());
#endif
  return (this->get_sims_n()==0) ? 0 : 1;
}
#if 0
bool Data:: has_data_for (const Work_type){
  return false;
}
#endif
#if 0
Data* Data:: get_task_for (const Work_type type, const std::string model){
  for(int i=0; i<this->task.count(); i++){
    Data* D=this->task.get<Data>(i);
    //if(D->get_model (model)==0{
      if(D->has_data_for (type)){
        return D;
  } }// }
  return nullptr;
}
#endif
std::string Data::print_details (){
  auto log = this->proc->log;
  std::string ret = log-> print_heading ("Data details");
#if 0
  ret += this->print_summary ();
#endif
  if (log->detail > this->verblevel) {
    const int task_n = this->task.count();
    //TODO add the rest to the return string.
    for (int i=0; i<task_n; i++) {
      Data* D = this->task.get<Data>(i);
      if (D->file_exts.size() > 0) {// supported file extensions
        std::string exts("");
        for (uint j=0; j<D->file_exts.size(); j++){
          exts += D->file_exts[j];
          if ((j+1) < D->file_exts.size()) {exts+=" ";}
        }
        std::string label = D->task_name+" exts";
        log->label_printf (label.c_str(),"%s\n",exts.c_str());
    } }
    std::string label = this->task_name+" vals";
    log->label_printf (label.c_str(),
      "%u built-in simulation variables defined\n",
      fmr::enum2val (fmr::Data::end));
    log->label_printf ("Maximum ints",//TODO Move to build details?
      "%iD, %s, %s local, %s global\n",
      std::numeric_limits<fmr::Dim_int>::max(),
      fmr::perf::format_units(
        std::numeric_limits<fmr::Enum_int>::max(),"ID/type").c_str(),
      fmr::perf::format_units(
        std::numeric_limits<fmr::Local_int>::max(),"ID").c_str(),
      fmr::perf::format_units(
        std::numeric_limits<fmr::Global_int>::max(),"ID").c_str()
    );
    log->label_printf ("Precision size",//TODO Move to build details?
      "%i B Geometry, %i B Physics, %i B Solver, %i B Preconditioner\n",
      sizeof (fmr::Geom_float), sizeof (fmr::Phys_float),
      sizeof (fmr::Solv_float), sizeof (fmr::Cond_float)
    );
#if 0
    //TODO turn back on if heterogenous data packing is used.
    log->label_printf ("Packed ints",//TODO Move to build details?
      "%i dims, %i enums, %i local, %i global scalar items each\n",
      8 * sizeof (fmr::Dim_int) -2,
      8 * sizeof (fmr::Enum_int) -2,
      8 * sizeof (fmr::Local_int) -2,
      8 * sizeof (fmr::Global_int) -2);
    log->label_printf ("Packed vals",//TODO Move to build details?
      "%i single, %i double precision scalar items each\n",
      8 * sizeof (float) -2,
      8 * sizeof (double) -2);
#endif
#if 0
    for(int i=0; i<task_n; i++){
      Data* D = this->task.get<Data>(i);
      if(D != this){
        std::string label = D->task_name+" mode";// concurrent access mode
        log->label_printf (label.c_str(),"%s\n",
          fmr::detail::concurrency_name[D->concurrency].c_str() );
  } }
#endif
  }
  if(log->detail >= this->verblevel){
    std::string s("");;
#if 0
    s = (out_file_names.size() == 1) ? "Input file" : "File";
    for (auto file : inp_file_names ){
      log->label_printf (s.c_str(),"%s\n", file.c_str());
    }
    s="";
#endif
    if (this->sims_names.size()==1){
      s = std::string(": ") + std::string(*this->sims_names.begin());
    }
    log->label_fprintf (log->fmrout,"Model data",
      "%lu input%s\n",fmr::Local_int(this->sims_names.size()), s.c_str());
    fmr::Local_int imodel=0;
    for (auto model : this->sims_names){
      s ="Model "+std::to_string(imodel)+" data"; imodel++;
      log->label_fprintf (log->fmrout,
        s.c_str(),"%s\n", model.c_str());
  } }
  return ret ;
}
// Specialize initialization and exit for each task->item
int Data::exit_task (int err){return err;//TODO why not called?
#if 0
  if (this->proc->log->detail >= this->verblevel) {
    this->proc->log->print_heading ("Done");
  }
#endif
}
int Data::init_task (int* argc, char** argv){ int err=0;
#ifdef FMR_DEBUG
  std::printf("*** Data::init_task start...\n");
#endif
  auto log = this->proc->log;
  fmr::perf::timer_resume (&this->time);
//  std::deque<std::string> chk_file_names ={};// all file names
  FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
    int argc2=argc[0];// Copy getopt variables.
    auto opterr2=opterr; auto optopt2=optopt;
    auto optind2=optind; auto optarg2=optarg;
    opterr = 0; int optchar;
#if 0
    bool read_db_stdin = false, write_db_stdout = false;
    log-> fmrout = stdout;
    bool is_read_only=false, has_one_dest=false; std::string one_dest_file("");
#endif
    while ((optchar = getopt (argc[0], argv, ":i:o:m:")) != -1){
      // x:  requires an argument
      // x:: optional argument (Gnu compiler)
      switch (optchar){
        case 'i':{this->proc->opt_add (optchar);
          chk_file_names.push_back (std::string(optarg)); break;}
        case 'o':{this->proc->opt_add (optchar);
          out_file_names.push_back (std::string(optarg)); break;}
        case 'm':{this->proc->opt_add (optchar);
          this->sims_names.insert (std::string(optarg)); break;}
#if 0
        case '-':{ break; }// ignore --
        case '+':{ //read_db_stdin = true; write_db_stdout = true;
          break; }// "-+" same as "-" "+"
        case 'E':{ do_erase    =true; break; }
        case 'R':{ do_restart  =true; break; }
        case '0':{ is_read_only=true; break; }
        case '1':{ has_one_dest=true; break; }
        case ',':{break; }
#endif
        case ':':{break; }
        case '?':{break; }
        default :{break; }
    } }
    // Now parse non-option arguments.
    // args assumed names of database files, directories, stdout, or stdin.
#if 0
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
    if( write_db_stdout ){ log-> fmrout = stderr;
      if(iprint){
      log->printf("NOTE Messages have been redirectd to stderr.\n"); } }
    // Redirect stdout msgs to stderr if write_db_stdout==true.
#endif
    //std::queue<std::string> unk_file_names={};
    //
    for(; optind < argc[0]; optind++){// Parse non-option arguments.
      chk_file_names.push_back (std::string (argv[optind]));// assume input file
    }
    // Restore getopt variables.
    argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
  }//end non-threadsafe section
  err= this->chck_file_names ();//chk_file_names);
  for (auto model : this->sims_names){// Copy model names to the main queue
    fmr::sims::add (model);
  }
  if (log->detail >= this->verblevel){this->print_details (); }
  fmr::perf::timer_pause (&this->time);
  return err;
}
int Data::chck_file_names (){
#if 1
  auto log = this->proc->log;
  if (log->verbosity >= this->verblevel){
    log->label_printf ("Check stat","of %s\n",
      chk_file_names.size() == 1 ? chk_file_names[0].c_str()
      : (std::to_string (chk_file_names.size())+" files...").c_str());
  }
#endif
  return Data::chck_file_names (this->chk_file_names);
}
int Data::chck_file_names (std::deque<std::string> files){int err=0;
  /* Check files in parallel to build a map between file storage and model data,
   * caching data found while traversing file structures.
   */
  const bool do_read_sims_info = true;//(sims_names.size() == 0);//TODO
#if 0
  for (std::string fname : this->out_file_names){// Check output files first.
    if( 0==0 ){//TODO File exists.
      //
      if( 0==0 ){//TODO err=chck (fname)
      }
      else{// File can not be opened.
      }
    }
  }
  //TODO  const auto this_access = (out_file_names.size() == 0)
  //TODO    ? fmr::data::Access::Modify : fmr::data::Access::Read;
#endif
  const fmr::Local_int fname_n = fmr::Local_int(files.size());
  const int dlevel = 0;
  const int thrd_n = this->proc->get_stat()[dlevel].thrd_n;//TODO get by level
  // Input file checking done parallel.
  // Data info synchronization may be needed later for collective data access.
  auto log = this->proc->log;
  if (files.size() > 0) {
    FMR_PRAGMA_OMP(omp parallel)
    for (fmr::Local_int fname_i=this->proc->get_stat()[dlevel].thrd_id;
      fname_i<fname_n; fname_i += thrd_n){// static round-robin distribution
  //    bool file_ok=true;//TODO Set false here, true if ok and sync all_file_names.
      std::string fname = files [fname_i];
#ifdef FMR_DEBUG
      this->proc->log-> fprintf(this->proc->log-> fmrout,"file[%i] %s...\n",
        fname_i,fname.c_str());
#endif
      struct ::stat s;// Check if file exists.
        { Data::Lock_here lock (this->data_lock);
        if (::stat (fname.c_str(), &s) != 0) {// File not found.
          log->fprintf (log->fmrerr,
            "WARN""ING Input file not found: %s\n",
            fname.c_str());
        }
        else if (do_read_sims_info) {
          auto info = get_file_info (Data_file (nullptr,fname));
          if (fname_i==0) {if (log->verbosity >= this->verblevel) {
            std::string label = this->task_name +" info";
            if (log->detail >= this->verblevel) {
              const std::string mode = fmr::get_enum_string (
                fmr::data::Access_name, info.access);
              log->label_fprintf (log->fmrout, label.c_str(),
                "%s %i file%s...\n",
                mode.c_str(), fname_n, (fname_n==1)?"":"s");
          } } }
          Data* D = info.data_file.first;
          if (D != nullptr) {// recognized file format
            if (!info.state.has_error) {
              info = this->scan_file_data (info.data_file);
              if (!info.state.has_error) {
                this->inp_file_names.push_back (fname);
        } } } }
      } }
      this->get_sims_names();
  }//end if files provided
  if (log->verbosity >= this->verblevel){
    log->label_printf ("Input data","in %s\n",
      this->inp_file_names.size() == 1 ? this->inp_file_names[0].c_str()
      : (std::to_string (this->inp_file_names.size())+" files").c_str());
    log->label_printf ("Output data","to %s\n",
      this->out_file_names.size() == 1 ? this->out_file_names[0].c_str()
      : "each input file");
  }
  return err;
}
}// end Femera namespace
int fmr::data::clear (){
  return fmr::detail::main->data->clear ();
}
int fmr::data::clr (){
  return fmr::detail::main->data->clear ();
}
int fmr::data::add_inp_file (const std::string name){
  return fmr::detail::main->data->add_inp_file (name);
}
int fmr::data::set_out_file (const std::string name){
  return fmr::detail::main->data->set_out_file (name);
}
std::deque<std::string> fmr::data::get_sims (){int err=0;
  err= fmr::detail::main->data->chck_file_names ();
#if 1
  if (err) {fmr::detail::main->proc->log->printf_err (
      "W""ARNING One or more files did NOT check out ok.\n");
  }
#endif
#if 0
  if (fmr::detail::main->data->get_sims_names().size () == 0){
    int fsz = int(fmr::detail::main->data->get_inp_file_names().size());
    fmr::detail::main->proc->log->fprintf (fmr::detail::main->proc->log->fmrerr,
      "W""ARNING Found no sims for process %i in %i %s.\n",
      fmr::detail::main->proc->get_proc_id (), fsz,
      (fsz==1) ? "file" : "files");
  }
#else
  if (fmr::detail::main->data->get_sims_names().size () == 0){
    fmr::Local_int fsz = fmr::Local_int(fmr::detail::main->data
      ->get_inp_file_names().size());
    fmr::detail::main->proc->log->printf (
      "W""ARNING Found no sims in %i %s.\n",
      fsz, (fsz==1) ? "file" : "files");
  }
#endif
  return fmr::detail::main->data->get_sims_names ();
}
std::deque<std::string> fmr::data::get_models (){//int err=0;
  return fmr::data::get_sims ();//TODO get all models
}
int fmr::sims::run_file (const std::string fname){
  fmr::data::set_out_file (fname);
  fmr::data::add_inp_file (fname);
  fmr::sims::add (fmr::data::get_sims ());
  fmr::sims::run ();
#if 0
  return fmr::sims::run ();//TODO Implement this.
#else
  return 0;
#endif
}
#if 0
int fmr::data::init (int* argc, char** argv ){
  fmr::detail::main->data=new Femera::Data(fmr::detail::main->proc);
  return fmr::detail::main->data-> init (argc,argv);
}
int fmr::data::exit (int err ){
  return fmr::detail::main->data-> exit (err);
  //delete fmr::detail::main->data; fmr::detail::main->data=nullptr;
}
#endif
#undef FMR_DEBUG
