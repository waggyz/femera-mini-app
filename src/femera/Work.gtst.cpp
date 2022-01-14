#include "Work.hpp"

#include "gtest/gtest.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}
//=============================================================================
// Template a class derived from the abstract (pure virtual) Work class...
template <typename T>
class Base : public femera::Work {
private:
  using Derived_t = std::shared_ptr<T>;
public:
  fmr::Exit_int init     (int*, char**) noexcept override;
  fmr::Exit_int exit     (fmr::Exit_int err=0) noexcept override;
#if 0
  Derived_t     get_task (fmr::Local_int i);
  Derived_t     get_task (femera::Work::Task_path_t tree);//FIXME Replace w/below.
#endif
  T*     get_task (fmr::Local_int i);
  T*     get_task (femera::Work::Task_path_t tree);
private:
  T* derived (Base*);
protected:// Make it clear that Base needs to be inherited.
  Base ()            =default;
  Base (const Base&) =default;
  Base (Base&&)      =default;// shallow (pointer) copyable
  Base& operator =
    (const Base&)    =default;
  ~Base ()           =default;
};//---------------------------------------------------------------------------
class Testable;// ...then derive a CRTP concrete class from Base for testing.
using Testable_t = std::shared_ptr <Testable>;
class Testable : public Base <Testable> {
public:
  Testable (femera::Work::Core_t) noexcept;
  //FIXME This may not be needed if copy constructors set proc,data,file,test
  Testable () noexcept;
  void task_exit ();
};//===========================================================================
#if 1
inline
Testable::Testable (femera::Work::Core_t W) noexcept {
  this->name ="testable";
  std::tie(this->proc,this->data,this->test) = W;
}
#endif
inline
Testable::Testable () noexcept {
  this->name ="testable";
}
inline
void Testable::task_exit () {
  throw std::runtime_error("woops");
}//----------------------------------------------------------------------------
template <typename T> inline
T* Base<T>::derived (Base* ptr) {
  return static_cast<T*> (ptr);
}
template <typename T> inline
fmr::Exit_int Base<T>:: init (int*, char**) noexcept {
  return 0;
}
template <typename T> inline
fmr::Exit_int Base<T>::exit (fmr::Exit_int err) noexcept {
  if (err>0) {return err;}
  //err = femera::Work::exit (err);
  try { exit_list (); }// Exit the task stack,
  catch (...) { err=1; }
  if (err>0) {return err;}
  try { Base::derived(this)->task_exit (); }// then exit this task.
  catch (std::exception& e) { err = 42; }
  return err;
}
//FIXME Replace these =========================================================
# if 0
template <typename T> inline
std::shared_ptr<T> Base<T>::get_task (const fmr::Local_int i) {
  return std::static_pointer_cast<T> (this->get_work (i));
}
template <typename T> inline
std::shared_ptr<T> Base<T>::get_task (const femera::Work::Task_path_t tree) {
  return std::static_pointer_cast<T> (this->get_work (tree));
}
#endif
//FIXME with these ------------------------------------------------------------
template <typename T> inline
T* Base<T>::get_task (const fmr::Local_int i) {
  return static_cast<T*> (this->get_work_raw (i));
}
template <typename T> inline
T* Base<T>::get_task (const femera::Work::Task_path_t tree) {
  return static_cast<T*> (this->get_work_raw (tree));
}
//============================================================================
auto testable = std::make_shared<Testable> ();
auto another1 = std::make_shared<Testable> (testable->get_core());
//-----------------------------------------------------------------------------
TEST( TestableWork, ClassSize ) {
  EXPECT_EQ( sizeof(femera::Work::Task_list_t), size_t(80));
  EXPECT_EQ( sizeof(testable), sizeof(another1) );
}
TEST( TestableWork, TaskName ) {
  EXPECT_NE( testable, another1 );
  EXPECT_EQ( testable->name, "testable");
  EXPECT_EQ( another1->name, "testable");
  another1->name ="another";
  EXPECT_EQ( testable->name, "testable");
  EXPECT_EQ( another1->name, "another");
}
TEST( TestableWork, AddGetExitTask ) {
  EXPECT_EQ( testable->get_task_n(), fmr::Local_int(0));
  EXPECT_EQ( another1.use_count(), 1u);
  testable->add_task (another1);
  EXPECT_EQ( testable->get_task_n(), 1u);
  EXPECT_EQ( another1.use_count(), 2u);
  {// Scope T0
  auto T0 = testable->get_task(0u);// cast object in task_list to derived raw ptr
  EXPECT_EQ( another1.use_count(), 2u);
  EXPECT_NE( typeid(T0), typeid(Testable_t));// T* != shared_ptr<T>
  EXPECT_EQ( T0->name, "another");// derived instance Testable::name
  EXPECT_EQ( T0->Work::name, "another");
  EXPECT_EQ( T0->get_task_n(), 0u);
  }
  EXPECT_EQ( another1.use_count(), 2u);
  testable->exit ();
  EXPECT_EQ( testable->get_task_n(), 0u);
  EXPECT_EQ( another1.use_count(), 1u);
}
TEST( TestableWork, SubTask ) {
  auto subtask = std::make_shared<Testable> (testable->get_core());
  testable->add_task (another1);
  subtask->name = "subtask";
  another1->add_task (subtask);
  testable->get_task(0)->add_task (subtask);// same as above
  EXPECT_EQ( another1.use_count(), 2u);
  EXPECT_EQ( subtask.use_count(), 3u);
  EXPECT_EQ( testable->get_task_n(), 1u);
  EXPECT_EQ( testable->get_task(0)->get_task_n(), 2u);
  EXPECT_EQ( testable->get_task(femera::Work::Task_path_t({0}))->name, "another");
  EXPECT_EQ( testable->get_task(femera::Work::Task_path_t({0,0}))->name, "subtask");
  EXPECT_EQ( testable->get_task(femera::Work::Task_path_t({0,1}))->name, "subtask");
  EXPECT_EQ( testable->get_task(femera::Work::Task_path_t({0,3})), nullptr);
  testable->exit ();
  EXPECT_EQ( testable->get_task_n(), 0u);
  EXPECT_EQ( another1.use_count(), 1u);
  EXPECT_EQ( subtask.use_count(), 1u);
}
TEST( TestableWork, PointerCopy ) {
  auto T0 = std::static_pointer_cast<Testable> (testable);
  auto T1 = T0;
  T1->name = "changed name";
  EXPECT_EQ( T0, T1 );
  EXPECT_EQ( T0->name, "changed name");
}
TEST( TestableWork, PerfMeter ) {
  another1->time.add_idle_time_now ();
  testable->time.add_idle_time_now ();
  EXPECT_GT( testable->time.get_idle_s (), another1->time.get_idle_s () );
}
TEST( TestableWork, ExitErr ) {
  EXPECT_EQ( testable->exit(0), 42);
  EXPECT_EQ( testable->exit(3), 3);
}
#undef FMR_DEBUG
