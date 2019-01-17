#ifndef INCLUDED_TEST_H
#define INCLUDED_TEST_H
#include "femera.h"
//
class Test{
public:
  int CheckCubeError( RESTRICT Phys::vals &errors, FLOAT_PHYS nu,
    const RESTRICT Phys::vals node_coor, const RESTRICT Phys::vals disp);
  //
  //
protected:
private:
};
#endif