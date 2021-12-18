#ifndef FMR_HAS_WORK_IPP
#define FMR_HAS_WORK_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

//template <class T>
//femera::Work<T>::Work (Work* W) noexcept
//femera::Work::Work (Work* W) noexcept
//  : proc (W.proc), file (W.file), test (W.test) {}
femera::Work::Work (Work* W) noexcept
  : proc (W->proc), file (W->file), test (W->test) {}
#if 0
template <class T>
void femera::Work<T>::init (int* argc, char** argv) {
  //static_cast<T*>(this)->init (argc, argv);
  T::task_init (argc, argv);
}
#endif
//template <class T>
//int femera::Work<T>::exit (int err) {
int femera::Work::exit (int err) { return err;
  //const auto task_err = T::task_exit (err);
  //return err==0 ? task_err : err;
}
namespace femera {
#if 0
  template <class T>
  femera::Work<T>::Work (int* argc, char** argv) noexcept {
    proc = std::make_shared <femera::Proc::Base>(this, argc, argv);
    data = std::make_shared <femera::Data::Base>(this, argc, argv);
    file = std::make_shared <femera::File::Base>(this, argc, argv);
    test = std::make_shared <femera::Test::Base>(this, argc, argv);
  }
#endif
}// end Femera namespace

#undef FMR_DEBUG
//end FMR_HAS_WORK_IPP
#endif