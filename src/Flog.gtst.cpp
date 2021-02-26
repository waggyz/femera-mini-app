#include "gtest/gtest.h"
#include "core.h"
#if 0
using namespace Femera;

auto proc = new Proc();
auto a_flog = new Flog( proc );

TEST( Flog, TrivialTest ){
  EXPECT_EQ( 2+2, 4 );
}

int main(int argc, char** argv ){int err=0;
  err= a_flog->proc->init( &argc,argv );
  err= a_flog->proc->exit( err );// gtest run_all_tests is done here.
  delete a_flog;
  delete proc;
  return err;
}
#else
int main(int, char**){return 0; }
#endif
