#if VERB_MAX > 10
#include <iostream>
#endif
#include <cstring>// std::memcpy
#include "femera.h"
#include <immintrin.h>
//
// Fetch next u within G,H loop nest
#undef FETCH_U_EARLY
#define COMPRESS_STATE
//NOTE Prefetch state only works when compressed.
//
int ElastPlastKHIso3D::Setup( Elem* ){
  return 0;
}
int ElastPlastKHIso3D::ElemNonlinear( Elem*,
  const INT_MESH,const INT_MESH, FLOAT_SOLV*,
  const FLOAT_SOLV*,
  const FLOAT_SOLV*, bool ){
  return 0;
}
int ElastPlastKHIso3D::ElemLinear( Elem*,
  const INT_MESH,const INT_MESH, FLOAT_SOLV*,
  const FLOAT_SOLV* ){
  return 0;
}
//
