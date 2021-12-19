#include "Work.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif
// Derive a Base class from abstract (pure virtual) Work...

template <typename T>
class Base : public femera::Work {
  public:
    std::string name = std::string("Base derived from Work");
  public:
    void init (int*, char**) override {};
    int  exit (int err) override {
      const auto t=T::task_exit (err);return err==0?t:err; }
    std::shared_ptr<T> get_task (int i){
      return std::static_pointer_cast<T>(get_work (i));};
  protected://make it clear that Base class needs to be inherited
    Base ()                  =default;
    Base ( const Base&)      =default;
    Base (  Base&&)          =default;
    //Base ( Base& const)         =delete;// not copyable
    //void operator= (const Base&)=default;
    //~Testable ()                    =default;
};
// ...then derive a CRTP concrete class from the Base class for testing.
class Testable : public Base <Testable> {
  public:
    std::string name = std::string("testable class derived from Base");
    static int task_exit (int) {return 42;};
};

auto testable = std::make_shared <Testable> ();
auto another  = std::make_shared <Testable> (*testable);

TEST(TestableWork, WorkName) {
  EXPECT_EQ(testable->name, "testable class derived from Base");
  EXPECT_EQ(another ->name, "testable class derived from Base");
  another->name="another Testable";
  EXPECT_EQ(testable->name, "testable class derived from Base");
  EXPECT_EQ(another ->name, "another Testable");
}
TEST(TestableWork, AddGetTask) {
  EXPECT_EQ(testable->get_task_n(), 0);
  testable->add_task(another);
  EXPECT_EQ(testable->get_task_n(), 1);
  auto W0 = testable->get_work(0);// type is femera::Work_t
  auto T0 = testable->get_task(0);// same Work object, cast to derived class
  EXPECT_EQ(T0, W0);
  EXPECT_EQ(W0->name, "unknown work");// abstract class Work::name
  EXPECT_EQ(T0->name, "another Testable");// derived instance Testable::name
}
TEST(TestableWork, PointerCopy) {
  auto T0=std::static_pointer_cast<Testable>(testable);
  auto T1=T0; T1->name = "changed name";
  EXPECT_EQ(T0->name, "changed name");
}
TEST(TestableWork, ExitErr) {
  EXPECT_EQ(testable->exit(0), 42);
  EXPECT_EQ(testable->exit(1),  1);
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
