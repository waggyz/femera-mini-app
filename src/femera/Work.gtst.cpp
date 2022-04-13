#include "Work.hpp"

#include "gtest/gtest.h"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

fmr::Exit_int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

namespace femera { namespace test {
  class Base {// can be put into a homogeneous container
//    virtual void virtual_function () noexcept =0;// pure virtual function
    void not_virtual_function () noexcept {}
  public:
    virtual ~Base ();// virtual destructor for polymorphic base class
  };
  template <typename T>
  class Base_child : public Base {
//    void virtual_function () noexcept final override{}
  };
  class Crtp;// Derive a CRTP concrete class from Base_child.
  class Crtp : public Base_child <Crtp> { private: friend class Base_child;
  };
  /*
  stackoverflow.com/questions/5970333/how-to-determine-if-a-c-class-has-a-vtable
  */
  template<class T>
  class Has_vtable {
  public :
      class Derived : public T {
        virtual void force_a_vtable(){}
      };
      enum { Value = (sizeof(T) == sizeof(Derived)) };
  };
} }//end femera::test:: namespace
#define OBJECT_HAS_VTABLE(type) femera::test::Has_vtable<type>::Value

TEST( Work, TrivialTest ) {
  EXPECT_EQ( 1, 1 );
}
TEST( Crtp, HasVtable ) {
  EXPECT_EQ( 1, OBJECT_HAS_VTABLE(femera::test::Crtp) );
}
#undef FMR_DEBUG
