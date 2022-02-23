#ifndef FEMERA_HAS_TYPE_IPP
#define FEMERA_HAS_TYPE_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif


namespace femera {
  static inline constexpr Task_type task_cast (Task_type t) noexcept {
    return t;
  }
  static inline constexpr Task_type task_cast (Base_type t) noexcept {
    return static_cast<Task_type> (t);
  }
  static inline constexpr Task_type task_cast (Plug_type t) noexcept {
    return static_cast<Task_type> (t);
  }
}// end femera::namespace

#undef FMR_DEBUG
//end FEMERA_HAS_TYPE_IPP
#endif