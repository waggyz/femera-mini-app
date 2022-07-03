#include "../core.h"
#include "Self.hpp"

//#include "../proc/Node.hpp"//TODO Remove.

#include <valarray>

#if 0
//#include <gtest/gtest.h>
TEST( SelfTest, TrivialTest ){
  EXPECT_EQ( 0, 0 );
}
#endif
namespace femera {
  void test::Self::task_init (int*, char**) {// Test for initialization errors.
    if (true) {//TODO detail && this->test->do_test
      this->log_init_info ();// pulled out method to prevent inline fail warning
    }
    return;
  }
  void test::Self::log_init_info () {// pulled out of task_init (inline fail)
    const auto str = get_base_abrv ()+" "+ abrv +" ";
    const auto cppver = __cplusplus;
FMR_WARN_INLINE_OFF
    this->data->name_line (data->fmrlog, str +" ver",
      std::string (MAKESTR(FMR_VERSION)));
#if 0
    this->data->name_line (data->fmrlog, str +" C++",
      std::to_string(cppver) +", gcc "+ std::string(__VERSION__));
#else
    this->data->name_line (data->fmrlog, str +" g++",
      std::string(__VERSION__));
    this->data->name_line (data->fmrlog, str +" C++", "%li", cppver);
#endif
    this->data->name_line (data->fmrlog, str +"zord",
      "%4u maximum hypercomplex order", zyclops::max_zorder);
FMR_WARN_INLINE_ON
  }
}//end femera namespace
