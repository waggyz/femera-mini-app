#ifndef FMR_HAS_TASK_HPP
#define FMR_HAS_TASK_HPP
/** */
#include "type.hpp"
//#include "femera.hpp"//TODO Remove this file?

#include <stack>
#include <queue>
#include <deque>
#include <sstream>
#include <iterator>

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

#if 0
#define FMR_CAST_TO_DERIVED reinterpret_cast
#else
#define FMR_CAST_TO_DERIVED static_cast
#endif
// dynamic_cast is safer but slower than reinterpret_cast,
// but now static_cast works!

namespace Femera {
  //TODO Move these to fmr:: or fmr::detail:: ?
  enum class Work_type :fmr::Enum_int{};// strongly typed int; see:
  // https://stackoverflow.com/questions/18195312/what-happens-if-you-static-cast-invalid-value-to-enum-class
  enum class Base_type :fmr::Enum_int{//TODO maybe change this to Work_type or Core_type ?
    None=0, Work,
    Proc, Data, Main,// Flog,
    Sims, Geom, Load, Phys, Cond, Solv, Sync, Post,
  #ifdef _OPENMP
    Pomp,// actually derived from Proc, but treated as a Base_type
  #endif
    Frun, Part,// Derived from Sims //TODO Cpu1, Fifo, Fset?
    Plug// Must be last, derived from main
  };
  enum class Plug_type :fmr::Enum_int;// forward declare for definition in Main/Plug.hpp
  static inline constexpr Work_type work_cast (Work_type) noexcept;
  static inline constexpr Work_type work_cast (Base_type) noexcept;
  static inline constexpr Work_type work_cast (Plug_type) noexcept;
}//end Femera namespace

namespace fmr {namespace detail {
  template <typename T>
  void string_split (const std::string &s, char delim, T result){
    // Use for parsing filename.ext, and for navigating part tree: sim_name/1/1
    std::istringstream iss(s);
    std::string item;
    while (std::getline (iss, item, delim)) {result->push_back (item); }
  }
} }//end fmr::detail::namespace

// Header-only implmentation =================================================
namespace Femera {
  static inline constexpr Work_type work_cast (Work_type work) noexcept{
    return work;
  }
  static inline constexpr Work_type work_cast (Base_type work) noexcept{
    return static_cast<Work_type> (work);
  }
  static inline constexpr Work_type work_cast (Plug_type work) noexcept{
    return static_cast<Work_type> (work);
  }
  template<class T> class Task {
  // member variables --------------------------------------------------------
  private:
    T* work =nullptr;
    //TODO Change stack items to unique_ptr or shared_ptr, so no cleanup needed.
    std::deque<T*> todo_task ={};// BIDI, empty after this->exit_stack()
    std::queue<T*> todo_init ={};// FIFO, empty after this->init_stack()
    std::stack<T*> todo_exit ={};// LIFO, empty after this->exit_stack()
    std::stack<T*> todo_free ={};// LIFO, empty after this->free_stack()
  // methods -----------------------------------------------------------------
  private:
    Task ();
    Task<T>& operator=(const Task<T>&);// not assignable
  public:
    Task (T* mywork);
    Task (Task const&)=default;// copy constructor
#if 0
    Task<T>& operator=(const Task<T>&){return *this;}
#endif
    ~Task()noexcept=default;
    //
    int init_stack (int* argc, char** argv);
    int exit_stack (int err);
    int free_stack ();
    //
    int add (T* task);
    int count ();
    template<typename Y>int count (const Y work_type);
    template<class B> B* get (const int i);
    template<class B, typename Y> B* first (const Y work_type);
  };
// Header-only class templates ===============================================
template<class T> Task<T>::Task (T* mywork){
  this->work = mywork;
}
template<class T> int Task<T>::count (){
  return int(this->todo_task.size ());
}
template<class T> template<typename Y> int Task<T>::count (const Y type){
  int c=0;
  const size_t n = this->todo_task.size ();
  if (n>0) {
    for (size_t i=0; i<n; i++){
      if (this->todo_task [i]->work_type == work_cast (type)) {c+=1; }
  } }
  return c;
}
template<class T> template<class B> B* Task<T>::get (const int i){
  return FMR_CAST_TO_DERIVED<B*> (this->todo_task [i]);
}
template<class T> template<class B, typename Y>
  B* Task<T>::first (const Y type){
  const size_t n = this->todo_task.size ();
  for (size_t i=0; i<n; i++){
    if (this->todo_task [i]->work_type == work_cast (type)){
      return FMR_CAST_TO_DERIVED<B*> (this->todo_task [i]);
  } }
#if 0
  for (size_t i=0; i<n; i++){// If not found yet, try to match base)type.
    if (this->todo_task [i]->base_type == work_cast (type)){
      return FMR_CAST_TO_DERIVED<B*> (this->todo_task [i]);
  } }
#endif
  return static_cast<B*> (nullptr);
}
template<class T> int Task<T>::add (T* task){
  // Push task onto my stacks.
  this->todo_task.push_front (task);
  this->todo_init.push (task);
  this->todo_exit.push (task);
  this->todo_free.push (task);
  return 0;
}
#if 0
template<class T> int Task<T>:: remove_first(){
    //this->todo_init.pop();
    this->todo_exit.pop();
    this->todo_task.pop_front();
    T* W = this->todo_free.top(); this->todo_free.pop();
    if (W != work && W!= nullptr){delete W;W=nullptr; }
  return 0;
}
#endif
#if 0
template <class T> int Task<T>:: add_todo_free (T* task ){
    this->todo_free.push(task);
    return 0;
  }
#endif
template<class T> int Task<T>::init_stack (int* argc, char** argv){int err=0;
  while (!this->todo_init.empty()){
    T* W=this->todo_init.front(); this->todo_init.pop();
    if (W!=nullptr){
      int init_err = W->init_task (argc,argv);
      if (init_err){err= init_err; }
  } }
  return err;
}
template<class T> int Task<T>::exit_stack (int err){
  int exit_err=0;
  while (!this->todo_exit.empty()){
    T* W=this->todo_exit.top(); this->todo_exit.pop();
    if (W!=nullptr){
#ifdef FMR_DEBUG
      std::printf("Exiting %s...\n",
        W->task_name.c_str());
#endif
      exit_err = W->exit_task (err);
      if (exit_err){err= exit_err; }
  } }
  while (!this->todo_task.empty()){ this->todo_task.pop_front(); }
  this->free_stack();
  return err;
}
template<class T> int Task<T>:: free_stack (){// Delete todo_free objects
  while (!this->todo_free.empty()){
#ifdef FMR_DEBUG
    std::printf("Deleting %s...\n",
      this->todo_free.top()->task_name.c_str());
#endif
    T* W = this->todo_free.top(); this->todo_free.pop();
    if (W != work && W!= nullptr){delete W;W=nullptr; }// Do not delete self.
  }
  return 0;
}
}//end Femera namespace
// #undef FMR_CAST_TO_DERIVED //TODO should only be used here.
#undef FMR_DEBUG
#endif
