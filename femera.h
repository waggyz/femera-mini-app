#ifndef INCLUDED_FEMERA_H
#define INCLUDED_FEMERA_H
// Conditional Compiling
#ifndef VERB_MAX
#define VERB_MAX 3
#endif
// Maximum verbosity supported by executable
// 0:none, 1:CSV line output, 2: timing, 3: status //11:debug level
//FIXME Change the next to INT_PART
#define INT_MESH_PART int
// Mesh Partition ID (user-defined?)
//#define INT_MESH unsigned int// Element Numbers
//#define INT_ELEM_COLOR unsigned char// Color numbers
// (unsigned char or unisgned short int) color 0 is no color assigned
#define INT_ELEM_NODE unsigned short int
// char Number of nodes/element
#define INT_DIM short int
//FIXME Change the next two to INT_DIM
#define INT_ORDER unsigned short int
// char Element order, mesh dimension, etc.
#define INT_DOF unsigned short int
// char DOF id number
//
// Node, Element, DOF Numbers
#define INT_MESH unsigned int
#define FLOAT_MESH double
// Mesh precision
#define FLOAT_PHYS double
// Physics precision
#define FLOAT_SOLV double
// Solver precision
#define ALIGN_SYS
// align system vectors
//FIXME I don't think the following are needed...
#define VOIGT_FRUP 0,4,8, 5,2,1// Extract from upper-triangular part of
//symmetric 3x3 stress or strain tensor
#define VOIGT_FRLO 0,4,8, 7,6,3// Voigt: sxx,syy,szz, syz,sxz,sxy;
//or exx,eyy,ezz, 2eyz,2exz,2exy
#define VOIGT_TOFU 0,5,4, 5,1,3, 4,3,2// Voigt vector to full symmetric 3x3 tensor
//
// Integer return values
#define SOLV_CNVG_PTOL 1// Preconditioned norm
//
#define COLOR_NONE 0
//
#ifndef RESTRICT
#define RESTRICT __restrict
#endif
// #define HAS_PATCH //Set this as a compiler option: -DHAS_PATCH
// #define HAS_TEST  //Set this as a compiler option: -DHAS_TEST
//FIXME Remove unneeded headers:
#include <valarray>
#include <deque>
#include <utility>// std::pair
#include <tuple>
#include <vector>
#include <chrono>
const double PI = FLOAT_PHYS(3.141592653589793);
class Mesh;
class Elem;
class Phys;
class Solv;
#include "solv.h"
#include "mesh.h"
#include "elem.h"
#include "phys.h"
//
class Femera{
public:
  typedef std::tuple< Elem*, Phys*, Solv* > partitem;
  typedef std::vector<partitem> partlist;
  //
  //
  partlist part={};
  //
protected:
private:
};



inline void accumulate_f( __m256d* vf,
  const __m256d* a, const FLOAT_PHYS* G, const int elem_p ){
  __m256d a036=a[0];__m256d a147=a[1];__m256d a258=a[2];
#if 1
  for(int i=0; i<2; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]);
    g1 = _mm256_set1_pd(G[8*i+1]);
    g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i],
      _mm256_add_pd(_mm256_mul_pd(g0,a036),
        _mm256_add_pd(_mm256_mul_pd(g1,a147),
          _mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]);
    g4 = _mm256_set1_pd(G[8*i+5]);
    g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1],
      _mm256_add_pd(_mm256_mul_pd(g3,a036),
        _mm256_add_pd(_mm256_mul_pd(g4,a147),
          _mm256_mul_pd(g5,a258))));
  }
  if(elem_p>1){
    for(int i=2; i<5; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]);
    g1 = _mm256_set1_pd(G[8*i+1]);
    g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i],
      _mm256_add_pd(_mm256_mul_pd(g0,a036),
        _mm256_add_pd(_mm256_mul_pd(g1,a147),
          _mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]);
    g4 = _mm256_set1_pd(G[8*i+5]);
    g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1],
      _mm256_add_pd(_mm256_mul_pd(g3,a036),
        _mm256_add_pd(_mm256_mul_pd(g4,a147),
          _mm256_mul_pd(g5,a258))));
    }
  }
  if(elem_p>2){
    for(int i=5; i<10; i++){
    __m256d g0,g1,g2, g3,g4,g5;
    g0 = _mm256_set1_pd(G[8*i  ]);
    g1 = _mm256_set1_pd(G[8*i+1]);
    g2 = _mm256_set1_pd(G[8*i+2]);
    vf[2*i]= _mm256_add_pd(vf[2*i],
      _mm256_add_pd(_mm256_mul_pd(g0,a036),
        _mm256_add_pd(_mm256_mul_pd(g1,a147),
          _mm256_mul_pd(g2,a258))));
    g3 = _mm256_set1_pd(G[8*i+4]);
    g4 = _mm256_set1_pd(G[8*i+5]);
    g5 = _mm256_set1_pd(G[8*i+6]);
    vf[2*i+1]= _mm256_add_pd(vf[2*i+1],
      _mm256_add_pd(_mm256_mul_pd(g3,a036),
        _mm256_add_pd(_mm256_mul_pd(g4,a147),
          _mm256_mul_pd(g5,a258))));
    }
  }
#else
  for(int i=0; i<4; i++){
    __m256d g0,g1,g2;
    g0 = _mm256_set1_pd(G[4*i  ]);
    g1 = _mm256_set1_pd(G[4*i+1]); g2
    = _mm256_set1_pd(G[4*i+2]);
    vf[i]= _mm256_add_pd(vf[i],
      _mm256_add_pd(_mm256_mul_pd(g0,a036),
        _mm256_add_pd(_mm256_mul_pd(g1,a147),
          _mm256_mul_pd(g2,a258))));
  }
  if(elem_p>1){
    for(int i=4; i<10; i++){
      __m256d g0,g1,g2;
      g0 = _mm256_set1_pd(G[4*i]);
      g1 = _mm256_set1_pd(G[4*i+1]);
      g2 = _mm256_set1_pd(G[4*i+2]);
      vf[i]= _mm256_add_pd(vf[i],
        _mm256_add_pd(_mm256_mul_pd(g0,a036),
          _mm256_add_pd(_mm256_mul_pd(g1,a147),
            _mm256_mul_pd(g2,a258))));
    }
  }
  if(elem_p>2){
    for(int i=10; i<20; i++){
      __m256d g0,g1,g2;
      g0 = _mm256_set1_pd(G[4*i]);
      g1 = _mm256_set1_pd(G[4*i+1]);
      g2 = _mm256_set1_pd(G[4*i+2]);
      vf[i]= _mm256_add_pd(vf[i],
        _mm256_add_pd(_mm256_mul_pd(g0,a036),
          _mm256_add_pd(_mm256_mul_pd(g1,a147),
            _mm256_mul_pd(g2,a258))));
    }
  }
#endif
}
inline void rotate_s( __m256d* a,
  const FLOAT_PHYS* R, const FLOAT_PHYS* S ){
  __m256d s0,s1,s2,s4,s5,s8;
  __m256d r0,r3,r6;
  r0  = _mm256_loadu_pd(&R[0]);
  r3 = _mm256_loadu_pd(&R[3]);
  r6 = _mm256_loadu_pd(&R[6]);
  s0  = _mm256_set1_pd(S[0]);
  s1 = _mm256_set1_pd(S[4]);
  s2 = _mm256_set1_pd(S[5]);
  s4  = _mm256_set1_pd(S[1]);
  s5 = _mm256_set1_pd(S[6]);
  s8 = _mm256_set1_pd(S[2]);
  a[0]=_mm256_add_pd(_mm256_mul_pd(r0,s0),
    _mm256_add_pd(_mm256_mul_pd(r3,s1),
      _mm256_mul_pd(r6,s2)));
  a[1]=_mm256_add_pd(_mm256_mul_pd(r0,s1),
    _mm256_add_pd(_mm256_mul_pd(r3,s4),
      _mm256_mul_pd(r6,s5)));
  a[2]=_mm256_add_pd(_mm256_mul_pd(r0,s2),
    _mm256_add_pd(_mm256_mul_pd(r3,s5),
      _mm256_mul_pd(r6,s8)));
}
inline void compute_s(FLOAT_PHYS* S, const FLOAT_PHYS* H,
  const FLOAT_PHYS* C, const __m256d c0,const __m256d c1,const __m256d c2,
  const FLOAT_PHYS dw){
    __m256d s048 =
      _mm256_mul_pd(_mm256_set1_pd(dw),
        _mm256_add_pd(_mm256_mul_pd(c0,_mm256_set1_pd(H[0])),
          _mm256_add_pd(_mm256_mul_pd(c1,_mm256_set1_pd(H[5])),
            _mm256_mul_pd(c2,_mm256_set1_pd(H[10])))));
    _mm256_store_pd(&S[0], s048);
    S[4]=(H[1] + H[4])*C[6]*dw; // S[1]
    S[5]=(H[2] + H[8])*C[8]*dw; // S[2]
    S[6]=(H[6] + H[9])*C[7]*dw; // S[5]
}
inline void compute_g_h(
  FLOAT_PHYS* G, FLOAT_PHYS* H,
  const int Ne, const __m256d j0,const __m256d j1,const __m256d j2,
  const FLOAT_PHYS* isp, const FLOAT_PHYS* R, const FLOAT_PHYS* u ){
  //FLOAT_PHYS* RESTRICT isp = &intp_shpg[ip*Ne];
  __m256d a036=_mm256_set1_pd(0.0), a147=_mm256_set1_pd(0.0),
    a258=_mm256_set1_pd(0.0);
  int ig=0;
  for(int i= 0; i<Ne; i+=9){
    __m256d u0,u1,u2,u3,u4,u5,u6,u7,u8,g0,g1,g2;
    __m256d is0,is1,is2,is3,is4,is5,is6,is7,is8;
    is0= _mm256_set1_pd(isp[i+0]);
    is1= _mm256_set1_pd(isp[i+1]);
    is2= _mm256_set1_pd(isp[i+2]);
    u0 = _mm256_set1_pd(  u[i+0]);
    u1 = _mm256_set1_pd(  u[i+1]);
    u2 = _mm256_set1_pd(  u[i+2]);
    g0 = _mm256_add_pd(_mm256_mul_pd(j0,is0),
      _mm256_add_pd(_mm256_mul_pd(j1,is1),_mm256_mul_pd(j2,is2)));
    a036= _mm256_add_pd(a036, _mm256_mul_pd(g0,u0));
    a147 = _mm256_add_pd(a147, _mm256_mul_pd(g0,u1));
    a258 = _mm256_add_pd(a258, _mm256_mul_pd(g0,u2));
    _mm256_store_pd(&G[ig],g0);
    ig+=4;
    if((i+5)<Ne){
      is3= _mm256_set1_pd(isp[i+3]);
      is4= _mm256_set1_pd(isp[i+4]);
      is5= _mm256_set1_pd(isp[i+5]);
      u3 = _mm256_set1_pd(  u[i+3]);
      u4 = _mm256_set1_pd(  u[i+4]);
      u5 = _mm256_set1_pd(  u[i+5]);
      g1 = _mm256_add_pd(_mm256_mul_pd(j0,is3),
        _mm256_add_pd(_mm256_mul_pd(j1,is4),_mm256_mul_pd(j2,is5)));
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g1,u3));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g1,u4));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g1,u5));
      _mm256_store_pd(&G[ig],g1);
      ig+=4;
    }if((i+8)<Ne){
      is6= _mm256_set1_pd(isp[i+6]);
      is7= _mm256_set1_pd(isp[i+7]);
      is8= _mm256_set1_pd(isp[i+8]);
      u6 = _mm256_set1_pd(  u[i+6]);
      u7 = _mm256_set1_pd(  u[i+7]);
      u8 = _mm256_set1_pd(  u[i+8]);
      g2 = _mm256_add_pd(_mm256_mul_pd(j0,is6),
        _mm256_add_pd(_mm256_mul_pd(j1,is7),_mm256_mul_pd(j2,is8)));
      a036= _mm256_add_pd(a036, _mm256_mul_pd(g2,u6));
      a147 = _mm256_add_pd(a147, _mm256_mul_pd(g2,u7));
      a258 = _mm256_add_pd(a258, _mm256_mul_pd(g2,u8));
      _mm256_store_pd(&G[ig],g2);
      ig+=4;
    }
  }
  {// scope vector registers
  __m256d h036,h147,h258;
  {
  __m256d r0,r1,r2,r3,r4,r5,r6,r7,r8;
  r0 =_mm256_set1_pd(R[0]); r1 =_mm256_set1_pd(R[1]); r2 =_mm256_set1_pd(R[2]);
  r3 =_mm256_set1_pd(R[3]); r4 =_mm256_set1_pd(R[4]); r5 =_mm256_set1_pd(R[5]);
  r6 =_mm256_set1_pd(R[6]); r7 =_mm256_set1_pd(R[7]); r8 =_mm256_set1_pd(R[8]);
  h036 = _mm256_add_pd(_mm256_mul_pd(r0,a036),
         _mm256_add_pd(_mm256_mul_pd(r1,a147),_mm256_mul_pd(r2,a258)));
  h147 = _mm256_add_pd(_mm256_mul_pd(r3,a036),
         _mm256_add_pd(_mm256_mul_pd(r4,a147),_mm256_mul_pd(r5,a258)));
  h258 = _mm256_add_pd(_mm256_mul_pd(r6,a036),
         _mm256_add_pd(_mm256_mul_pd(r7,a147),_mm256_mul_pd(r8,a258)));
  }
  // Take advantage of the fact that the pattern of usage is invariant
  // with respect to transpose _MM256_TRANSPOSE3_PD(h036,h147,h258);
  _mm256_store_pd(&H[0],h036);
  _mm256_store_pd(&H[4],h147);
  _mm256_store_pd(&H[8],h258);
  }
}


// Helper functions that can be compiled out for performance testing.
#if VERB_MAX>1
inline void time_reset(long int &counter,
  std::chrono::high_resolution_clock::time_point &start ){
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  counter += dur.count();
  start = std::chrono::high_resolution_clock::now();
};
inline void time_accum(long int &counter,
  std::chrono::high_resolution_clock::time_point &start ){
  auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::high_resolution_clock::now()-start);
  counter += dur.count();
};
inline void time_start(
  std::chrono::high_resolution_clock::time_point &start ){
  start = std::chrono::high_resolution_clock::now();
};
#else
inline void time_reset(long int &,
  std::chrono::high_resolution_clock::time_point & ){};
inline void time_accum(long int &,
  std::chrono::high_resolution_clock::time_point & ){};
inline void time_start(
  std::chrono::high_resolution_clock::time_point & ){};
#endif


#endif
/*
Notices:
Copyright 2018 United States Government as represented by the Administrator of the National Aeronautics and Space Administration. No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.

Disclaimers
No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS, RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
*/
