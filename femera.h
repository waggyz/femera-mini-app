#ifndef INCLUDED_FEMERA_H
#define INCLUDED_FEMERA_H
// Conditional Compiling
#ifndef VERB_MAX
#define VERB_MAX 3
#endif
// Maximum verbosity supported by executable
// 0:none, 1:CSV line output, 2: timing, 3: status //11:debug level
#define INT_PART int
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
// Partion Node, Element, DOF Numbers
#define INT_MESH unsigned int
//
// Global Node, Element, DOF Numbers
//FIXME Not yet used
#define INT_GLOB unsigned int
//
#define FLOAT_MESH double
// Mesh precision
#define FLOAT_PHYS double
// Physics precision
#define FLOAT_SOLV double
// Solver precision
#define ALIGN_SYS
// align system vectors
#define SYSALIGN_BYTE 64
#define VECALIGN_BYTE 32
#define VECALIGNED __attribute__((aligned(32)))
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
#ifdef COLLECT_VTUNE_DATA
#ifdef __INTEL_COMPILER
// # INTEL_NO_ITTNOTIFY_API
#include <ittnotify.h>
#endif
#endif
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
