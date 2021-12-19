#include "Work.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

template <typename T>// Derive a Base class from abstract (pure virtual) Work...
class Base : public femera::Work {
  public:
    void init (int*, char**) override {};
    int  exit (int err) override {
      return err==0 ? T::task_exit (err) : err;
    };
    std::shared_ptr<T> get_task (int i) {
      return std::static_pointer_cast<T> (get_work (i));
    };
  protected:// make it clear that Base needs to be inherited
    Base ()            =default;
    Base (const Base&) =default;
    Base (Base&&)      =default;// pointer copyable
    Base<T>& operator=
      (const Base<T>&) =default;
    ~Base ()           =default;
};
class Testable;// ...then derive a CRTP concrete class from Base for testing.
using Testable_t = std::shared_ptr <Testable>;
class Testable : public Base <Testable> {
  public:
    static int task_exit (int) {return 42;};
    Testable () {this->name ="testable class derived from Base"; };
};

auto testable = std::make_shared<Testable> ();
auto another  = std::make_shared<Testable> (*testable);

TEST( TestableWork, TaskName ) {
  EXPECT_NE( testable, another );
  EXPECT_EQ( testable->name, "testable class derived from Base");
  EXPECT_EQ( another ->name, "testable class derived from Base");
  another->name ="another Testable";
  EXPECT_EQ( testable->name, "testable class derived from Base");
  EXPECT_EQ( another ->name, "another Testable");
}
TEST( TestableWork, AddGetTask ) {
  EXPECT_EQ( testable->get_task_n(), 0);
  testable->add_task (another);
  EXPECT_EQ( testable->get_task_n(), 1);
  auto T0 = testable->get_task(0);// Work object in task_list, cast to derived
  EXPECT_EQ( typeid(T0), typeid(Testable_t));
  EXPECT_EQ( T0->name, "another Testable");// derived instance Testable::name
}
TEST( TestableWork, PointerCopy ) {
  auto T0 = std::static_pointer_cast<Testable> (testable);
  auto T1 = T0; T1->name = "changed name";
  EXPECT_EQ( T0, T1 );
  EXPECT_EQ( T0->name, "changed name");
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
//delete testable;
