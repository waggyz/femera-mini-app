#include "base.h"
#include "mini.hpp"

#include <vector>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>       // printf
#endif
//#include <unistd.h>     // usleep //TODO Remove when fmr::sims::iter() works
#if 0
inline int print_poly_info (uint pstart, uint pend) {//TODO Move to Elem.
  const std::vector<fmr::math::Poly> types
    = {fmr::math::Poly::Full,fmr::math::Poly::Bi,fmr::math::Poly::Serendipity};
  for (uint dim=1; dim<=3; dim++) {
    for (auto ptype : types) {
      for (uint pord=pstart; pord<=pend; pord++) {
        const auto L = fmr::math::Poly_name.at(ptype).substr(0,1);
        const auto terms = fmr::math::poly_terms (dim, ptype, pord);
        fmr::flog::label_printf ("poly", "%uD %s%u: %u terms\n",
          dim, L.c_str(), pord, terms);
  } } }
  return 0;
}
#endif

int main (int argc, char** argv){int err=0;
#if 0
  return fmr::exit (err= fmr::init (&argc, argv) ? err : fmr::sims::run ());
#endif
  fmr::perf::Meter main_time;
  fmr::perf::timer_start (& main_time);// Start paused, counting idle time
  //=============================== Functional ================================
  err= fmr::init (& argc,argv);// returns <0 for exit now without error
  if (err) {return err > 0 ? err : 0; }
  fmr::perf::timer_resume (& main_time);
  fmr::flog::label_printf ("Femera init",
    fmr::perf::format_time_units (main_time.last_idle_ns)+" in main\n");
  //
//  print_poly_info (1,3);
  //
  //if (fmr::flog::get_detail () > 0) {fmr::flog::print_heading ("Start"); }
  fmr::perf::timer_pause (& main_time);
  fmr::sims::run ();
  //
//auto test1 = fmr::Local_int_vals (fmr::Data::Geom_info);
//auto test2 = fmr::Local_int_vals (fmr::Data::Node_x);

#if 0
#if 0
  fmr::data::set_concurrency (fmr::Distribute::Main_level);// 1/MPI (default)
  fmr::sims::set_concurrency (fmr::Sims_size::XS);// XS,SM,MD,LG,XL
  fmr::sims::set_concurrency (fmr::Distribute::One_per_core);// XS models
//#else
  fmr::set_concurrency (..);//TODO should just set in one place.
  fmr::sims::set (fmr::Sims::All, fmr::Distribute::One_per_core);
#endif
  fmr::sims::clear ();
  fmr::data::clear ();
  fmr::sims::run_file ("tests/mesh/cube-tet6p2n1.cgns");
  //
  fmr::sims::clear ();
  fmr::data::clear ();
  fmr::data::add_inp_file ("tests/mesh/cube-tet6p1n1.cgns");
  fmr::data::add_inp_file ("tests/mesh/cube-tet6p1n2.cgns");
  fmr::data::set_out_file (FMR_BUILD_DIR"/mini-test-1.cgn");
  //
#if 1
  fmr::sims::add ("cube-tet6p1n1");// in cube-tet6p1n1.msh2
#else
  fmr::sims::add (fmr::data::get_sims ());
  //fmr::sims::add (fmr::data::Test_data::Sim_1);// Sym3_impl_elas_iso3
#endif
  fmr::sims::run ();
#if 0
#endif



  //
  fmr::load::set (fmr::Sims::All, fmr::load::Step::Constant);
  //
  fmr::load::add (fmr::Sims::All,
    fmr::load::dirichlet (fmr::geom::Point::Origin, fmr::dofs::all (0.0));
  fmr::load::add (fmr::Sims::All,
    fmr::load::dirichlet (fmr::geom::Surface::Origin_yz_plane,
      fmr::dofs::displace_x (0.0));
  fmr::load::add (fmr::Sims::All,
    fmr::load::dirichlet (fmr::geom::plane (1.0,0.0,0.0, 1.0),
      fmr::dofs::displace_x (1e-3));
  //
#if 0
  fmr::mtrl::set (fmr::Sims::All,// all parts
    fmr::mtrl::elastic_isotropic (fmr::mtrl::youngs (100e9),
      fmr::mtrl::poissons (0.3)));
#else
  fmr::mtrl::set (fmr::sims::get (1),
    fmr::mtrl::elastic_isotropic_ref (fmr::Mtrl::Steel_AISI_1020);
  fmr::mtrl::set (fmr::sims::get (2),
    fmr::mtrl::elastic_cubic_ref (fmr::mtrl::youngs (100e9),
      fmr::mtrl::poissons (0.3), fmr::mtrl::mu (50e9),
        fmr::mtrl::bunge_deg (0.0,0.0,0.0));
#endif
  //
  fmr::cond::set (fmr::Sims::All, fmr::Cond::Jacobi);
  fmr::solv::set (fmr::Sims::All, fmr::Solv::Implicit);
  //
  fmr::post::add (fmr::Sims::All, fmr::Post::L2_solv_err);// L2 norm of solution error
  fmr::post::add (fmr::Sims::All, fmr::Post::Save_all);
  //
  fmr::sync::add (fmr::Sims::All, fmr::post::stats (fmr::Post::L2_solv_err));
  fmr::sync::add (fmr::Sims::All, fmr::Post::Perf);
  // fmr::post::stats(..) : mean, variance
  //
  fmr::sims::run ();
    //
    fmr::sims::dist ribute();
    fmr::sims::init ();
    fmr::sims::iter ate(); //solve(); //run_all_sims();// Loop:
      fmr::sims::dist ribute(i);// load balancing
      fmr::sims::init (i);
      fmr::sims::iter ate(i); //solve(i)
        [dist(j); init(j); iter(j); [...] sync(j); post(j); exit(j);]
      fmr::sims::sync (i);
      fmr::sims::post _process(i);
      fmr::sims::exit (i);
    //end loop
    fmr::sims::sync ()
    fmr::sims::post _process();
    fmr::sims::exit ();

  if (fmr::flog::get_detail () > 0) {fmr::flog::print_heading ("D_one"); }
#endif
  // Keep state for use after fmr::exit (err).
  const bool this_is_master = fmr::proc::is_master     ();
  const int  this_verbosity = fmr::flog::get_verbosity ();
  const int  this_timing    = fmr::flog::get_timing    ();
  const std::string fexit   = fmr::flog::format_label_line ("Femera exit",
    "%s in main");
  const std::string fdone   = fmr::flog::format_label_line ("Femera done",
    "%s in main");
  //
  fmr::perf::timer_pause (& main_time);
  if( fmr::flog::get_detail () > 1 ){fmr::flog::print_heading ("Done"); }
  fmr::flog::label_printf ("Femera time",
    fmr::perf::format_time_units (timer_total_elapsed (main_time))+" in main\n");
  //
  err= fmr::exit (err);// Exit cleanly (0) if processes exit correctly.
  //
  if (this_is_master && (this_timing > 0) && (this_verbosity > 0)){
    fmr::perf::timer_resume (& main_time);
    const std::string timestr1 = fmr::perf::format_time_units (
      main_time.last_idle_ns);
    printf (fexit.c_str(), timestr1.c_str());
    fmr::perf::timer_stop (& main_time);
    const std::string timestr2 = fmr::perf::format_time_units (
      main_time.click - main_time.start);
    printf (fdone.c_str(), timestr2.c_str());
  }
  return (err>0) ? err : 0;
}
#undef FMR_DEBUG





#if 0
  //TODO use functional C++ API for the rest...
  auto proc = fmr::detail::main->proc;
#if 0
  std::queue<std::string> sims_queue ={};//TODO need MPI comm for a queue
  //TODO Implement sims batch queue later, when load balancing is needed.
  for(int i=0;i<6;i++){ sims_queue.push("sim_"+std::to_string(i+1)); }
  const int sims_n = sims_queue.size();// Total number of sims
#endif
  const int sims_n = 6;// Total number of sims
  int sims_cpu_n = 2;// cpus/sim
  //
  // small sim default.
  int sims_lvl_i = 0;// Independent simulation concurrency procesing level
  int sims_par_n = 1;// Number of concurrent sims (sims batch size)
  int sims_snc_i = 0;// Simulation synchronization level
  //
  if( sims_cpu_n == 1){// small sims
    // 1 cpu/sim, max sims concurrency, no sync needed
    // 1 mpi independent data thread/sim
    sims_lvl_i = 1; sims_snc_i = 0;
    sims_par_n = proc-> lvl_thrd_n (0);// total threads in this job
  }
#if 0
  else if( sims_cpu_n == proc->get_proc_n() ){// huge sims
    // all cpu/sim, 1 sims spread over all nodes
    // 4-8 mpi concurrent data threads/node, sync openmp and mpi
    sims_lvl_i = 0; sims_par_n = 1; sims_snc_i = 2;
  }
#endif
  else if( sims_cpu_n > proc-> get_proc_n ()){// large sims
    // many cpu/sim, 1 sims spread over some or all nodes
    // 4-8 mpi concurrent data threads/node, sync openmp and mpi
    sims_lvl_i = 0; sims_par_n = 1; sims_snc_i = 2;
  }
  else{// medium sims
    // 1 mpi/sims, 1 mpi independent data thread/sim, sync openmp only
    sims_lvl_i = 2; sims_snc_i = 1;
    sims_par_n = proc->get_task( sims_lvl_i )-> get_proc_n ();
  }
  const int sims_bat_n = fmr::math:: divide_ceil( sims_n, sims_par_n );
  //...
  //
  if(proc->log-> detail > 2){ proc->log->printf("\n"); }
  FMR_PRAGMA_OMP (omp parallel)
  {// start OpenMP parallel region
  for( proc-> redo_i = 0; proc-> redo_i < proc-> redo_n; proc-> redo_i++){
    const auto stat = proc-> get_stat ();
    const int proc_lvl_n = proc->task.count ();
    std::vector<std::string> proc_lvl_name (proc_lvl_n,"");
    for(int i=0; i<proc_lvl_n; i++){
      proc_lvl_name [i] = proc-> get_task (i)-> task_name;
    }
#if FMR_VERBMAX>2
    if( proc->log-> detail > 2 && proc-> redo_n > 1 ){
      std::string s = "%"
        + std:: to_string (fmr::math::count_digits(uint(proc-> redo_n )))
        +"i / %i ...\n";
      proc->log-> label_printf ("Start repeat", s.c_str(),
        proc-> redo_i+1, proc-> redo_n );
    }
#endif
    for(int sims_bat_i=0; sims_bat_i < sims_bat_n; sims_bat_i++){
      for(int sims_par_i=0; sims_par_i < sims_par_n; sims_par_i++){
        const int sims_i = sims_bat_i * sims_par_n + sims_par_i;
        if( sims_i < sims_n ){
          if( sims_i % sims_par_n == stat[sims_lvl_i].thrd_id ){
#if FMR_VERBMAX>2
            if( proc->log-> detail > 2 ){
              //if( stat[sims_lvl_i-1].thrd_id == 0){
              if( stat[ proc_lvl_n > 1 ? sims_lvl_i-1 : 0 ].thrd_id == 0){
                auto i1 = std:: to_string(fmr::math::count_digits(uint( sims_bat_n )));
                auto i2 = std:: to_string(fmr::math::count_digits(uint( sims_n )));
                auto i3 = std:: to_string(fmr::math::count_digits(
                  uint( stat[sims_lvl_i].thrd_n )));
                std::string s
                  = "batch %" + i1 + "i/%i"
                  + ", sim %" + i2 + "i/%i"
                  + " on %s %"+ i3 + "i\n";
                proc->log-> label_fprintf (proc->log-> fmrout,
                  "Sims init", s.c_str(),
                  sims_bat_i+1,sims_bat_n, sims_i+1,sims_n,
                  proc_lvl_name [sims_lvl_i].c_str(), stat[sims_lvl_i].thrd_id);
            } }
            FMR_PRAGMA_OMP(omp barrier)
            FMR_PRAGMA_OMP(omp master)
            if( proc->log-> timing > 2 ){
              fmr::perf::timer_resume (& main_time );
              //fmr::perf::timer_pause  (& main_time );
            }
#endif
            //
            // Sims init...
            //
            //}
            FMR_PRAGMA_OMP(omp master)
            if( proc->log-> detail > 2 ){
              if( sims_snc_i > 0 ){
                if( false ){
                proc->log-> label_fprintf (proc->log-> fmrout,
                  "Sims sync", "%s\n",
                  proc_lvl_name [sims_snc_i].c_str());
                }
            } }
            const int part_n = 2;
            const int part_lvl_i = 1;// Partition concurrency level
            const int part_par_n // Number of concurrent parts (part batch sz)
              = proc->get_task(part_lvl_i)-> get_proc_n ();
            //TODO Replace dummy data above.
            const int part_bat_n = fmr::math:: divide_ceil( part_n, part_par_n );
            //...
            //TODO Load step loop?
            //TODO Newton step loop?
            //
            for(int part_bat_i=0; part_bat_i < part_bat_n; part_bat_i++){
              //
              for(int part_par_i=0; part_par_i < part_par_n; part_par_i++){
                const int part_i = part_bat_i * part_par_n + part_par_i;
                if( part_i < part_n ){
                  if( part_i % part_n == stat [ part_lvl_i ].thrd_id){
#if FMR_VERBMAX>6
                    if( proc->log-> detail > 6 ){
                      auto i1 = std:: to_string (
                        fmr::math::count_digits(uint( part_n )));
                      auto i2 = std:: to_string (
                        fmr::math::count_digits(uint( sims_n )));
                      auto i3 = std:: to_string (
                        fmr::math::count_digits(uint( stat[sims_lvl_i].thrd_n )));
                      auto i4 = std:: to_string (
                        fmr::math::count_digits(uint( stat[part_lvl_i].thrd_n )));
                      std::string s
                        = "part  %"+ i1 + "i/%i"
                        + ", sim %"+ i2 + "i/%i"
                        + " on %s %"+ i3 + "i"
                        + ", %s %"+ i4 + "i\n";
                      proc->log-> label_fprintf (proc->log-> fmrout, "Part init",
                        s.c_str(),
                        part_i+1,part_n, sims_i+1,sims_n,
                        proc_lvl_name[sims_lvl_i].c_str(),
                        stat[sims_lvl_i].thrd_id,
                        proc_lvl_name[part_lvl_i].c_str(),
                        stat[part_lvl_i].thrd_id );
                     }
#endif
                    const int rand_factor = RAND_MAX / 16384;
                    usleep( rand() / rand_factor );
                    //
                    // Part init...
                    //
                } }//end part init
              }//end concurrent part loop
            }//end part batch loop
            const int iter_n=5 ;
            for(int iter_i=0; iter_i<iter_n; iter_i++){//TODO while resid<toler..
              for(int part_bat_i=0; part_bat_i < part_bat_n; part_bat_i++){
                //
#if 0
                FMR_PRAGMA_OMP(omp master)
                if( proc->log-> timing > 2 ){
                  fmr::perf::timer_resume (& main_time );
                }
#endif
                for(int part_par_i=0; part_par_i < part_par_n; part_par_i++){
                  const int part_i = part_bat_i * part_par_n + part_par_i;
                  if( part_i < part_n ){
                    if( part_i % part_n == stat[part_lvl_i].thrd_id){
#if FMR_VERBMAX>7
                      if( proc->log-> detail > 7 ){
                        proc->log-> label_fprintf (proc->log-> fmrout,"Part iter",
                          "part  %i/%i, sim %i   on %s %i, %s %i\n",
                          part_i+1,part_n, sims_i+1,
                          proc_lvl_name[sims_lvl_i].c_str(),
                          stat[sims_lvl_i].thrd_id,
                          proc_lvl_name[part_lvl_i].c_str(),
                          stat[part_lvl_i].thrd_id );
                      }
#endif
                      const int rand_factor = RAND_MAX / 16384;
                      usleep( rand() / rand_factor );
                      //
                      // Iter...
                      //
                  } }
                }//end concurrent part loop
#if 0
                FMR_PRAGMA_OMP(omp master)
                if( proc->log-> timing > 2 ){
                  fmr::perf::timer_pause (& main_time, part_bat_n );
                }
#endif
            }//end part batch loop
            FMR_PRAGMA_OMP(omp barrier)
          }//end iteration loop
        } }//end concurrent sim loop
        FMR_PRAGMA_OMP(omp master)
        if( proc->log-> timing > 2 ){
          fmr::perf::timer_pause (& main_time );
        }
        //
    } }// end sims batch loop
    //
  FMR_PRAGMA_OMP(omp master)
  fmr::perf::timer_pause (& main_time, sims_n );
  }// end repeating all tasks loop
  }// end OMP parallel region.


  if( fmr::flog:: get_timing ()> 2 ){
    fmr::proc:: wait_for_team ();
    if( fmr::flog:: get_verbosity ()> 1 ){
      if( fmr::flog:: get_detail ()> 1 ){
        fmr::flog:: print_heading ("D_one");
      }
      fmr::flog:: print_label_units_meter ("Sims","sim",main_time );
  } }
  if (fmr::flog::get_detail () > 0) {fmr::flog::print_heading ("Exit"); }
#endif


/*
Notices:
Copyright 2018 United States Government as represented by the Administrator of
the National Aeronautics and Space Administration. No copyright is claimed in
the United States under Title 17, U.S. Code. All Other Rights Reserved.

Disclaimers
No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR
FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE
SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF
PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE
UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY
PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY
LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE,
INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE
OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED
STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR
RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH
MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
*/
