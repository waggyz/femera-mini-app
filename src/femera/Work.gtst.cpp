#include "Work.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif
//=============================================================================
template <typename T>// Derive a Base class from abstract (pure virtual) Work...
class Base : public femera::Work {
private:
  using Derived_t = std::shared_ptr<T>;
public:
  void      init     (int*, char**) override;
  int       exit     (int err)      override;
  Derived_t get_task (int i);
private:
  T* cast_to_derived (Base* ptr);
protected:// make it clear that Base needs to be inherited
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
  Testable ();
  int task_exit (int);
};//===========================================================================
inline
Testable::Testable () {
  this->name ="testable class derived from Base";
}
inline
int Testable::task_exit (int) {//TODO throw and return void
  return 42;
}//----------------------------------------------------------------------------
template <typename T> inline
T* Base<T>::cast_to_derived (Base* ptr){
  return static_cast<T*>(ptr);
}
template <typename T> inline
void Base<T>:: init (int*, char**)  {
}
template <typename T> inline
int Base<T>::exit (int err) {
  err = femera::Work::exit (err);// exit task stack, then exit this task
  return err==0 ? Base::cast_to_derived(this)->task_exit (err) : err;//TODO try/catch
}
template <typename T> inline
std::shared_ptr<T> Base<T>::get_task (int i) {
  return std::static_pointer_cast<T> (this->get_work (i));
}//============================================================================
auto testable = std::make_shared<Testable> ();
auto another1 = std::make_shared<Testable> (*testable);
//-----------------------------------------------------------------------------
TEST( TestableWork, ClassSize ) {
  EXPECT_EQ( sizeof(femera::Work::Task_list_t), 80);
  const int instance_size=16, class_size = 224;
  EXPECT_EQ( sizeof(femera::Work), class_size );
  EXPECT_EQ( sizeof(Base<Testable>), class_size );
  EXPECT_EQ( sizeof(Testable), class_size );
  EXPECT_EQ( sizeof(testable), instance_size );
  EXPECT_EQ( sizeof(testable), sizeof(another1) );
}
TEST( TestableWork, TaskName ) {
  EXPECT_NE( testable, another1 );
  EXPECT_EQ( testable->name, "testable class derived from Base");
  EXPECT_EQ( another1->name, "testable class derived from Base");
  another1->name ="another Testable";
  EXPECT_EQ( testable->name, "testable class derived from Base");
  EXPECT_EQ( another1->name, "another Testable");
}
TEST( TestableWork, AddGetExitTask ) {
  EXPECT_EQ( testable->get_task_n(), 0);
  EXPECT_EQ( another1.use_count(), 1);
  testable->add_task (another1);
  EXPECT_EQ( testable->get_task_n(), 1);
  EXPECT_EQ( another1.use_count(), 2);
  {// Scope T0
  auto T0 = testable->get_task(0);// Work object in task_list, cast to derived
  EXPECT_EQ( another1.use_count(), 3);
  EXPECT_EQ( typeid(T0), typeid(Testable_t));
  EXPECT_EQ( T0->name, "another Testable");// derived instance Testable::name
  EXPECT_EQ( T0->Work::name, "another Testable");
  EXPECT_EQ( T0->get_task_n(), 0);
  }
  EXPECT_EQ( another1.use_count(), 2);
  testable->exit (0);
  EXPECT_EQ( testable->get_task_n(), 0);
  EXPECT_EQ( another1.use_count(), 1);
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
  EXPECT_EQ( testable->exit(1),  1);
}
int main (int argc, char** argv) {
#ifdef FMR_HAS_MPI
  int err=0;
  ::MPI_Init (&argc,&argv);
  ::testing::InitGoogleTest (&argc,argv);
  int proc_id=0; ::MPI_Comm_rank (MPI_COMM_WORLD,& proc_id);
  //
  //from: https://github.com/google/googletest/issues/822
  ::testing::TestEventListeners& listeners
    = ::testing::UnitTest::GetInstance ()->listeners ();
  if (proc_id != 0) {// Only print from master; release the others.
    delete listeners.Release (listeners.default_result_printer ());
  }
  err = RUN_ALL_TESTS();
  ::MPI_Finalize ();
  return err;
#else
  ::testing::InitGoogleTest (&argc,argv);
  return RUN_ALL_TESTS();
#endif
}
#undef FMR_DEBUG
