#ifndef FEMERA_HAS_WORK_IPP
#define FEMERA_HAS_WORK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

template <typename T, typename C> inline
T* femera::cast_via_work (C* child) {
  return static_cast<T*> (static_cast<Work*> (child));
}
namespace femera {
  inline
  Work::Work (const Work::Core_ptrs_t core) noexcept {
    std::tie (this->proc, this->data, this->test) = core;
  }
  inline
  std::string Work::get_version ()
  noexcept {
    return this->version;
  }
  inline
  Work::Core_ptrs_t Work::get_core ()
  noexcept {
    return std::make_tuple (this->proc, this->data, this->test);
  }
#if 0
  inline
  Work_spt Work::get_work_spt (const fmr::Local_int i)
  noexcept {
    return (i < this->task_list.size()) ? std::move (task_list [i]) : nullptr;
  }
  inline
  Work_spt Work::get_work_spt (const Work::Task_path_t path)
  noexcept {
    Work_spt W = nullptr;
    auto Wraw = this;
    const auto sz = path.size();
#if 0
    if (sz > 0) {
      if (path [0] < this->get_task_n ()) {
        W = this->get_work_spt (path [0]);
        if (sz > 1) {
          for (fmr::Local_int i = 1; i < sz; i++) {
            if (W != nullptr) {
              if (path [i] < W->get_task_n()) {W = W->get_work_spt (path [i]);}
              else { W = nullptr; }
    } } } } }
#else
    if (sz > 0) {
      for (fmr::Local_int i = 0; i < sz; i++) {
        if (Wraw != nullptr) {
          W = W->get_work_spt (path [i]);
          Wraw = W.get();
    } } }
#endif
    return W;
  }
#endif
  inline
  Work* Work::get_work_raw (const fmr::Local_int ix)
  noexcept {
    return (ix < this->task_list.size()) ? this->task_list [ix].get() : nullptr;
  }
  inline
  Work* Work::get_work_raw (const Work::Task_path_t& path)
  noexcept {
    auto W = this;
    const auto sz = path.size ();
    if (sz > 0) {
      for (fmr::Local_int i = 0; i < sz; i++) {
        if (W != nullptr) {
          W = W->get_work_raw (path [i]);
    } } }
    return W;
  }
  inline
  Work* Work::get_work_raw (const Task_type t, const fmr::Local_int ix)
  noexcept {
    fmr::Local_int i=0;
    auto W = this;
#if 1
    if (W->task_type == t) {
      //TODO Is this the desired behavior of nested drivers of the same type?
      //     Task 0 is the parent, with 1-indexed children of the same type.
      if (i == ix) { return W; }
      i++;
    }
#endif
    while (! W->task_list.empty ()) {
      const fmr::Local_int n = W->get_task_n ();
      for (fmr::Local_int Wix=0; Wix < n; Wix++) {
        if (W->task_list [Wix].get()->task_type == t) {
          if (i == ix) { return W->task_list [Wix].get(); }
          i++;
      } }
      W = W->task_list [0].get();//TODO other branches
    }
    return nullptr;
  }
  inline
  fmr::Local_int Work::get_task_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size ());
  }
  inline
  fmr::Local_int Work::add_task (Work_spt W)
  noexcept {
    this->task_list.push_back (std::move(W));
    return fmr::Local_int (this->task_list.size () - 1);// index of task added
  }
  inline
  fmr::Local_int Work::del_task (const fmr::Local_int ix)
  noexcept {
    if (ix < this->task_list.size ()) {
      this->task_list.erase (this->task_list.begin() + ix);
    }
    return fmr::Local_int (this->task_list.size ());
  }
  inline
  std::string Work::get_name ()
  noexcept {
    return this->name;
  }
  inline
  std::string Work::get_abrv ()
  noexcept {
    return this->abrv;
  }
  inline
  std::string Work::set_name (const std::string& nm)
  noexcept {
    this->name = nm;
    return this->name;
  }
  inline
  std::string Work::set_abrv (const std::string& ab)
  noexcept {
    this->abrv = ab;
    return this->abrv;
  }
}//end femera:: namespace
#if 0
namespace fmr {
template<typename T, typename... Args> inline
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}//end fmr:: namespace
#endif
#if 0
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
namespace std {                                             // from C++14
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3656.htm
  template<class T> struct _Unique_if {
  typedef unique_ptr<T> _Single_object;
  };
  template<class T> struct _Unique_if<T[]> {
  typedef unique_ptr<T[]> _Unknown_bound;
  };
  template<class T, size_t N> struct _Unique_if<T[N]> {
  typedef void _Known_bound;
  };
  template<class T, class... Args>
  typename _Unique_if<T>::_Single_object
  make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
  template<class T>
  typename _Unique_if<T>::_Unknown_bound
  make_unique(size_t n) {
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
  }
  template<class T, class... Args>
  typename _Unique_if<T>::_Known_bound
  make_unique(Args&&...) = delete;
}
#endif

#undef FMR_DEBUG
//end FEMERA_HAS_WORK_IPP
#endif