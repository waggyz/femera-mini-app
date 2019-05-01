#include <utility>//std::pair
#include <vector>
#include <set>// This is ordered
#include <algorithm>    // std::copy
#include <unordered_map>
#include <vector>
#include <tuple>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <omp.h>
#include "femera.h"
//
int PCR::BC (Mesh* ){return 0;};
int PCR::RHS(Mesh* ){return 0;};
int PCR::Iter(){ return(0); };
//int PCR::IterGPU( const FLOAT_GPU*,const FLOAT_GPU*,
//                  const FLOAT_GPU*, FLOAT_GPU* ){ return(0); };
//
int PCR::RHS(Elem* , Phys*  ){ return(0); };
int PCR::BCS(Elem* , Phys*  ){ return(0); };
int PCR::BC0(Elem* , Phys*  ){ return(0); };
int PCR::Setup( Elem* , Phys*  ){ return(0); };
int PCR::Init( Elem* , Phys*  ){ return 0; };
int PCR::Init(){ return 0; };
int PCR::Solve( Elem* , Phys*  ){ return 0; };
int HaloPCR::Init(){ return 0; };
int HaloPCR::Iter(){ return 0; };
int HaloPCR::IterGPU( const IDX_GPU*, const IDX_GPU*,
                      const INT_GPU*, FLOAT_GPU* ){ return 0; };

