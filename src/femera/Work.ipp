#ifndef FMR_HAS_WORK_IPP
#define FMR_HAS_WORK_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

template <class T>
femera::Work<T>::Work (Work* W) noexcept
  : proc (W.proc), file (W.file), test (W.test) {}

template <class T>
int femera::Work<T>::exit (int err) { return err; }

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