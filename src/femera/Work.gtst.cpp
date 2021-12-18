#include "Work.hpp"

#include "gtest/gtest.h"

#ifdef FMR_HAS_MPI
#include "mpi.h"
#endif

// Derive a concrete class from abstract (pure virtual) Work for testing.
class Testable : femera::Work <Testable> {
  // Variables ----------------------------------------------------------------
  public:
    std::string name ="Testable Work";
  // Methods ------------------------------------------------------------------
  public:
    void init (int*, char**) {};
    int  exit (int) override {return 0;}
    //
    Testable ()                     =default;
    Testable (Testable const&)      =delete;// not copyable
    void operator= (const Testable&)=delete;
    ~Testable ()                    =default;
};

auto testable = std::make_shared <Testable>();

TEST(TestableWork, WorkName) {
  EXPECT_EQ(testable->name, "Testable Work");
}
TEST(TestableWork, ExitErr) {
  EXPECT_EQ(testable->exit(1), 0);
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
