#ifndef FEMERA_HAS_FILE_IPP
#define FEMERA_HAS_FILE_IPP

#undef FMR_DEBUG
#include <cstdio>     // std::printf
#ifdef FMR_DEBUG
#endif

namespace femera {

template <typename T> inline
T* File<T>::derived (File* ptr) {
  return static_cast<T*> (ptr);
}
template <typename T> inline
void File<T>:: init (int*, char**) {
}
template <typename T> inline
fmr::Exit_int File<T>::exit (fmr::Exit_int err) noexcept {
  if (err>0) {return err;}
  err = femera::Work::exit (err);// Exit the task stack (exceptions caught),
  if (err>0) {return err;}// then exit this task.
  try { File::derived(this)->task_exit (); }
  catch (std::exception& e) { err = 2; }
  return err;
}
template <typename T> inline
std::shared_ptr<T> File<T>::get_task (const size_t i) {
  return std::static_pointer_cast<T> (this->get_work (i));
}
template <typename T> inline
std::shared_ptr<T> File<T>::get_task (const std::vector<size_t> tree) {
  return std::static_pointer_cast<T> (this->get_work (tree));
}

}// end femera:: namespace

#undef FMR_DEBUG
//end FEMERA_HAS_FILE_IPP
#endif