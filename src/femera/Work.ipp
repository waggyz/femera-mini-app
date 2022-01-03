#ifndef FEMERA_HAS_WORK_IPP
#define FEMERA_HAS_WORK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace femera {
  inline
  Work::Core_t Work::get_core ()
  noexcept {
    return std::make_tuple(this->proc, this->data, this->test);
  }
  inline
  Work_t Work::get_work (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? this->task_list [i] : nullptr;
  }
  inline
  Work_t Work::get_work (const Work::Task_path_t path)
  noexcept {
    Work_t W = nullptr;
    const auto sz = path.size();
    if (sz > 0) {
      if (path[0] < this->get_task_n()) {
        W = this->get_work (path[0]);
        if (sz > 1) {
          for (fmr::Local_int i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path[i] < W->get_task_n()) { W = W->get_work (path[i]); }
              else { W = nullptr; }
    } } } } }
    return W;
  }
  inline
  fmr::Local_int Work::add_task (Work_t W)
  noexcept {
    this->task_list.push_back (W);
    return fmr::Local_int (this->task_list.size () - 1);
  }
  inline
  fmr::Local_int Work::get_task_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size ());
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
  fmr::Exit_int Work::exit_list () noexcept {
    fmr::Exit_int err =0;
    while (! this->task_list.empty ()) {
      auto W = this->task_list.back ();// Exit in reverse order.
      if (W != nullptr) {
        fmr::Exit_int Werr =0;
#ifdef FMR_DEBUG
        printf ("exit task %s\n", W->name.c_str());
#endif
        try { Werr = W->exit (err); }
        catch (std::exception& e) { Werr = 1; }
        err = (Werr == 0) ? err : Werr;
#ifdef FMR_DEBUG
        printf ("exit done %s\n", W->name.c_str());
#endif
      }
      this->task_list.pop_back ();
    }
    return err;
  }
}//end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_IPP
#endif