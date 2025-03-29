#ifndef HAS_FEMERA_HPP
#define HAS_FEMERA_HPP

#define FMR_EXPORT __attribute__((visibility("default")))

#include "core.h"

namespace fmr {
  // typedefs
  /*
   * The "using" syntax is preferred to "typedef" in the Google C++ Style Guide:
   * https://google.github.io/styleguide/cppguide.html#Aliases
   */
  using Jobs_t = femera::Jobs_spt;// smart pointer for internal C++ interface
  // functions
  Jobs_t new_jobs (int*, char**);
  Jobs_t new_jobs ();
}
namespace fmc {// Femera C namespace
  using Jobs_t = femera::task::Jobs;// plain pointer for C/Python interface
  extern "C" {
    // C/Python interface to a femera::task::Jobs instance.
    FMR_EXPORT Jobs_t* fmr_new_jobs ();
    FMR_EXPORT void fmr_jobs_init (Jobs_t*);
    FMR_EXPORT bool fmr_jobs_did_init (Jobs_t*);
    FMR_EXPORT void fmr_jobs_exit (Jobs_t*);
    FMR_EXPORT void fmr_delete_jobs (Jobs_t*);
    FMR_EXPORT const char* fmr_get_version (Jobs_t*);
    FMR_EXPORT const char* fmr_get_jobs_name (Jobs_t*);
    FMR_EXPORT fmr::Dim_int fmr_get_verbosity (Jobs_t*);
    FMR_EXPORT fmr::Dim_int fmr_set_verbosity (Jobs_t*, fmr::Dim_int);
    FMR_EXPORT fmr::Local_int fmr_add_sims (Jobs_t*);

    FMR_EXPORT fmr::Local_int fmr_get_sims_n (Jobs_t*);

    FMR_EXPORT const char* fmr_get_sims_name (Jobs_t*, fmr::Local_int);
    FMR_EXPORT void fmr_set_sims_name (Jobs_t*, fmr::Local_int, const char*);

    FMR_EXPORT const char* fmr_get_sims_version (Jobs_t*, fmr::Local_int);
    FMR_EXPORT void fmr_set_sims_version (Jobs_t*, fmr::Local_int, const char*);

    FMR_EXPORT fmr::Enum_int fmr_get_sims_application (Jobs_t*, fmr::Local_int);
    FMR_EXPORT fmr::Enum_int fmr_set_sims_application 
      (Jobs_t*, fmr::Local_int, fmr::Enum_int);
  }
}//end fmc:: namespace

#include "femera.ipp"
/*
Notices:
Copyright 2018 United States Government as represented by the Administrator of
the National Aeronautics and Space Administration. No copyright is claimed in
the United States under Title 17, U.S. Code. All Other Rights Reserved.

Disclaimers
No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR
FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE
SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF
PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE
UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY
PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY
LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE,
INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE
OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED
STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR
RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH
MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
*/
//end HAS_FEMERA_HPP
#endif
