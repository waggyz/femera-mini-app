#ifndef FEMERA_HAS_WORK_IPP
#define FEMERA_HAS_WORK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  Work::Make_work_t Work::ptrs ()
  noexcept {
    return std::make_tuple(this->proc, this->data, this->test);
  }
  inline
  Work_t Work::get_work (const std::size_t i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i] : nullptr;
  }
  inline
  Work_t Work::get_work (const std::vector<std::size_t> path)
  noexcept {
    Work_t W = nullptr;
    const auto sz = path.size();
    if (sz > 0) {
      if (path[0] < this->get_task_n()) {
        W = this->get_work (path[0]);
        if (sz > 1) {
          for (std::size_t i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path[i] < W->get_task_n()) { W = W->get_work (path[i]); }
              else { W = nullptr; }
    } } } } }
    return W;
  }
  inline
  std::size_t Work::add_task (Work_t W)
  noexcept {
    this->task_list.push_back (W);
    return this->task_list.size () - 1;
  }
  inline
  std::size_t Work::get_task_n ()
  noexcept {
    return this->task_list.size ();
  }
  inline
  void Work::init_list (int* argc, char** argv) noexcept {
    for (auto W : this->task_list) {// Init task_list forward.
      if (W != nullptr) {
#ifdef FMR_DEBUG
        printf ("init task %s\n", W->name.c_str());
#endif
        try { W->init (argc, argv); }
        catch (std::exception& e) { }
#ifdef FMR_DEBUG
        printf ("init done %s\n", W->name.c_str());
#endif
  } } }
  inline
  fmr::Exit_int Work::exit_tree () noexcept {
    fmr::Exit_int err =0;
    std::vector<std::size_t> branch ={};
    while (! this->task_list.empty ()) {
      auto W = this->task_list.back ();
      if ( W == nullptr ){ W->task_list.pop_back (); }
      else { //go to the bottom of the hierarchy
        while (! W->task_list.empty ()) {
          branch.push_back (W->get_task_n () - 1);
          W = W->task_list.back ();
        }
        while (! branch.empty ()) {
          W = this->get_work (branch);
#ifdef FMR_DEBUG
          printf ("exit tree %s\n", W->task_list.back()->name.c_str());
#endif
          fmr::Exit_int Werr =0;
          try { Werr = W->task_list.back()->exit (err); }
          catch (std::exception& e) { Werr = 1; }
          err = (Werr == 0) ? err : Werr;
          branch.pop_back ();
          W->task_list.pop_back ();
        }
      }
#ifdef FMR_DEBUG
      printf ("exit tree %s\n", this->task_list.back()->name.c_str());
#endif
    this->task_list.pop_back (); 
    }
#ifdef FMR_DEBUG
  printf ("exit base %s\n", this->name.c_str());
#endif
  return err;
  }
#if 0
  inline
  fmr::Exit_int Work::exit (fmr::Exit_int err) noexcept {
  }
#endif
}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_IPP
#endif