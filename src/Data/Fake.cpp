#include "../Main/Plug.hpp"

#include <unistd.h>   // getopt, optarg

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera {
  Fake::Fake (Proc* P, Data* D) noexcept {this->proc=P; this->data=D;
    this-> work_type = work_cast (Plug_type::Fake);
//    this-> base_type = work_cast (Base_type::Data);
    this-> task_name ="Defaults";
    this-> verblevel = 2;
    this-> file_exts ={};
  }
  int Fake::prep (){
    if (true){//TODO Remove.
      //info.access == fmr::data::Access::Check){
      //TODO Move to chck()?
      const fmr::Local_int geom_d=3, gcad_d=0, mesh_d=3, grid_d=0,
        gset_n=3, part_n=1, mesh_n=1, grid_n=0;
      const fmr::Local_int phys_d=3, mtrl_n=1;
//      const fmr::Local_int boco_n=2;
      std::string data_id = "fmr:sims:default";
      if (this->proc->log->verbosity >= this->verblevel){
        this->proc->log->label_printf ("Data init", "%s%s...\n",
        false ? "Test: ":"" , this->task_name.c_str());//TODO test name?
      }
      const auto gid = this->data->make_data_id (data_id,
        fmr::Tree_type::Sims,{}, fmr::Data::Geom_info);
      const bool is_gid_found = data->local_vals.count(gid) > 0;
      if (!is_gid_found) {
        data->local_vals[gid] = fmr::Local_int_vals (fmr::Data::Geom_info);
      }
      auto vals =& data->local_vals[gid].data[0];
      vals [enum2val (fmr::Geom_info::Geom_d)] = geom_d;
      vals [enum2val (fmr::Geom_info::Gcad_d)] = gcad_d;
      vals [enum2val (fmr::Geom_info::Mesh_d)] = mesh_d;
      vals [enum2val (fmr::Geom_info::Grid_d)] = grid_d;
      vals [enum2val (fmr::Geom_info::Gset_n)] = gset_n;
      vals [enum2val (fmr::Geom_info::Part_n)] = part_n;
      vals [enum2val (fmr::Geom_info::Mesh_n)] = mesh_n;
      vals [enum2val (fmr::Geom_info::Grid_n)] = grid_n;
      //
#if 0
      const auto yid = this->data->make_data_id (data_id,//TODO Remove.
        fmr::Tree_type::Sims,{}, fmr::Data::Phys_info);
      const bool is_yid_found = data->local_vals.count(yid) > 0;
      if (!is_yid_found) {
        data->local_vals[yid] = fmr::Local_int_vals (fmr::Data::Geom_info);
      }
      vals =& data->local_vals[yid].data[0];
      vals [enum2val (fmr::Phys_info::Phys_d)] = phys_d;
      vals [enum2val (fmr::Phys_info::Mtrl_n)] = mtrl_n;
#endif
//      vals[enum2val(fmr::Geom_info::Boco_n)] = boco_n;
    }
    return 0;
  }
  int Fake::exit_task (int err){
    return err;
  }
  std::deque<std::string> Fake::get_sims_names (){//TODO
    std::deque<std::string> names={};
#if 0
    std::deque<std::string> names={std::string("test-data-1")};
#endif
#if 0
    std::deque<std::string> names=this->data->model_names;
#endif
    return names;
  }
  bool Fake::is_this_type (const std::string){
    return false;
  }
  int Fake::close (){
    return 0;
  }
  int Fake::close (const std::string) {
    return 0;
  }
  int Fake::init_task (int* argc, char** argv){int err=0;
    FMR_PRAGMA_OMP(omp master) {//NOTE getopt is NOT thread safe.
      int argc2=argc[0];// Copy getopt variables.
      auto opterr2=opterr; auto optopt2=optopt;
      auto optind2=optind; auto optarg2=optarg;
      opterr = 0; int optchar;
      while ((optchar = getopt (argc[0], argv, "D")) != -1){
      switch (optchar){
        case 'D':{this->proc->opt_add('D');
          this->use_test_data = true;
#if 0
          this->task_name="XS cubes";//TODO -D optional int to select dataset
#endif
          break; }
        default :{ break; }
      } }
      // Restore getopt variables.
      argc[0]=argc2;opterr=opterr2;optopt=optopt2;optind=optind2;optarg=optarg2;
    }//end non-threadsafe section
    err= this->prep();
    return err;
  }
}// end Femera namespace
#undef FMR_DEBUG
