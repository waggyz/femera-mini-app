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
  template <typename T>
  class Class {// not derived from a base class
    void not_virtual_function () noexcept {}
  };
  class Crtp_class;// Derive a CRTP concrete class from Class.
  class Crtp_class : public Class <Crtp_class> { private: friend class Class;
  };
  class Base {// can be put into a homogeneous container
//    virtual void virtual_function () noexcept =0;// pure virtual function
    void not_virtual_function () noexcept {}
  public:
    virtual ~Base ();// virtual destructor for polymorphic base class
  };
  template <typename T>
  class Child : public Base {
//    void virtual_function () noexcept final override{}
  };
  class Crtp;// Derive a CRTP concrete class from Child.
  class Crtp : public Child <Crtp> { private: friend class Child;
  };
  class Final;// Derive a final CRTP concrete class from Child.
  class Final final: public Child <Final> { private: friend class Child;
  };
  /*
  stackoverflow.com/questions/5970333/how-to-determine-if-a-c-class-has-a-vtable
  */
  template <class T>
  class Has_vtable {
  public :
      class Derived : public T {
        virtual void force_a_vtable (){}
      };
      enum { Value = (sizeof (T) == sizeof (Derived)) };
  };
} }//end femera::test:: namespace
#define OBJECT_HAS_VTABLE(type) femera::test::Has_vtable<type>::Value

TEST( Work, TrivialTest ) {
  EXPECT_EQ( 1, 1 );
}
TEST( Crtp, NoBaseNoVtable ) {
  EXPECT_EQ( 0, OBJECT_HAS_VTABLE(femera::test::Crtp_class) );
}
TEST( Crtp, DerivedHasVtable ) {
  EXPECT_EQ( 1, OBJECT_HAS_VTABLE(femera::test::Crtp) );
}
TEST( Crtp, FinalHasVtable ) {
  EXPECT_EQ( sizeof (femera::test::Crtp), sizeof (femera::test::Final) );
}
#undef FMR_DEBUG
