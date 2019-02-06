#ifndef INCLUDED_FEMERA_H
#define INCLUDED_FEMERA_H
// Conditional Compiling
#ifndef VERB_MAX
#define VERB_MAX 3
#endif
// Maximum verbosity supported by executable
// 0:none, 1:CSV line output, 2: timing, 3: status //11:debug level
//
#define INT_MESH_PART int// Mesh Partition ID (user-defined?)
//#define INT_MESH unsigned int// Element Numbers
//#define INT_ELEM_COLOR unsigned char// Color numbers
// (unsigned char or unisgned short int) color 0 is no color assigned
#define INT_ELEM_NODE unsigned short int// char Number of nodes/element
#define INT_ORDER unsigned short int// char Element order, mesh dimension, etc.
#define INT_DOF unsigned short int// char DOF id number
//
// Node, Element, DOF Numbers
#define INT_MESH unsigned int
#define FLOAT_MESH double
// Mesh precision
#define FLOAT_PHYS double
// Physics precision
#define FLOAT_SOLV double
// Solver precision
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
//FIXME Remove unneeded headers:
#include <valarray>
#include <deque>
#include <utility>// std::pair
#include <tuple>
#include <vector>
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
#endif
/*
Notices:
Copyright 2018 United States Government as represented by the Administrator of the National Aeronautics and Space Administration. No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.

Disclaimers
No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS, RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
*/
