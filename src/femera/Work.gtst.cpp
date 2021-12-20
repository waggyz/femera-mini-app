#include "Work.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

template <typename T>// Derive a Base class from abstract (pure virtual) Work...
class Base : public femera::Work {
public:
  void init (int*, char**) override {};
  int  exit (int err) override;
  std::shared_ptr<T> get_task (int i);
protected:// make it clear that Base needs to be inherited
  Base ()            =default;
  Base (const Base&) =default;
  Base (Base&&)      =default;// pointer copyable
  Base<T>& operator=
    (const Base<T>&) =default;
  ~Base ()           =default;
};
#define DERIVED static_cast<T*>(this)
template <typename T>
int Base<T>::exit (int err) {
  err = femera::Work::exit (err);// exit task stack, then exit this task
  return err==0 ? DERIVED->task_exit (err) : err;//TODO try/catch
}
template <typename T>
std::shared_ptr<T> get_task (int i) {
  return std::static_pointer_cast<T> (get_work (i));
}
#undef DERIVED
class Testable;// ...then derive a CRTP concrete class from Base for testing.
using Testable_t = std::shared_ptr <Testable>;
class Testable : public Base <Testable> {
  public:
    Testable () {this->name ="testable class derived from Base"; }
     int task_exit (int) {return 42;}//TODO throw and return void
};
auto testable = std::make_shared<Testable> ();
auto another1 = std::make_shared<Testable> (*testable);

TEST( TestableWork, ClassSize ) {
  const int class_size = 224, instance_size=16;
  EXPECT_EQ( sizeof(femera::Work::Task_list_t), 80);
  EXPECT_EQ( sizeof(femera::Work), class_size );
  EXPECT_EQ( sizeof(Base<Testable>), class_size );
  EXPECT_EQ( sizeof(Testable), class_size );
  EXPECT_EQ( sizeof(testable), instance_size );
  EXPECT_EQ( sizeof(testable), sizeof(another1) );
//    sizeof(femera::Work)
//    - sizeof(fmr::perf::Meter) - sizeof(std::string)
//    - 4 * sizeof(nullptr) - sizeof(femera::Work::Task_list_t), 16 );
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
  auto T0 = testable->get_task(0);// Work object in task_list, cast to derived
  EXPECT_EQ( another1.use_count(), 3);
  EXPECT_EQ( typeid(T0), typeid(Testable_t));
  EXPECT_EQ( T0->name, "another Testable");// derived instance Testable::name
  EXPECT_EQ( T0->Work::name, "another Testable");
  testable->exit (0);
  EXPECT_EQ( testable->get_task_n(), 0);
  EXPECT_EQ( T0->get_task_n(), 0);
  EXPECT_EQ( another1.use_count(), 2);
  T0 = nullptr;
  EXPECT_EQ( another1.use_count(),1);
}
TEST( TestableWork, PointerCopy ) {
  auto T0 = std::static_pointer_cast<Testable> (testable);
  auto T1 = T0; T1->name = "changed name";
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
