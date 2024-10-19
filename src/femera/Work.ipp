#ifndef FEMERA_HAS_WORK_IPP
#define FEMERA_HAS_WORK_IPP

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

template <typename T, typename C> inline constexpr
T* femera::Work::cast_via_work (C* child)
noexcept {
  return static_cast<T*> (static_cast<Work*> (child));
}
namespace femera {
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
  inline
  Work* Work::get_work (const Work::Task_path_t& path)
  noexcept {
    auto W = this;
    const auto sz = path.size ();
    if (sz > 0) {
      for (fmr::Local_int i = 0; i < sz; ++i) {
        if (W != nullptr) {
          W = W->get_work (path [i]);
    } } }
    return W;
  }
  inline
  bool Work::has_task (const Work::Task_path_t& path)
  noexcept {
    //const auto W = Work::get_work (path);
    return Work::get_work (path) != nullptr;
  }
  inline
  bool Work::has_task (const Work_type wt, const fmr::Local_int ix)
  noexcept {
    //const auto W = Work::get_work (wt, ix);
    return Work::get_work (wt, ix) != nullptr;
  }
  inline
  fmr::Local_int Work::get_task_n ()
  noexcept {
    return fmr::Local_int (this->task_list.size ());
  }
  inline
  fmr::Local_int Work::get_task_n (const Work_type wt)
  noexcept {
    fmr::Local_int count=0;
    const auto n = this->task_list.size ();
    for (fmr::Local_int i=0; i<n; ++i) {
      count += (this->get_work (i)->get_task_type () == wt) ? 1u : 0u ;
    }
    return count;
  }
  inline
  fmr::Local_int Work::get_task_n (const Task_type wt)
  noexcept {
    fmr::Local_int count=0;
    const auto n = this->task_list.size ();
    for (fmr::Local_int i=0; i<n; ++i) {
      count+= ( this->get_work (i)->get_task_type () == femera::task_cast (wt))
      ? 1u : 0u ;
    }
    return count;
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
  Work_type Work::get_task_type ()
  noexcept {
    return this->task_type;
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
  inline
  bool Work::did_init ()
  noexcept {
    return this->did_work_init;
  }
  inline
  bool Work::set_init (const bool tf)
  noexcept {
#ifdef FMR_DEBUG
    if (did_work_init == tf) {
      printf ("%s redundant set_init (%s)\n", abrv.c_str(),
        tf ? "true":"false");
    }
#endif
    this->did_work_init = tf;
    return this->did_work_init;
  }
  inline
  std::string Work::get_did_init_list ()
  noexcept { auto list = std::string ();
    const auto n = this->get_task_n ();
    if (n>0) {
      for (fmr::Local_int i=0; i<n; i++) {
        const auto W = this->get_work (i);
        if (W->did_work_init) {
          list += W->get_abrv ();
          if (i < (n - 1)) { list +=" "; }
    } } }
    return list;
  }
  inline
  std::string Work::get_not_init_list ()
  noexcept { auto list = std::string ();
    const auto n = this->get_task_n ();
    if (n>0) {
      for (fmr::Local_int i=0; i<n; i++) {
        const auto W = this->get_work (i);
        if (! W->did_work_init) {
          list += W->get_abrv ();
          if (i < (n - 1)) { list +=" "; }
    } } }
    return list;
  }
  //
}//end femera:: namespace
#if 0
namespace  fmr {
template<typename T, typename... Args> inline
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}//end fmr:: namespace
#endif
#if 0
//#include <cstddef>
//#include <memory>
//#include <type_traits>
//#include <utility>
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
